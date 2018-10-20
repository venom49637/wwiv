/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2017, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/
#include "bbs/batch.h"

#include <algorithm>
#include <chrono>
#include <iterator>
#include <string>
#include <utility>

#include "bbs/bbsovl3.h"
#include "bbs/bgetch.h"
#include "bbs/bbs.h"
#include "bbs/bbsutl.h"
#include "bbs/com.h"
#include "bbs/datetime.h"
#include "bbs/execexternal.h"
#include "bbs/input.h"
#include "bbs/instmsg.h"
#include "bbs/pause.h"
#include "bbs/printfile.h"
#include "bbs/remote_io.h"
#include "bbs/shortmsg.h"
#include "bbs/sr.h"
#include "bbs/srsend.h"
#include "bbs/stuffin.h"
#include "bbs/sysoplog.h"
#include "bbs/utility.h"
#include "local_io/wconstants.h"
#include "bbs/xfer.h"
#include "bbs/xferovl.h"
#include "bbs/xferovl1.h"
#include "bbs/make_abs_cmd.h"
#include "core/os.h"
#include "core/stl.h"
#include "core/strings.h"
#include "core/wwivassert.h"
#include "sdk/filenames.h"
#include "sdk/status.h"

using std::begin;
using std::end;
using std::string;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::stl;
using namespace wwiv::strings;

// trytoul.cpp
int try_to_ul(const string& file_name);

// normupld.cpp
void normalupload(int dn);

//////////////////////////////////////////////////////////////////////////////
// Implementation

// Shows listing of files currently in batch queue(s), both upload and
// download. Shows estimated time, by item, for files in the d/l queue.
static void listbatch() {
  bout.nl();
  if (a()->batch().entry.empty()) {
    return;
  }
  bool abort = false;
  bout << "|#9Files - |#2" << a()->batch().entry.size() << "  ";
  if (a()->batch().numbatchdl()) {
    bout << "|#9Time - |#2" << ctim(a()->batch().dl_time_in_secs());
  }
  bout.nl(2);
  int current_num = 0;
  for (const auto& b : a()->batch().entry) {
    if (abort || a()->hangup_) {
      break;
    }
    string buffer;
    ++current_num;
    if (b.sending) {
      const string t = ctim(std::lround(b.time));
      buffer = StringPrintf("%d. %s %s   %s  %s", current_num, "(D)",
          b.filename, t.c_str(), a()->directories[b.dir].name);
    } else {
      buffer = StringPrintf("%d. %s %s             %s", current_num, "(U)",
          b.filename, a()->directories[b.dir].name);
    }
    bout.bpla(buffer, &abort);
  }
  bout.nl();
}

std::vector<batchrec>::iterator delbatch(std::vector<batchrec>::iterator it) {
  return a()->batch().entry.erase(it);
}

// Deletes one item (index i) from the d/l batch queue.
void delbatch(int num) {
  if (num >= size_int(a()->batch().entry)) {
    return;
  }
  erase_at(a()->batch().entry, static_cast<size_t>(num));
}

static void downloaded(const string& file_name, long lCharsPerSecond) {
  uploadsrec u;

  for (auto it = begin(a()->batch().entry); it != end(a()->batch().entry); it++) {
    const auto& b = *it;
    if (file_name == b.filename && b.sending) {
      dliscan1(b.dir);
      int nRecNum = recno(b.filename);
      if (nRecNum > 0) {
        File file(a()->download_filename_);
        file.Open(File::modeReadWrite | File::modeBinary | File::modeCreateFile);
        FileAreaSetRecord(file, nRecNum);
        file.Read(&u, sizeof(uploadsrec));
        a()->user()->SetFilesDownloaded(a()->user()->GetFilesDownloaded() + 1);
        a()->user()->SetDownloadK(a()->user()->GetDownloadK() +
            static_cast<int>(bytes_to_k(u.numbytes)));
        ++u.numdloads;
        FileAreaSetRecord(file, nRecNum);
        file.Write(&u, sizeof(uploadsrec));
        file.Close();
        if (lCharsPerSecond) {
          sysoplog() << "Downloaded '" << u.filename << "' (" << lCharsPerSecond << " cps).";
        } else {
          sysoplog() << "Downloaded '" << u.filename << "'.";
        }
        if (a()->config()->sysconfig_flags() & sysconfig_log_dl) {
          User user;
          a()->users()->readuser(&user, u.ownerusr);
          if (!user.IsUserDeleted()) {
            if (date_to_daten(user.GetFirstOn()) < u.daten) {
              const string user_name_number = a()->names()->UserName(a()->usernum);
              ssm(u.ownerusr) << user_name_number << " downloaded|#1 \"" << u.filename << 
                "\" |#7on " << fulldate();
            }
          }
        }
      }
      it = delbatch(it);
      return;
    }
  }
  sysoplog() << "!!! Couldn't find \"" << file_name << "\" in DL batch queue.";
}

