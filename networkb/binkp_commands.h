/**************************************************************************/
/*                                                                        */
/*                          WWIV Version 5.x                              */
/*             Copyright (C)2015-2017, WWIV Software Services             */
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
#ifndef __INCLUDED_NETWORKB_BINKP_COMMANDS_H__
#define __INCLUDED_NETWORKB_BINKP_COMMANDS_H__
#pragma once

#include <string>

namespace wwiv {
namespace net {

class BinkpCommands {
public:
static const int M_NUL  = 0;
static const int M_ADR  = 1;
static const int M_PWD  = 2;
static const int M_FILE = 3;
static const int M_OK   = 4;
static const int M_EOB  = 5;
static const int M_GOT  = 6;
static const int M_ERR  = 7;
static const int M_BSY  = 8;
static const int M_GET  = 9;
static const int M_SKIP = 10;

static std::string command_id_to_name(int command_id);

private:
BinkpCommands() {}
~BinkpCommands() {}
};

}  // namespace net
} // namespace wwiv

#endif  // __INCLUDED_NETWORKB_BINKP_COMMANDS_H__