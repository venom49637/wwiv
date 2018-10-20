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
#include "bbs/multmail.h"

#include <string>

#include "bbs/bbs.h"
#include "bbs/bbsutl.h"
#include "bbs/com.h"
#include "bbs/email.h"
#include "bbs/finduser.h"

#include "bbs/inmsg.h"
#include "bbs/input.h"
#include "bbs/sysoplog.h"
#include "bbs/message_file.h"
#include "bbs/printfile.h"
#include "bbs/utility.h"
#include "local_io/wconstants.h"
#include "core/findfiles.h"
#include "core/strings.h"
#include "core/datetime.h"
#include "sdk/status.h"
#include "sdk/filenames.h"
#include "sdk/user.h"

// local function prototypes
void add_list(int *pnUserNumber, int *numu, int maxu, int allowdup);
int  oneuser();

using std::string;
using std::unique_ptr;
using namespace wwiv::bbs;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

void multimail(int *pnUserNumber, int numu) {
  mailrec m, m1;
  char s[255], s2[81];
  User user;
  memset(&m, 0, sizeof(mailrec));

  if (File::freespace_for_path(a()->config()->msgsdir()) < 10) {
    bout.nl();
    bout << "Sorry, not enough disk space left.\r\n\n";
    return;
  }
  bout.nl();

  MessageEditorData data;
  data.need_title = true;
  if (a()->effective_slrec().ability & ability_email_anony) {
    data.anonymous_flag = anony_enable_anony;
  }
  bout << "|#5Show all recipients in mail? ";
  bool show_all = yesno();
  int j = 0;
  string s1 = StringPrintf("\003""6CC: \003""1");

  m.msg.storage_type = EMAIL_STORAGE;
  a()->context().irt("Multi-Mail");
  File::Remove(QUOTES_TXT);
  data.aux = "email";
  data.fsed_flags = FsedFlags::NOFSED;
  data.to_name = "Multi-Mail";
  data.msged_flags = MSGED_FLAG_NONE;
  if (!inmsg(data)) {
    return;
  }
  savefile(data.text, &m.msg, data.aux);
  strcpy(m.title, data.title.c_str());

  bout <<  "Mail sent to:\r\n";
  sysoplog() << "Multi-Mail to:";

  lineadd(&m.msg, "\003""7----", "email");

  for (int cv = 0; cv < numu; cv++) {
    if (pnUserNumber[cv] < 0) {
      continue;
    }
    a()->users()->readuser(&user, pnUserNumber[cv]);
    if ((user.GetSl() == 255 && (user.GetNumMailWaiting() > static_cast<unsigned int>(a()->config()->max_waiting() * 5))) ||
        ((user.GetSl() != 255) && (user.GetNumMailWaiting() > a()->config()->max_waiting())) ||
        user.GetNumMailWaiting() > 200) {
      bout << a()->names()->UserName(pnUserNumber[cv]) << " mailbox full, not sent.";
      pnUserNumber[cv] = -1;
      continue;
    }
    if (user.IsUserDeleted()) {
      bout << "User deleted, not sent.\r\n";
      pnUserNumber[cv] = -1;
      continue;
    }
    strcpy(s, "  ");
    user.SetNumMailWaiting(user.GetNumMailWaiting() + 1);
    a()->users()->writeuser(&user, pnUserNumber[cv]);
    const string pnunn = a()->names()->UserName(pnUserNumber[cv]);
    strcat(s, pnunn.c_str());
    auto status = a()->status_manager()->BeginTransaction();
    if (pnUserNumber[cv] == 1) {
      status->IncrementNumFeedbackSentToday();
      a()->user()->SetNumFeedbackSentToday(a()->user()->GetNumFeedbackSentToday() + 1);
      a()->user()->SetNumFeedbackSent(a()->user()->GetNumFeedbackSent() + 1);
    } else {
      status->IncrementNumEmailSentToday();
      a()->user()->SetNumEmailSent(a()->user()->GetNumEmailSent() + 1);
      a()->user()->SetNumEmailSentToday(a()->user()->GetNumEmailSentToday() + 1);
    }
    a()->status_manager()->CommitTransaction(std::move(status));
    sysoplog() << s;
    bout << s;
    bout.nl();
    if (show_all) {
      const string pnunn2 = a()->names()->UserName(pnUserNumber[cv]);
      sprintf(s2, "%-22.22s  ", pnunn2.c_str());
      s1.assign(s2);
      j++;
      if (j >= 3) {
        lineadd(&m.msg, s1, "email");
        j = 0;
        s1 = "\003""1    ";
      }
    }
  }
  if (show_all) {
    if (j) {
      lineadd(&m.msg, s1, "email");
    }
  }
  s1 = StringPrintf("\003""2Mail Sent to %d Addresses!", numu);
  lineadd(&m.msg, "\003""7----", "email");
  lineadd(&m.msg, s1, "email");

  m.anony = static_cast<unsigned char>(data.anonymous_flag);
  m.fromsys = 0;
  m.fromuser = static_cast<uint16_t>(a()->usernum);
  m.tosys = 0;
  m.touser = 0;
  m.status = status_multimail;
  m.daten = daten_t_now();

  unique_ptr<File> pFileEmail(OpenEmailFile(true));
  auto len = pFileEmail->length() / sizeof(mailrec);
  int i = 0;
  if (len != 0) {
    i = len - 1;
    pFileEmail->Seek(static_cast<long>(i) * sizeof(mailrec), File::Whence::begin);
    pFileEmail->Read(&m1, sizeof(mailrec));
    while ((i > 0) && (m1.tosys == 0) && (m1.touser == 0)) {
      --i;
      pFileEmail->Seek(static_cast<long>(i) * sizeof(mailrec), File::Whence::begin);
      int i1 = pFileEmail->Read(&m1, sizeof(mailrec));
      if (i1 == -1) {
        bout << "|#6DIDN'T READ WRITE!\r\n";
      }
    }
    if ((m1.tosys) || (m1.touser)) {
      ++i;
    }
  }
  pFileEmail->Seek(static_cast<long>(i) * sizeof(mailrec), File::Whence::begin);
  for (int cv = 0; cv < numu; cv++) {
    if (pnUserNumber[cv] > 0) {
      m.touser = static_cast<uint16_t>(pnUserNumber[cv]);
      pFileEmail->Write(&m, sizeof(mailrec));
    }
  }
  pFileEmail->Close();
}