void didnt_upload(const batchrec& b) {
  uploadsrec u;

  if (b.sending) {
    return;
  }

  dliscan1(b.dir);
  int nRecNum = recno(b.filename);
  if (nRecNum > 0) {
    File file(a()->download_filename_);
    file.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite, File::shareDenyNone);
    do {
      FileAreaSetRecord(file, nRecNum);
      file.Read(&u, sizeof(uploadsrec));
      if (u.numbytes != 0) {
        file.Close();
        nRecNum = nrecno(b.filename, nRecNum);
        file.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite, File::shareDenyNone);
      }
    } while (nRecNum != -1 && u.numbytes != 0);

    if (nRecNum != -1 && u.numbytes == 0) {
      if (u.mask & mask_extended) {
        delete_extended_description(u.filename);
      }
      for (int i1 = nRecNum; i1 < a()->numf; i1++) {
        FileAreaSetRecord(file, i1 + 1);
        file.Read(&u, sizeof(uploadsrec));
        FileAreaSetRecord(file, i1);
        file.Write(&u, sizeof(uploadsrec));
      }
      --nRecNum;
      --a()->numf;
      FileAreaSetRecord(file, 0);
      file.Read(&u, sizeof(uploadsrec));
      u.numbytes = a()->numf;
      FileAreaSetRecord(file, 0);
      file.Write(&u, sizeof(uploadsrec));
      return;
    }
  }
  sysoplog() << StringPrintf("!!! Couldn't find \"%s\" in transfer area.", b.filename);
}

