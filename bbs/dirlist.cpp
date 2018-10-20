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

#include "bbs/bbsovl1.h"
#include "bbs/bbsutl2.h"
#include "bbs/conf.h"
#include "bbs/confutil.h"
#include "bbs/bbs.h"
#include "bbs/bbsutl.h"
#include "bbs/utility.h"
#include "bbs/xfer.h"
#include "bbs/mmkey.h"
#include "core/strings.h"


//////////////////////////////////////////////////////////////////////////////
//
// Implementation
//
//
//


/** Displays the available file areas for the current user. */
void dirlist(int mode) {
  bool next   = false;
  int oc      = a()->GetCurrentConferenceFileArea();
  int os      = a()->current_user_dir().subnum;
  int tally   = 0;
  int nd      = 0;
  int sn      = a()->GetCurrentConferenceFileArea();
  int en      = a()->GetCurrentConferenceFileArea();
  bool done   = false;

  do {
    bool is = false;
    bool abort = false;
    int p = 1;
    int i = sn;

    while (i <= en && a()->uconfdir[i].confnum != -1 && !abort) {
      size_t i1 = 0;
      while (i1 < a()->directories.size() && a()->udir[i1].subnum != -1 && !abort) {
        char s[255];
        size_t firstp = 0;
        if (p && mode == 0) {
          p = 0;
          firstp = i1;
          bout.cls();
          if (a()->uconfdir[1].confnum != -1 && okconf(a()->user())) {
            auto conf_name = stripcolors(a()->dirconfs[a()->uconfdir[i].confnum].conf_name);
            sprintf(s, " [ %s %c ] [ %s ] ", "Conference",
              a()->dirconfs[a()->uconfdir[i].confnum].designator,
              conf_name.c_str());
          } else {
            sprintf(s, " [ %s File Areas ] ", a()->config()->system_name().c_str());
          }
          bout.litebar(s);
          DisplayHorizontalBar(78, 7);
          bout << "|#2 Dir Qscan?     Directory Name                          Total Files\r\n";
          DisplayHorizontalBar(78, 7);
        }
        ++nd;
        int directory_number = a()->udir[i1].subnum;
        if (directory_number == 0) {
          is = true;
        }
        std::string scanme = "|#6No ";
        if (a()->context().qsc_n[directory_number / 32] & (1L << (directory_number % 32))) {
          scanme = "|#5Yes";
        }
        dliscan1(directory_number);
        if (a()->current_user_dir().subnum == a()->udir[i1].subnum) {
          sprintf(s, " |#9%3s |#9\xB3 |#6%3s |#9\xB3|17|15 %-40.40s |#9\xB3 |#9%4d|16",
                  a()->udir[i1].keys, scanme.c_str(), a()->directories[directory_number].name,
                  a()->numf);
        } else {
          sprintf(s, " |#9%3s |#9\xB3 |#6%3s |#9\xB3 %s%-40.40s |#9\xB3 |#9%4d",
                  a()->udir[i1].keys, scanme.c_str(),
                  (((mode == 1) && (a()->directories[a()->udir[i1].subnum].mask & mask_cdrom)) ? "|#9" : "|#1"),
                  a()->directories[ directory_number ].name, a()->numf);
        }
        if (okansi()) {
          bout.bputs(s, &abort, &next);
        } else {
          bout.bputs(stripcolors(s), &abort, &next);
        }
        tally += a()->numf;
        int lastp = i1++;
        bout.nl();
        if (bout.lines_listed() >= a()->screenlinest - 2 && mode == 0) {
          p = 1;
          bout.clear_lines_listed();
          DisplayHorizontalBar(78, 7);
          bout.bprintf("|#1Select |#9[|#2%d-%d, [N]ext Page, [Q]uit|#9]|#0 : ",
                                            is ? firstp : firstp + 1, lastp);
          std::string ss = mmkey(MMKeyAreaType::dirs, true);
          if (isdigit(ss[0])) {
            for (uint16_t i3 = 0; i3 < static_cast<uint16_t>(a()->directories.size()); i3++) {
              if (ss == a()->udir[i3].keys) {
                a()->set_current_user_dir_num(i3);
                os = a()->current_user_dir().subnum;
                done = true;
                abort = true;
              }
            }
          } else {
            switch (ss.front()) {
            case 'Q':
              if (okconf(a()->user())) {
                setuconf(ConferenceType::CONF_DIRS, oc, os);
              }
              done    = true;
              abort   = true;
              break;
            default:
              bout.backline();
              break;
            }
          }
        }
      }
      if (nd) {
        i++;
      }
      if (!okconf(a()->user())) {
        break;
      }
    }
    if (i == 0) {
      bout.bpla("None.", &abort);
      bout.nl();
    }
    if (!abort && mode == 0) {
      p = 1;
      DisplayHorizontalBar(78, 7);
      if (okconf(a()->user())) {
        if (a()->uconfdir[1].confnum != -1) {
          bout.bprintf("|#1Select |#9[|#2%d-%d, J=Join Conference, ?=List Again, Q=Quit|#9]|#0 : ",
                                            is ? 0 : 1, is ? nd - 1 : nd);
        } else {
          bout.bprintf("|#1Select |#9[|#2%d-%d, ?=List Again, Q=Quit|#9]|#0 : ", is ? 0 : 1,
                                            is ? nd - 1 : nd);
        }
      } else {
        bout.bprintf("|#1Select |#9[|#2%d-%d, ?=List Again, Q=Quit|#9]|#0 : ", is ? 0 : 1,
                                          is ? nd - 1 : nd);
      }
      std::string ss = mmkey(MMKeyAreaType::subs, true);
      if (ss.empty() || ss == "Q" || ss == "\r") {
        if (okconf(a()->user())) {
          setuconf(ConferenceType::CONF_DIRS, oc, os);
        }
        done = true;
      }
      if (ss == "J") {
        if (okconf(a()->user())) {
          jump_conf(ConferenceType::CONF_DIRS);
        }
        sn = en = oc = a()->GetCurrentConferenceFileArea();
        nd = i = 0;
        is = false;
      }
      if (isdigit(ss.front())) {
        for (uint16_t i3 = 0; i3 < a()->directories.size(); i3++) {
          if (ss == a()->udir[i3].keys) {
            a()->set_current_user_dir_num(i3);
            os = a()->current_user_dir().subnum;
            done = true;
          }
        }
      }
      nd = 0;
    } else {
      if (okconf(a()->user())) {
        setuconf(ConferenceType::CONF_DIRS, oc, os);
      }
      done = true;
    }
  } while (!a()->hangup_ && !done);
}


