/**************************************************************************/
/*                                                                        */
/*                  WWIV Initialization Utility Version 5                 */
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

#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include "core/strings.h"
#include "localui/stdio_win.h"
#include "localui/wwiv_curses.h"

using std::string;
using wwiv::strings::StringPrintf;

int StdioWindow::GetChar() const { return std::cin.get(); }

void StdioWindow::Putch(uint32_t ch) { std::cout << ch; }

void StdioWindow::Puts(const std::string& text) { std::cout << text; }

void StdioWindow::PutsXY(int, int, const std::string& text) { Puts(text); }