static void uploaded(const string& file_name, long lCharsPerSecond) {
  uploadsrec u;

  for (auto it = begin(a()->batch().entry); it != end(a()->batch().entry); it++) {
    const auto& b = *it;
    if (file_name == b.filename && !b.sending) {
      dliscan1(b.dir);
      int nRecNum = recno(b.filename);
      if (nRecNum > 0) {
        File downFile(a()->download_filename_);
        downFile.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite);
        do {
          FileAreaSetRecord(downFile, nRecNum);
          downFile.Read(&u, sizeof(uploadsrec));
          if (u.numbytes != 0) {
            nRecNum = nrecno(b.filename, nRecNum);
          }
        } while (nRecNum != -1 && u.numbytes != 0);
        downFile.Close();
        if (nRecNum != -1 && u.numbytes == 0) {
          auto source_filename = FilePath(a()->batch_directory(), file_name);
          auto dest_filename = FilePath(a()->directories[b.dir].path, file_name);
          if (source_filename != dest_filename && File::Exists(source_filename)) {
            bool found = false;
            if (source_filename[1] != ':' && dest_filename[1] != ':') {
              found = true;
            }
            if (source_filename[1] == ':' && dest_filename[1] == ':' && source_filename[0] == dest_filename[0]) {
              found = true;
            }
            if (found) {
              File::Rename(source_filename, dest_filename);
              File::Remove(source_filename);
            } else {
              copyfile(source_filename, dest_filename, false);
              File::Remove(source_filename);
            }
          }
          File file(dest_filename);
          if (file.Open(File::modeBinary | File::modeReadOnly)) {
            if (!a()->upload_cmd.empty()) {
              file.Close();
              if (!check_ul_event(b.dir, &u)) {
                didnt_upload(b);
              } else {
                file.Open(File::modeBinary | File::modeReadOnly);
              }
            }
            if (file.IsOpen()) {
              u.numbytes = file.length();
              file.Close();
              get_file_idz(&u, b.dir);
              a()->user()->SetFilesUploaded(a()->user()->GetFilesUploaded() + 1);
              add_to_file_database(u.filename);
              a()->user()->SetUploadK(a()->user()->GetUploadK() +
                  static_cast<int>(bytes_to_k(u.numbytes)));
              a()->status_manager()->Run([](WStatus& s) {
                s.IncrementNumUploadsToday();
                s.IncrementFileChangedFlag(WStatus::fileChangeUpload);
              });
              File fileDn(a()->download_filename_);
              fileDn.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite);
              FileAreaSetRecord(fileDn, nRecNum);
              fileDn.Write(&u, sizeof(uploadsrec));
              fileDn.Close();
              sysoplog() << StringPrintf("+ \"%s\" uploaded on %s (%ld cps)", u.filename, a()->directories[b.dir].name, lCharsPerSecond);
              bout << "Uploaded '" << u.filename << "' to "  << a()->directories[b.dir].name 
                   << " (" << lCharsPerSecond << " cps)" << wwiv::endl;
            }
          }
          it = delbatch(it);
          return;
        }
      }
      it = delbatch(it);
      if (try_to_ul(file_name)) {
        sysoplog() << StringPrintf("!!! Couldn't find file \"%s\" in directory.", file_name.c_str());
        bout << "Deleting - couldn't find data for file " << file_name << wwiv::endl;
      }
      return;
    }
  }
  if (try_to_ul(file_name)) {
    sysoplog() << StringPrintf("!!! Couldn't find \"%s\" in UL batch queue.", file_name.c_str());
    bout << "Deleting - don't know what to do with file " << file_name << wwiv::endl;

    File::Remove(a()->batch_directory(), file_name);
  }
}

// This function returns one character from either the local keyboard or
// remote com port (if applicable).  Every second of inactivity, a
// beep is sounded.  After 10 seconds of inactivity, the user is hung up.
static void bihangup() {
  bout.dump();
  auto batch_lastchar = std::chrono::steady_clock::now();
  auto nextbeep = std::chrono::seconds(1);
  bout << "\r\n|#2Automatic disconnect in progress.\r\n";
  bout << "|#2Press 'H' to a()->hangup_, or any other key to return to system.\r\n";

  unsigned char ch = 0;
  do {
    while (!bkbhit() && !a()->hangup_) {
      auto dd = std::chrono::steady_clock::now();
      if ((dd - batch_lastchar) > nextbeep) {
        nextbeep += std::chrono::seconds(1);
      }
      if ((dd - batch_lastchar) > std::chrono::seconds(10)) {
        bout.nl();
        bout << "Thank you for calling.";
        bout.nl();
        a()->remoteIO()->disconnect();
        Hangup();
      }
      giveup_timeslice();
      CheckForHangup();
    }
    ch = bout.getkey();
    if (ch == 'h' || ch == 'H') {
      Hangup();
    }
  } while (!ch && !a()->hangup_);
}

