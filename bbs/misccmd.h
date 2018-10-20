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
/**************************************************************************/
#ifndef __INCLUDED_BBS_MISCCMD_H__
#define __INCLUDED_BBS_MISCCMD_H__

#include <string>
#include "sdk/net.h"

void kill_old_email();
void list_users(int mode);
void time_bank();
int getnetnum(const std::string& network_name);
int getnetnum_by_type(network_type_t type);
void uudecode(const char *input_filename, const char *output_filename);
void Packers();

#endif  // __INCLUDED_BBS_MISCCMD_H__