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
#include "bbs/execexternal.h"

#include "bbs/bbs.h"
#include "bbs/com.h"
#include "bbs/dropfile.h"
#include "bbs/instmsg.h"
#include "bbs/wqscn.h"
#include "bbs/exec.h"
#include "core/log.h"

using namespace wwiv::core;

int ExecuteExternalProgram(const std::string& commandLine, int nFlags) {
  LOG(INFO) << "ExecuteExternalProgram: errno: " << errno;
  // forget it if the user has hung up
  if (!(nFlags & EFLAG_NOHUP)) {
    if (CheckForHangup()) {
      return -1;
    }
  }
  create_chain_file();

  // get ready to run it
  if (a()->IsUserOnline()) {
    a()->WriteCurrentUser();
    write_qscn(a()->usernum, a()->context().qsc, false);
  }

  // extra processing for net programs
  if (nFlags & EFLAG_NETPROG) {
    write_inst(INST_LOC_NET, a()->net_num() + 1, INST_FLAGS_NONE);
  }

  // Make sure our working dir is back to the BBS dir.
  a()->CdHome();
  
  if (nFlags & EFLAG_TEMP_DIR) {
    // If EFLAG_TEMP_DIR is specified, we should set the working directory
    // to the TEMP directory for the instance instead of the BBS directory.
    if (!File::set_current_directory(a()->temp_directory())) {
      LOG(ERROR) << "Unable to set working directory to: " << a()->temp_directory();
    }
  }

  // Some LocalIO implementations (Curses) needs to disable itself before
  // we fork some other process.
  a()->localIO()->DisableLocalIO();
  auto return_code = exec_cmdline(commandLine, nFlags);

  // Re-engage the local IO engine if needed.
  a()->localIO()->ReenableLocalIO();
  a()->CdHome();

  // Reread the user record.
  if (a()->IsUserOnline()) {
    a()->ReadCurrentUser();
    read_qscn(a()->usernum, a()->context().qsc, false, true);
    a()->UpdateTopScreen();
  }

  // return to caller
  return return_code;
}