void zmbatchdl(bool bHangupAfterDl) {
  int cur = 0;
  uploadsrec u;

  if (!a()->context().incom()) {
    return;
  }

  string message = StrCat("ZModem Download: Files - ", a()->batch().entry.size(), 
    " Time - ", ctim(a()->batch().dl_time_in_secs()));
  if (bHangupAfterDl) {
    message += ", HAD";
  }
  sysoplog() << message;
  bout.nl();
  bout << message;
  bout.nl(2);

  bool bRatioBad = false;
  bool ok = true;
  // TODO(rushfan): Rewrite this to use iterators;
  do {
    a()->tleft(true);
    if ((a()->config()->req_ratio() > 0.0001) && (ratio() < a()->config()->req_ratio())) {
      bRatioBad = true;
    }
    if (a()->user()->IsExemptRatio()) {
      bRatioBad = false;
    }
    if (!a()->batch().entry[cur].sending) {
      bRatioBad = false;
      ++cur;
    }
    if ((nsl() >= a()->batch().entry[cur].time) && !bRatioBad) {
      dliscan1(a()->batch().entry[cur].dir);
      int nRecordNumber = recno(a()->batch().entry[cur].filename);
      if (nRecordNumber <= 0) {
        delbatch(cur);
      } else {
        a()->localIO()->Puts(
            StrCat("Files left - ", a()->batch().entry.size(), ", Time left - ",
              ctim(a()->batch().dl_time_in_secs()), "\r\n"));
        File file(a()->download_filename_);
        file.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite);
        FileAreaSetRecord(file, nRecordNumber);
        file.Read(&u, sizeof(uploadsrec));
        file.Close();
        auto send_filename =
            FilePath(a()->directories[a()->batch().entry[cur].dir].path, u.filename);
        if (a()->directories[a()->batch().entry[cur].dir].mask & mask_cdrom) {
          auto orig_filename =
              FilePath(a()->directories[a()->batch().entry[cur].dir].path, u.filename);
          // update the send filename and copy it from the cdrom
          send_filename = FilePath(a()->temp_directory(), u.filename);
          if (!File::Exists(send_filename)) {
            copyfile(orig_filename, send_filename, true);
          }
        }
        write_inst(INST_LOC_DOWNLOAD, a()->current_user_dir().subnum, INST_FLAGS_NONE);
        StringRemoveWhitespace(&send_filename);
        double percent;
        zmodem_send(send_filename, &ok, &percent);
        if (ok) {
          downloaded(u.filename, 0);
        }
      }
    } else {
      delbatch(cur);
    }
  } while (ok && !a()->hangup_ && size_int(a()->batch().entry) > cur && !bRatioBad);

  if (ok && !a()->hangup_) {
    endbatch();
  }
  if (bRatioBad) {
    bout << "\r\nYour ratio is too low to continue the transfer.\r\n\n\n";
  }
  if (bHangupAfterDl) {
    bihangup();
  }
}

void ymbatchdl(bool bHangupAfterDl) {
  int cur = 0;
  uploadsrec u;

  if (!a()->context().incom()) {
    return;
  }
  string message = StrCat("Ymodem Download: Files - ", a()->batch().entry.size(),
    ", Time - ", ctim(a()->batch().dl_time_in_secs()));
  if (bHangupAfterDl) {
    message += ", HAD";
  }
  sysoplog() << message;
  bout.nl();
  bout << message;
  bout.nl(2);

  bool bRatioBad = false;
  bool ok = true;
  //TODO(rushfan): rewrite to use iterators.
  do {
    a()->tleft(true);
    if ((a()->config()->req_ratio() > 0.0001) && (ratio() < a()->config()->req_ratio())) {
      bRatioBad = true;
    }
    if (a()->user()->IsExemptRatio()) {
      bRatioBad = false;
    }
    if (!a()->batch().entry[cur].sending) {
      bRatioBad = false;
      ++cur;
    }
    if ((nsl() >= a()->batch().entry[cur].time) && !bRatioBad) {
      dliscan1(a()->batch().entry[cur].dir);
      int nRecordNumber = recno(a()->batch().entry[cur].filename);
      if (nRecordNumber <= 0) {
        delbatch(cur);
      } else {
        a()->localIO()->Puts(
			      StrCat("Files left - ", a()->batch().entry.size(),
              ", Time left - ", ctim(a()->batch().dl_time_in_secs()),"\r\n"));
        File file(a()->download_filename_);
        file.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite);
        FileAreaSetRecord(file, nRecordNumber);
        file.Read(&u, sizeof(uploadsrec));
        file.Close();
        auto send_filename =
            FilePath(a()->directories[a()->batch().entry[cur].dir].path, u.filename);
        if (a()->directories[a()->batch().entry[cur].dir].mask & mask_cdrom) {
          auto orig_filename =
              FilePath(a()->directories[a()->batch().entry[cur].dir].path, u.filename);
          send_filename = FilePath(a()->temp_directory(), u.filename);
          if (!File::Exists(send_filename)) {
            copyfile(orig_filename, send_filename, true);
          }
        }
        write_inst(INST_LOC_DOWNLOAD, a()->current_user_dir().subnum, INST_FLAGS_NONE);
        double percent;
        xymodem_send(send_filename, &ok, &percent, true, true, true);
        if (ok) {
          downloaded(u.filename, 0);
        }
      }
    } else {
      delbatch(cur);
    }
  } while (ok && !a()->hangup_ && size_int(a()->batch().entry) > cur && !bRatioBad);

  if (ok && !a()->hangup_) {
    endbatch();
  }
  if (bRatioBad) {
    bout << "\r\nYour ratio is too low to continue the transfer.\r\n\n";
  }
  if (bHangupAfterDl) {
    bihangup();
  }
}

