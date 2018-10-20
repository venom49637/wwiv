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
#include "bbsovl1.h"

#include <sstream>
#include <string>

#include "bbs/bbs.h"
#include "bbs/bbsutl.h"
#include "bbs/bbsutl1.h"
#include "bbs/com.h"
#include "bbs/conf.h"
#include "bbs/datetime.h"
#include "bbs/email.h"
#include "bbs/external_edit.h"
#include "bbs/input.h"
#include "bbs/instmsg.h"
#include "bbs/message_editor_data.h"
#include "bbs/pause.h"
#include "bbs/quote.h"
#include "bbs/sr.h"
#include "bbs/sysoplog.h"
#include "bbs/utility.h"
#include "local_io/wconstants.h"
#include "bbs/workspace.h"
#include "bbs/xfer.h"
#include "core/strings.h"
#include "sdk/filenames.h"
#include "sdk/status.h"

using std::string;
using namespace wwiv::bbs;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

//////////////////////////////////////////////////////////////////////////////
// Implementation

extern char str_quit[];

/**
 * Displays a horizontal bar of nSize characters in nColor
 * @param nSize Length of the horizontal bar to display
 * @param nColor Color of the horizontal bar to display
 */
void DisplayHorizontalBar(int width, int color) {
  char ch = (okansi()) ? '\xC4' : '-';
  bout.Color(color);
  bout << string(width, ch);
  bout.nl();
}

/**
 * Displays some basic user statistics for the current user.
 */
void YourInfo() {
  bout.cls();
  bout.litebar("Your User Information");
  bout.nl();
  bout << "|#9Your name      : |#2" << a()->names()->UserName(a()->usernum) << wwiv::endl;
  bout << "|#9Phone number   : |#2" << a()->user()->GetVoicePhoneNumber() << wwiv::endl;
  if (a()->user()->GetNumMailWaiting() > 0) {
    bout << "|#9Mail Waiting   : |#2" << a()->user()->GetNumMailWaiting() << wwiv::endl;
  }
  bout << "|#9Security Level : |#2" << a()->user()->GetSl() << wwiv::endl;
  if (a()->effective_sl() != a()->user()->GetSl()) {
    bout << "|#1 (temporarily |#2" << a()->effective_sl() << "|#1)";
  }
  bout.nl();
  bout << "|#9Transfer SL    : |#2" << a()->user()->GetDsl() << wwiv::endl;
  bout << "|#9Date Last On   : |#2" << a()->user()->GetLastOn() << wwiv::endl;
  bout << "|#9Times on       : |#2" << a()->user()->GetNumLogons() << wwiv::endl;
  bout << "|#9On today       : |#2" << a()->user()->GetTimesOnToday() << wwiv::endl;
  bout << "|#9Messages posted: |#2" << a()->user()->GetNumMessagesPosted() << wwiv::endl;
  auto total_mail_sent = a()->user()->GetNumEmailSent() + a()->user()->GetNumFeedbackSent() +
                         a()->user()->GetNumNetEmailSent();
  bout << "|#9E-mail sent    : |#2" << total_mail_sent << wwiv::endl;
  auto seconds_used = static_cast<int>(a()->user()->GetTimeOn());
  auto minutes_used = seconds_used / SECONDS_PER_MINUTE;
  minutes_used +=
      std::chrono::duration_cast<std::chrono::minutes>(a()->duration_used_this_session()).count();
  bout << "|#9Time spent on  : |#2" << minutes_used << " |#9Minutes" << wwiv::endl;

  // Transfer Area Statistics
  bout << "|#9Uploads        : |#2" << a()->user()->GetUploadK() << "|#9k in|#2 "
       << a()->user()->GetFilesUploaded() << " |#9files" << wwiv::endl;
  bout << "|#9Downloads      : |#2" << a()->user()->GetDownloadK() << "|#9k in|#2 "
       << a()->user()->GetFilesDownloaded() << " |#9files" << wwiv::endl;
  bout << "|#9Transfer Ratio : |#2" << ratio() << wwiv::endl;
  bout.nl();
  pausescr();
}

/**
 * Gets the maximum number of lines allowed for a post by the current user.
 * @return The maximum message length in lines
 */
int GetMaxMessageLinesAllowed() {
  if (so()) {
    return 120;
  }
  return (cs()) ? 100 : 80;
}

/**
 * Allows user to upload a post.
 */
void upload_post() {
  File file(FilePath(a()->temp_directory(), INPUT_MSG));
  off_t lMaxBytes = 250 * static_cast<off_t>(GetMaxMessageLinesAllowed());

  bout << "\r\nYou may now upload a message, max bytes: " << lMaxBytes << wwiv::endl << wwiv::endl;
  int i = 0;
  receive_file(file.full_pathname(), &i, INPUT_MSG, -1);
  if (file.Open(File::modeReadOnly | File::modeBinary)) {
    auto file_size = file.length();
    if (file_size > lMaxBytes) {
      bout << "\r\n|#6Sorry, your message is too long.  Not saved.\r\n\n";
      file.Close();
      file.Delete();
    } else {
      file.Close();
      use_workspace = true;
      bout << "\r\n|#7* |#1Message uploaded.  The next post or email will contain that text.\r\n\n";
    }
  } else {
    bout << "\r\n|#3Nothing saved.\r\n\n";
  }
}

