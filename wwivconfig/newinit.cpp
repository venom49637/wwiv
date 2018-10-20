/**************************************************************************/
/*                                                                        */
/*                  WWIV Initialization Utility Version 5                 */
/*               Copyright (C)2014-2017, WWIV Software Services           */
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
#include "wwivconfig/newinit.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif
#include <string>
#include <sys/stat.h>
#include <vector>

#include "core/datafile.h"
#include "core/file.h"
#include "core/strings.h"
#include "core/textfile.h"
#include "core/version.h"
#include "core/wwivport.h"
#include "localui/input.h"
#include "localui/ui_win.h"
#include "localui/wwiv_curses.h"
#include "wwivconfig/archivers.h"
#include "wwivconfig/utility.h"
#include "wwivconfig/wwivconfig.h"
#include "sdk/vardec.h"

#include "core/datetime.h"
#include "sdk/filenames.h"
#include "sdk/subxtr.h"
#include "sdk/user.h"
#include "sdk/wwivcolors.h"

using std::string;
using std::vector;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

static void write_qscn(const Config& config, unsigned int un, uint32_t* qscn) {
  File file(FilePath(config.datadir(), USER_QSC));
  if (file.Open(File::modeReadWrite | File::modeBinary | File::modeCreateFile)) {
    file.Seek(config.qscn_len() * un, File::Whence::begin);
    file.Write(qscn, config.qscn_len());
    file.Close();
  }
}

static bool unzip_file(UIWindow* window, const std::string& zipfile, const std::string& dir) {
  if (File::Exists(zipfile)) {
    window->SetColor(SchemeId::NORMAL);
    window->Puts(StrCat("Decompressing file: ", zipfile, "\n"));
    const auto unzip_cmd = StrCat("unzip -qq -o ", zipfile, " -d", dir);
    auto rc = system(unzip_cmd.c_str());
    if (rc != 0) {
      window->SetColor(SchemeId::ERROR_TEXT);
      window->Puts("ERROR Unable to unzip file.\n");
    }

    const auto sysop_dir = FilePath("dloads", "sysop");
    File::Rename(zipfile, FilePath(sysop_dir, zipfile));
    return true;
  }
  return false;
}

static void save_config(const configrec& c) {
  DataFile<configrec> file(CONFIG_DAT,
                           File::modeBinary | File::modeReadWrite | File::modeCreateFile);
  if (file) {
    file.Write(&c);
  }
}