static void handle_dszline(char *l) {
  long lCharsPerSecond = 0;

  // find the filename
  char* ss = strtok(l, " \t");
  for (int i = 0; i < 10 && ss; i++) {
    switch (i) {
    case 4:
      lCharsPerSecond = to_number<long>(ss);
      break;
    }
    ss = strtok(nullptr, " \t");
  }

  if (ss) {
    const auto  filename = aligns(stripfn(ss));
    
    switch (*l) {
    case 'Z':
    case 'r':
    case 'R':
    case 'B':
    case 'H':
      // received a file
      uploaded(filename, lCharsPerSecond);
      break;
    case 'z':
    case 's':
    case 'S':
    case 'b':
    case 'h':
    case 'Q':
      // sent a file
      downloaded(filename, lCharsPerSecond);
      break;
    case 'E':
    case 'e':
    case 'L':
    case 'l':
    case 'U':
      // error
      sysoplog() << StringPrintf("Error transferring \"%s\"", ss);
      break;
    }
  }
}

static double ratio1(unsigned long xa) {
  if (a()->user()->GetDownloadK() == 0 && a == 0) {
    return 99.999;
  }
  double r = static_cast<float>(a()->user()->GetUploadK()) /
             static_cast<float>(a()->user()->GetDownloadK() + xa);
  return std::min<double>(r, 99.998);
}

static string make_ul_batch_list() {
  const auto fn = StringPrintf("%s.%3.3u", FILESUL_NOEXT, a()->instance_number());
  // TODO(rushfan): This should move to a temp directory.
  const auto list_filename = FilePath(a()->GetHomeDir(), fn);

  File::SetFilePermissions(list_filename, File::permReadWrite);
  File::Remove(list_filename);

  File fileList(list_filename);
  if (!fileList.Open(File::modeBinary | File::modeCreateFile | File::modeTruncate | File::modeReadWrite)) {
    return "";
  }
  for (const auto& b : a()->batch().entry) {
    if (!b.sending) {
      File::set_current_directory(a()->directories[b.dir].path);
      File file(FilePath(File::current_directory(), stripfn(b.filename)));
      a()->CdHome();
      fileList.Write(StrCat(file.full_pathname(), "\r\n"));
    }
  }
  fileList.Close();
  return list_filename;
}