/**
 * High-level function for sending email.
 */
void send_email() {
  write_inst(INST_LOC_EMAIL, 0, INST_FLAGS_NONE);
  bout << "\r\n\n|#9Enter user name or number:\r\n:";
  auto username = input_text(75);
  a()->context().clear_irt();
  auto atpos = username.find_first_of("@");
  if (atpos != string::npos && atpos != username.length() && isalpha(username[atpos + 1])) {
    if (username.find(INTERNET_EMAIL_FAKE_OUTBOUND_ADDRESS) == string::npos) {
      StringLowerCase(&username);
      username += StrCat(" ", INTERNET_EMAIL_FAKE_OUTBOUND_ADDRESS);
    }
  } else if (username.find('(') != std::string::npos && username.find(')') != std::string::npos) {
    // This is where we'd check for (NNNN) and add in the @NNN for the FTN networks.
    auto first = username.find_last_of('(');
    auto last = username.find_last_of(')');
    if (last > first) {
      auto inner = username.substr(first + 1, last - first - 1);
      if (inner.find('/') != std::string::npos) {
        // At least need a FTN address.
        username += StrCat(" ", FTN_FAKE_OUTBOUND_ADDRESS);
        bout << "\r\n|#9Sending to FTN Address: |#2" << inner << wwiv::endl;
      }
    }
  }

  uint16_t system_number, user_number;
  parse_email_info(username, &user_number, &system_number);
  clear_quotes();
  if (user_number || system_number) {
    email("", user_number, system_number, false, 0);
  }
}

/**
 * High-level function for selecting conference type to edit.
 */
void edit_confs() {
  if (!ValidateSysopPassword()) {
    return;
  }

  while (!a()->hangup_) {
    bout << "\r\n\n|#5Edit Which Conferences:\r\n\n";
    bout << "|#21|#9)|#1 Subs\r\n";
    bout << "|#22|#9)|#1 Dirs\r\n";
    bout << "\r\n|#9Select [|#21|#9,|#22|#9,|#2Q|#9]: ";
    char ch = onek("Q12", true);
    switch (ch) {
    case '1':
      conf_edit(ConferenceType::CONF_SUBS);
      break;
    case '2':
      conf_edit(ConferenceType::CONF_DIRS);
      break;
    case 'Q':
      return;
    }
    CheckForHangup();
  }
}

/**
 * Sends Feedback to the SysOp.  If  bNewUserFeedback is true then this is
 * newuser feedback, otherwise it is "normal" feedback.
 * The user can choose to email anyone listed.
 * Users with a()->usernum < 10 who have sysop privs will be listed, so
 * this user can select which sysop to leave feedback to.
 */
void feedback(bool bNewUserFeedback) {
  int i;
  char onek_str[20], ch;

  clear_quotes();

  if (bNewUserFeedback) {
    auto title =
        StringPrintf("|#1Validation Feedback (|#6%d|#2 slots left|#1)",
                     a()->config()->max_users() - a()->status_manager()->GetUserCount());
    // We disable the fsed here since it was hanging on some systems.  Not sure why
    // but it's better to be safe -- Rushfan 2003-12-04
    email(title, 1, 0, true, 0, false);
    return;
  }
  if (a()->context().guest_user()) {
    a()->status_manager()->RefreshStatusCache();
    email("Guest Account Feedback", 1, 0, true, 0, true);
    return;
  }
  int nNumUserRecords = a()->users()->num_user_records();
  int i1 = 0;

  for (i = 2; i < 10 && i < nNumUserRecords; i++) {
    User user;
    a()->users()->readuser(&user, i);
    if ((user.GetSl() == 255 || (a()->config()->sl(user.GetSl()).ability & ability_cosysop)) &&
        !user.IsUserDeleted()) {
      i1++;
    }
  }

  if (!i1) {
    i = 1;
  } else {
    onek_str[0] = '\0';
    i1 = 0;
    bout.nl();
    for (i = 1; (i < 10 && i < nNumUserRecords); i++) {
      User user;
      a()->users()->readuser(&user, i);
      if ((user.GetSl() == 255 || (a()->config()->sl(user.GetSl()).ability & ability_cosysop)) &&
          !user.IsUserDeleted()) {
        bout << "|#2" << i << "|#7)|#1 " << a()->names()->UserName(i) << wwiv::endl;
        onek_str[i1++] = static_cast<char>('0' + i);
      }
    }
    onek_str[i1++] = *str_quit;
    onek_str[i1] = '\0';
    bout.nl();
    bout << "|#1Feedback to (" << onek_str << "): ";
    ch = onek(onek_str, true);
    if (ch == *str_quit) {
      return;
    }
    bout.nl();
    i = ch - '0';
  }

  email("|#1Feedback", static_cast<uint16_t>(i), 0, false, 0, true);
}

/**
 * Allows editing of ASCII textfiles. Must have an external editor defined,
 * and toggled for use in defaults.
 */
void text_edit() {
  bout.nl();
  bout << "|#9Enter Filename: ";
  const auto filename = input_filename(12);
  if (filename.find(".log") != string::npos || !okfn(filename)) {
    return;
  }
  sysoplog() << "@ Edited: " << filename;
  if (okfsed()) {
    external_text_edit(filename, a()->config()->gfilesdir(), 500, MSGED_FLAG_NO_TAGLINE);
  }
}