static char *mml_s;
static int mml_started;

int oneuser() {
  char s[81], *ss;
  int i;
  User user;

  if (mml_s) {
    if (mml_started) {
      ss = strtok(nullptr, "\r\n");
    } else {
      ss = strtok(mml_s, "\r\n");
    }
    mml_started = 1;
    if (ss == nullptr) {
      free(mml_s);
      mml_s = nullptr;
      return -1;
    }
    strcpy(s, ss);
    for (i = 0; s[i] != 0; i++) {
      s[i] = upcase(s[i]);
    }
  } else {
    bout << "|#2>";
    input(s, 40);
  }
  auto user_number_int = finduser1(s);
  if (user_number_int == 65535) {
    return -1;
  }
  if (s[0] == 0) {
    return -1;
  }
  if (user_number_int <= 0) {
    bout.nl();
    bout << "Unknown user.\r\n\n";
    return 0;
  }
  uint16_t user_number = static_cast<uint16_t>(user_number_int);
  uint16_t system_number = 0;
  if (ForwardMessage(&user_number, &system_number)) {
    bout.nl();
    bout << "Forwarded.\r\n\n";
    if (system_number) {
      bout << "Forwarded to another system.\r\n";
      bout << "Can't send multi-mail to another system.\r\n\n";
      return 0;
    }
  }
  if (user_number == 0) {
    bout.nl();
    bout << "Unknown user.\r\n\n";
    return 0;
  }
  a()->users()->readuser(&user, user_number);
  if (((user.GetSl() == 255) && (user.GetNumMailWaiting() > static_cast<unsigned int>(a()->config()->max_waiting() * 5))) ||
      ((user.GetSl() != 255) && (user.GetNumMailWaiting() > a()->config()->max_waiting())) ||
      (user.GetNumMailWaiting() > 200)) {
    bout.nl();
    bout << "Mailbox full.\r\n\n";
    return 0;
  }
  if (user.IsUserDeleted()) {
    bout.nl();
    bout << "Deleted user.\r\n\n";
    return 0;
  }
  bout << "     -> " << a()->names()->UserName(user_number) << wwiv::endl;
  return user_number;
}