static string make_dl_batch_list() {
  const auto fn = StringPrintf("%s.%3.3u", FILESDL_NOEXT, a()->instance_number());
  const auto list_filename = FilePath(a()->GetHomeDir(), fn);

  File::SetFilePermissions(list_filename, File::permReadWrite);
  File::Remove(list_filename);

  File fileList(list_filename);
  if (!fileList.Open(File::modeBinary | File::modeCreateFile | File::modeTruncate | File::modeReadWrite)) {
    return "";
  }

  double at = 0.0;
  unsigned long addk = 0;
  for (const auto& b : a()->batch().entry) {
    if (b.sending) {
      string filename_to_send;
      if (a()->directories[b.dir].mask & mask_cdrom) {
        File::set_current_directory(a()->temp_directory());
        const string current_dir = File::current_directory();
        File fileToSend(FilePath(current_dir, stripfn(b.filename)));
        if (!fileToSend.Exists()) {
          File::set_current_directory(a()->directories[b.dir].path);
          File sourceFile(FilePath(File::current_directory(), stripfn(b.filename)));
          copyfile(sourceFile.full_pathname(), fileToSend.full_pathname(), true);
        }
        filename_to_send = fileToSend.full_pathname();
      } else {
        File::set_current_directory(a()->directories[b.dir].path);
        File fileToSend(FilePath(File::current_directory(), stripfn(b.filename)));
        filename_to_send = fileToSend.full_pathname();
      }
      bool ok = true;
      a()->CdHome();
      if (nsl() < (b.time + at)) {
        ok = false;
        bout << "Cannot download " << b.filename << ": Not enough time" << wwiv::endl;
      }
      unsigned long thisk = bytes_to_k(b.len);
      if ((a()->config()->req_ratio() > 0.0001) &&
          (ratio1(addk + thisk) < a()->config()->req_ratio()) &&
          !a()->user()->IsExemptRatio()) {
        ok = false;
        bout << "Cannot download " << b.filename << ": Ratio too low" << wwiv::endl;
      }
      if (ok) {
        fileList.Write(StrCat(filename_to_send, "\r\n"));
        at += b.time;
        addk += thisk;
      }
    }
  }
  fileList.Close();
  return list_filename;
}

void ProcessDSZLogFile() {
  char **lines = static_cast<char **>(calloc((a()->max_batch * sizeof(char *) * 2) + 1, 1));
  WWIV_ASSERT(lines != nullptr);

  if (!lines) {
    return;
  }

  File fileDszLog(a()->dsz_logfile_name_);
  if (fileDszLog.Open(File::modeBinary | File::modeReadOnly)) {
    auto nFileSize = fileDszLog.length();
    char *ss = static_cast<char *>(calloc(nFileSize + 1, 1));
    WWIV_ASSERT(ss != nullptr);
    if (ss) {
      auto nBytesRead = fileDszLog.Read(ss, nFileSize);
      if (nBytesRead > 0) {
        ss[nBytesRead] = 0;
        lines[0] = strtok(ss, "\r\n");
        for (int i = 1; (i < a()->max_batch * 2 - 2) && (lines[i - 1]); i++) {
          lines[i] = strtok(nullptr, "\n");
        }
        lines[a()->max_batch * 2 - 2] = nullptr;
        for (int i1 = 0; lines[i1]; i1++) {
          handle_dszline(lines[i1]);
        }
      }
      free(ss);
    }
    fileDszLog.Close();
  }
  free(lines);
}

static void run_cmd(const string& orig_commandline, const string& downlist, const string& uplist, const string& dl, bool bHangupAfterDl) {
  string commandLine = stuff_in(orig_commandline,
      std::to_string(std::min<int>(a()->modem_speed_, 57600)), 
      std::to_string(a()->primary_port()),
      downlist, 
      std::to_string(std::min<int>(a()->modem_speed_, 57600)), 
      uplist);

  if (!commandLine.empty()) {
    make_abs_cmd(a()->bbsdir(), &commandLine);
    a()->Cls();
    const string user_name_number = a()->names()->UserName(a()->usernum);
    const string message = StringPrintf(
        "%s is currently online at %u bps\r\n\r\n%s\r\n%s\r\n",
        user_name_number.c_str(),
        a()->modem_speed_, dl.c_str(), commandLine.c_str());
    a()->localIO()->Puts(message);
    if (a()->context().incom()) {
      File::SetFilePermissions(a()->dsz_logfile_name_, File::permReadWrite);
      File::Remove(a()->dsz_logfile_name_);
      File::set_current_directory(a()->batch_directory());
      ExecuteExternalProgram(commandLine, a()->spawn_option(SPAWNOPT_PROT_BATCH));
      if (bHangupAfterDl) {
        bihangup();
      } else {
        bout << "\r\n|#9Please wait...\r\n\n";
      }
      ProcessDSZLogFile();
      a()->UpdateTopScreen();
    }
  }
  if (!downlist.empty()) {
    File::SetFilePermissions(downlist, File::permReadWrite);
    File::Remove(downlist);
  }
  if (!uplist.empty()) {
    File::SetFilePermissions(uplist, File::permReadWrite);
    File::Remove(uplist);
  }
}