static void init_files(UIWindow* window, const string& bbsdir, bool unzip_files) {
  window->SetColor(SchemeId::PROMPT);
  window->Puts("Creating Data Files.\n");
  window->SetColor(SchemeId::NORMAL);

  configrec cfg430{};
  memset(&cfg430, 0, sizeof(configrec));

  // Set header
  cfg430.header.header.config_revision_number = 1;
  cfg430.header.header.config_size = sizeof(configrec);
  cfg430.header.header.written_by_wwiv_num_version = wwiv_num_version;
  to_char_array(cfg430.header.header.signature, "WWIV");

  to_char_array(cfg430.datadir, "data");

  to_char_array(cfg430.systempw, "SYSOP");
  to_char_array(cfg430.systemname, "My WWIV BBS");
  to_char_array(cfg430.systemphone, "   -   -    ");
  to_char_array(cfg430.sysopname, "The New Sysop");

  to_char_array(cfg430.msgsdir, "msgs");
  to_char_array(cfg430.gfilesdir, "gfiles");
  to_char_array(cfg430.dloadsdir, "dloads");
  to_char_array(cfg430.tempdir, FilePath("temp", "1"));
  to_char_array(cfg430.menudir, FilePath("gfiles", "menus"));
  to_char_array(cfg430.scriptdir, "scripts");

  cfg430.newusersl = 10;
  cfg430.newuserdsl = 0;
  cfg430.maxwaiting = 50;
  // Always use 1 for the primary port.
  cfg430.primaryport = 1;
  cfg430.newuploads = 0;
  cfg430.maxusers = 500;
  cfg430.newuser_restrict = restrict_validate;
  cfg430.req_ratio = 0.0;
  cfg430.newusergold = 100.0;

  valrec v{};
  v.ar = 0;
  v.dar = 0;
  v.restrict = 0;
  v.sl = 10;
  v.dsl = 0;
  for (int i = 0; i < 10; i++) {
    cfg430.autoval[i] = v;
  }
  for (int i = 0; i < 256; i++) {
    slrec sl{};
    sl.time_per_logon = static_cast<uint16_t>((i / 10) * 10);
    sl.time_per_day = static_cast<uint16_t>(((float)sl.time_per_logon) * 2.5);
    sl.messages_read = static_cast<uint16_t>((i / 10) * 100);
    if (i < 10) {
      sl.emails = 0;
    } else if (i <= 19) {
      sl.emails = 5;
    } else {
      sl.emails = 20;
    }

    if (i <= 10) {
      sl.posts = 10;
    } else if (i <= 25) {
      sl.posts = 10;
    } else if (i <= 39) {
      sl.posts = 4;
    } else if (i <= 79) {
      sl.posts = 10;
    } else {
      sl.posts = 25;
    }

    sl.ability = 0;
    if (i >= 150) {
      sl.ability |= ability_cosysop;
    }
    if (i >= 100) {
      sl.ability |= ability_limited_cosysop;
    }
    if (i >= 90) {
      sl.ability |= ability_read_email_anony;
    }
    if (i >= 80) {
      sl.ability |= ability_read_post_anony;
    }
    if (i >= 70) {
      sl.ability |= ability_email_anony;
    }
    if (i >= 60) {
      sl.ability |= ability_post_anony;
    }
    if (i == 255) {
      sl.time_per_logon = 255;
      sl.time_per_day = 255;
      sl.posts = 255;
      sl.emails = 255;
    }
    cfg430.sl[i] = sl;
  }

  cfg430.userreclen = sizeof(userrec);
  cfg430.waitingoffset = offsetof(userrec, waiting);
  cfg430.inactoffset = offsetof(userrec, inact);
  cfg430.sysstatusoffset = offsetof(userrec, sysstatus);
  cfg430.fuoffset = offsetof(userrec, forwardusr);
  cfg430.fsoffset = offsetof(userrec, forwardsys);
  cfg430.fnoffset = offsetof(userrec, net_num);

  cfg430.max_subs = 64;
  cfg430.max_dirs = 64;
  cfg430.qscn_len =
      4 * (1 + cfg430.max_subs + ((cfg430.max_subs + 31) / 32) + ((cfg430.max_dirs + 31) / 32));

  cfg430.post_call_ratio = 0.0;
  save_config(cfg430);

  // cfg430 is done
  Config config(cfg430);

  const auto datadir = FilePath(bbsdir, "data");
  create_arcs(window, datadir);
  statusrec_t statusrec{};
  memset(&statusrec, 0, sizeof(statusrec_t));
  auto now = date();
  to_char_array(statusrec.date1, now);
  to_char_array(statusrec.date2, "00/00/00");
  to_char_array(statusrec.date3, "00/00/00");
  to_char_array(statusrec.log1, "000000.log");
  to_char_array(statusrec.log2, "000000.log");
  to_char_array(statusrec.gfiledate, now);
  statusrec.callernum = 65535;
  statusrec.qscanptr = 2;
  statusrec.net_bias = 0.001f;
  statusrec.net_req_free = 3.0;

  auto qsc = std::make_unique<uint32_t[]>(config.qscn_len() / sizeof(uint32_t));

  save_status(datadir, statusrec);
  userrec u = {};
  memset(&u, 0, sizeof(u));
  write_user(config, 0, &u);
  write_qscn(config, 0, qsc.get());

  // Note: this is where wwivconfig makes a user record #1 that is deleted for new installs.
  // TODO(rushfan): We should use User::CreateNewUserRecord here.
  u.inact = inact_deleted;
  memset(u.lp_colors, static_cast<uint8_t>(Color::CYAN), sizeof(u.lp_colors));
  u.lp_colors[0] = static_cast<uint8_t>(Color::LIGHTGREEN);
  u.lp_colors[1] = static_cast<uint8_t>(Color::LIGHTGREEN);
  u.lp_colors[2] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[3] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[4] = static_cast<uint8_t>(Color::LIGHTCYAN);
  u.lp_colors[5] = static_cast<uint8_t>(Color::LIGHTCYAN);
  u.lp_colors[6] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[7] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[8] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[9] = static_cast<uint8_t>(Color::CYAN);
  u.lp_colors[10] = static_cast<uint8_t>(Color::LIGHTCYAN);
  u.lp_options = cfl_fname | cfl_extension | cfl_dloads | cfl_kbytes | cfl_description;
  u.hot_keys = 0;
  to_char_array(u.menu_set, "wwiv");

  write_user(config, 1, &u);
  write_qscn(config, 1, qsc.get());
  {
    File namesfile(FilePath("data", NAMES_LST));
    namesfile.Open(File::modeBinary | File::modeReadWrite | File::modeCreateFile);
  }
  {
    subboard_t r = {};
    r.name = "General";
    r.filename = "GENERAL";
    r.readsl = 10;
    r.postsl = 20;
    r.maxmsgs = 50;
    r.storage_type = 2;

    Subs subs("data/", {});
    subs.insert(0, r);
    // TODO(rushfan): Check for error.
    subs.Save();
  }

  {
    directoryrec d1;
    memset(&d1, 0, sizeof(directoryrec));
    to_char_array(d1.name, "Sysop");
    to_char_array(d1.filename, "SYSOP");
    to_char_array(d1.path, FilePath("dloads", StrCat("sysop", File::pathSeparatorString)));
    File::mkdir(d1.path);
    d1.dsl = 100;
    d1.maxfiles = 50;
    d1.type = 65535;
    File dirsfile(FilePath("data", DIRS_DAT));
    dirsfile.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite);
    dirsfile.Write(&d1, sizeof(directoryrec));

    memset(&d1, 0, sizeof(directoryrec));
    to_char_array(d1.name, "Miscellaneous");
    to_char_array(d1.filename, "misc");
    auto last = StrCat("misc", File::pathSeparatorString);
    to_char_array(d1.path, FilePath("dloads", last));
    File::mkdir(d1.path);
    d1.dsl = 10;
    d1.age = 0;
    d1.dar = 0;
    d1.maxfiles = 50;
    d1.mask = 0;
    d1.type = 0;
    dirsfile.Write(&d1, sizeof(directoryrec));
    dirsfile.Close();
  }

  window->Puts(".\n");

  window->SetColor(SchemeId::PROMPT);
  window->Puts("Copying String and Miscellaneous files.\n");
  window->SetColor(SchemeId::NORMAL);

  window->Puts(".\n");

  window->SetColor(SchemeId::PROMPT);
  window->Puts("Decompressing archives.  Please wait");
  window->SetColor(SchemeId::NORMAL);

  if (unzip_files) {
    unzip_file(window, "inifiles.zip", bbsdir);
    unzip_file(window, "gfiles.zip", "gfiles");
    unzip_file(window, "scripts.zip", "scripts");
    unzip_file(window, "data.zip", "data");
    unzip_file(window, "regions.zip", "data");
    unzip_file(window, "zip-city.zip", "data");
  }

  window->SetColor(SchemeId::NORMAL);
}

bool new_init(UIWindow* window, const string& bbsdir, bool unzip_files) {
  static const vector<string> dirnames = {
      "attach", "data",        "data/regions", "data/zip-city", "gfiles",  "gfiles/menus", "msgs",
      "dloads", "dloads/misc", "dloads/sysop", "temp",          "temp/1",  "temp/2",       "temp/3",
      "temp/4", "batch",       "batch/1",      "batch/2",       "batch/3", "batch/4"};
  window->SetColor(SchemeId::PROMPT);
  window->Puts("\n\nNow performing installation.  Please wait...\n\n");
  window->Puts("Creating Directories\n");
  window->SetColor(SchemeId::NORMAL);
  for (const auto& dirname : dirnames) {
    window->SetColor(SchemeId::NORMAL);
    bool chdir_ok = File::set_current_directory(dirname);
    if (!chdir_ok) {
      if (!File::mkdir(dirname)) {
        window->SetColor(SchemeId::ERROR_TEXT);
        window->Puts(StrCat("\n\nERROR!!! Couldn't make '", dirname, "' Sub-Dir.\nExiting..."));
        return false;
      }
    } else {
      File::set_current_directory(bbsdir);
    }
  }

  init_files(window, bbsdir, unzip_files);
  return true;
}