void add_list(int *pnUserNumber, int *numu, int maxu, int allowdup) {
  bool done = false;
  int mml = (mml_s != nullptr);
  mml_started = 0;
  while (!done && (*numu < maxu)) {
    int i = oneuser();
    if (mml && (!mml_s)) {
      done = true;
    }
    if (i == -1) {
      done = true;
    } else if (i) {
      if (!allowdup) {
        for (int i1 = 0; i1 < *numu; i1++) {
          if (pnUserNumber[i1] == i) {
            bout.nl();
            bout << "Already in list, not added.\r\n\n";
            i = 0;
          }
          if (i) {
            pnUserNumber[(*numu)++] = i;
          }
        }
      }
    }
  }
  if (*numu == maxu) {
    bout.nl();
    bout << "List full.\r\n\n";
  }
}

#define MAX_LIST 40

void slash_e() {
  int user_number[MAX_LIST], numu, i, i1;
  char s[81], ch, *sss;

  mml_s = nullptr;
  mml_started = 0;
  if (File::freespace_for_path(a()->config()->msgsdir()) < 10) {
    bout.nl();
    bout << "Sorry, not enough disk space left.\r\n\n";
    return;
  }
  if (((a()->user()->GetNumFeedbackSentToday() >= 10) ||
       (a()->user()->GetNumEmailSentToday() >= a()->effective_slrec().emails))
      && (!cs())) {
    bout << "Too much mail sent today.\r\n\n";
    return;
  }
  if (a()->user()->IsRestrictionEmail()) {
    bout << "You can't send mail.\r\n";
    return;
  }
  bool done = false;
  numu = 0;
  do {
    bout.nl(2);
    bout << "|#2Multi-Mail: A,M,D,L,E,Q,? : ";
    ch = onek("QAMDEL?");
    switch (ch) {
    case '?':
      printfile(MMAIL_NOEXT);
      break;
    case 'Q':
      done = true;
      break;
    case 'A':
      bout.nl();
      bout << "Enter names/numbers for users, one per line, max 20.\r\n\n";
      mml_s = nullptr;
      add_list(user_number, &numu, MAX_LIST, so());
      break;
    case 'M': {
      FindFiles ff(FilePath(a()->config()->datadir(), "*.mml"), FindFilesType::any);
      if (ff.empty()) {
        bout.nl();
        bout << "No mailing lists available.\r\n\n";
        break;
      }
      bout.nl();
      bout << "Available mailing lists:\r\n\n";
      for (const auto& f : ff) {
        to_char_array(s, f.name);
        sss = strchr(s, '.');
        if (sss) {
          *sss = 0;
        }
        bout << s;
        bout.nl();
      }

      bout.nl();
      bout << "|#2Which? ";
      input(s, 8);

      File fileMailList(FilePath(a()->config()->datadir(), s));
      if (!fileMailList.Open(File::modeBinary | File::modeReadOnly)) {
        bout.nl();
        bout << "Unknown mailing list.\r\n\n";
      } else {
        i1 = fileMailList.length();
        mml_s = static_cast<char *>(BbsAllocA(i1 + 10L));
        fileMailList.Read(mml_s, i1);
        mml_s[i1] = '\n';
        mml_s[i1 + 1] = 0;
        fileMailList.Close();
        mml_started = 0;
        add_list(user_number, &numu, MAX_LIST, so());
        if (mml_s) {
          free(mml_s);
          mml_s = nullptr;
        }
      }
    }
    break;
    case 'E':
      if (!numu) {
        bout.nl();
        bout << "Need to specify some users first - use A or M\r\n\n";
      } else {
        multimail(user_number, numu);
        done = true;
      }
      break;
    case 'D':
      if (numu) {
        bout.nl();
        bout << "|#2Delete which? ";
        input(s, 2);
        i = to_number<int>(s);
        if ((i > 0) && (i <= numu)) {
          --numu;
          for (i1 = i - 1; i1 < numu; i1++) {
            user_number[i1] = user_number[i1 + 1];
          }
        }
      }
      break;
    case 'L':
      for (i = 0; i < numu; i++) {
        User user;
        a()->users()->readuser(&user, user_number[i]);
        bout << i + 1 << ". " << a()->names()->UserName(user_number[i]) << wwiv::endl;
      }
      break;
    }
    CheckForHangup();
  } while (!done);
}