void dszbatchdl(bool bHangupAfterDl, char *command_line, char *description) {
  string download_log_entry = StrCat(description,
      "%s BATCH Download: Files - ", a()->batch().entry.size(), 
      ", Time - ", ctim(a()->batch().dl_time_in_secs()));
  if (bHangupAfterDl) {
    download_log_entry += ", HAD";
  }
  sysoplog() << download_log_entry;
  bout.nl();
  bout << download_log_entry;
  bout.nl(2);

  write_inst(INST_LOC_DOWNLOAD, a()->current_user_dir().subnum, INST_FLAGS_NONE);
  const string list_filename = make_dl_batch_list();
  run_cmd(command_line, list_filename, "", download_log_entry, bHangupAfterDl);
}

static void dszbatchul(bool bHangupAfterDl, char *command_line, char *description) {
  string download_log_entry = StringPrintf("%s BATCH Upload: Files - %d", description,
          a()->batch().entry.size());
  if (bHangupAfterDl) {
    download_log_entry += ", HAD";
  }
  sysoplog() << download_log_entry;
  bout.nl();
  bout << download_log_entry;
  bout.nl(2);

  write_inst(INST_LOC_UPLOAD, a()->current_user_dir().subnum, INST_FLAGS_NONE);
  string list_filename = make_ul_batch_list();

  auto ti = std::chrono::system_clock::now();
  run_cmd(command_line, "", list_filename, download_log_entry, bHangupAfterDl);
  auto time_used = std::chrono::system_clock::now() - ti;
  a()->user()->add_extratime(time_used);
}

int batchdl(int mode) {
  bool bHangupAfterDl = false;
  bool done = false;
  do {
    char ch = 0;
    switch (mode) {
    case 0:
    case 3:
      bout.nl();
      if (mode == 3) {
        bout << "|#7[|#2L|#7]|#1ist Files, |#7[|#2C|#7]|#1lear Queue, |#7[|#2R|#7]|#1emove File, |#7[|#2Q|#7]|#1uit or |#7[|#2D|#7]|#1ownload : |#0";
        ch = onek("QLRDC\r");
      } else {
        bout << "|#9Batch: L,R,Q,C,D,U,? : ";
        ch = onek("Q?CLRDU");
      }
      break;
    case 1:
      listbatch();
      ch = 'D';
      break;
    case 2:
      listbatch();
      ch = 'U';
      break;
    }
    switch (ch) {
    case '?':
      printfile(TBATCH_NOEXT);
      break;
    case 'Q':
      if (mode == 3) {
        return 1;
      }
      done = true;
      break;
    case 'L':
      listbatch();
      break;
    case 'R': {
      bout.nl();
      bout << "|#9Remove which? ";
      string s = input(4);
      auto i = to_number<int>(s);
      if (i > 0 && i <= size_int(a()->batch().entry)) {
        didnt_upload(a()->batch().entry[i-1]);
        delbatch(i-1);
      }
      if (a()->batch().entry.empty()) {
        bout << "\r\nBatch queue empty.\r\n\n";
        done = true;
      }
    } break;
    case 'C':
      bout << "|#5Clear queue? ";
      if (yesno()) {
        for (const auto& b : a()->batch().entry) { didnt_upload(b); }
        a()->batch().entry.clear();
        done = true;
        bout << "Queue cleared.\r\n";
        if (mode == 3) {
          return 1;
        }
      }
      done = true;
      break;
    case 'U': {
      if (mode != 3) {
        bout.nl();
        bout << "|#5Hang up after transfer? ";
        bHangupAfterDl = yesno();
        bout.nl(2);
        int i = get_protocol(xf_up_batch);
        if (i > 0) {
          dszbatchul(bHangupAfterDl, a()->externs[i - WWIV_NUM_INTERNAL_PROTOCOLS].receivebatchfn,
                     a()->externs[i - WWIV_NUM_INTERNAL_PROTOCOLS].description);
          if (!bHangupAfterDl) {
            bout.bprintf("Your ratio is now: %-6.3f\r\n", ratio());
          }
        }
        done = true;
      }
    } break;
    case 'D':
    case 13:
      if (mode != 3) {
        if (a()->batch().numbatchdl() == 0) {
          bout << "\r\nNothing in batch download queue.\r\n\n";
          done = true;
          break;
        }
        bout.nl();
        if (!ratio_ok()) {
          bout << "\r\nSorry, your ratio is too low.\r\n\n";
          done = true;
          break;
        }
        bout << "|#5Hang up after transfer? ";
        bHangupAfterDl = yesno();
        bout.nl();
        int i = get_protocol(xf_down_batch);
        if (i > 0) {
          if (i == WWIV_INTERNAL_PROT_YMODEM) {
            if (a()->over_intern.size() > 0 
                && (a()->over_intern[2].othr & othr_override_internal) 
                && (a()->over_intern[2].sendbatchfn[0])) {
              dszbatchdl(bHangupAfterDl, a()->over_intern[2].sendbatchfn, prot_name(4));
            } else {
              ymbatchdl(bHangupAfterDl);
            }
          } else if (i == WWIV_INTERNAL_PROT_ZMODEM) {
            zmbatchdl(bHangupAfterDl);
          } else {
            dszbatchdl(bHangupAfterDl, a()->externs[i - WWIV_NUM_INTERNAL_PROTOCOLS].sendbatchfn,
                       a()->externs[i - WWIV_NUM_INTERNAL_PROTOCOLS].description);
          }
          if (!bHangupAfterDl) {
            bout.nl();
            bout.bprintf("Your ratio is now: %-6.3f\r\n", ratio());
          }
        }
      }
      done = true;
      break;
    }
  } while (!done && !a()->hangup_);
  return 0;
}

void upload(int dn) {
  dliscan1(dn);
  auto d = a()->directories[dn];
  long free_space = File::freespace_for_path(d.path);
  if (free_space < 100) {
    bout << "\r\nNot enough disk space to upload here.\r\n\n";
    return;
  }
  listbatch();
  bout << "Upload - " << free_space << "k free.";
  bout.nl();
  printfile(TRY2UL_NOEXT);

  bool done = false;
  do {
    bout << "|#2B|#7) |#1Blind batch upload\r\n";
    bout << "|#2N|#7) |#1Normal upload\r\n";
    bout << "|#2Q|#7) |#1Quit\r\n\n";
    bout << "|#2Which |#7(|#2B,n,q,?|#7)|#1: ";

    char key = onek("QB\rN?");
    switch (key) {
    case 'B':
    case '\r':
      batchdl(2);
      done = true;
      break;
    case 'Q':
      done = true;
      break;
    case 'N':
      normalupload(dn);
      done = true;
      break;
    case '?':
      bout << "This is help?";
      bout.nl();
      pausescr();
      done = false;
      break;
    }
  } while (!done && !a()->hangup_);
}

static char *unalign_char(char *file_name) {
  char* temp = strstr(file_name, " ");
  if (temp) {
    *temp++ = '\0';
    char* temp2 = strstr(temp, ".");
    if (temp2) {
      strcat(file_name, temp2);
    }
  }
  return file_name;
}

std::string unalign(const std::string& file_name) { 
  char s[1025];
  to_char_array(s, file_name);
  const auto r = unalign_char(s);
  return ToStringLowerCase(r);
}


bool Batch::delbatch(size_t pos) {
  if (pos >= entry.size()) {
    return false;
  }
  return erase_at(entry, pos);
}

long Batch::dl_time_in_secs() const {
  size_t r = 0;  
  for (const auto& e : entry) { 
    if (e.sending) {
      r += e.len;
    }
  }

  auto t =(12.656 * r) / a()->modem_speed_;
  return std::lround(t);
}
