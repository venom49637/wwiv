/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*                 Copyright (C)2015-2017, WWIV Software Services         */
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
#include "core/os.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <sstream>

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX

#include <Windows.h>
#include <DbgHelp.h>

#endif  // _WIN32

#include <process.h>

#include "core/strings.h"
#include "core/file.h"

#pragma comment (lib, "DbgHelp.lib")

using std::numeric_limits;
using std::string;
using std::stringstream;
using namespace std::chrono;
using namespace wwiv::strings;

namespace wwiv {
namespace os {

void sleep_for(duration<double> d) {
  auto count = duration_cast<milliseconds>(d).count();
  if (count > numeric_limits<uint32_t>::max()) {
    count = numeric_limits<uint32_t>::max();
  }
  ::Sleep(static_cast<uint32_t>(count));
}

void sound(uint32_t frequency, duration<double> d) {
  auto count = duration_cast<milliseconds>(d).count();
  ::Beep(frequency, static_cast<uint32_t>(count));
}

std::string os_version_string() {
  return "Windows";
}

bool set_environment_variable(const std::string& variable_name, const std::string& value) {
  return ::SetEnvironmentVariable(variable_name.c_str(), value.c_str()) ? true : false;
}

std::string environment_variable(const std::string& variable_name) {
  // See http://techunravel.blogspot.com/2011/08/win32-env-variable-pitfall-of.html
  // Use Win32 functions to get since we do to set...
  char buffer[4096];
  DWORD size = GetEnvironmentVariable(variable_name.c_str(), buffer, 4096);
  if (size == 0) {
    // error or does not exits.
    return "";
  }
  return string(buffer);
}

string stacktrace() {
  HANDLE process = GetCurrentProcess();
  if (process == NULL) {
    return "";
  }
  SymInitialize(process, nullptr, TRUE);
  void* stack[100];
  uint16_t frames = CaptureStackBackTrace(0, 100, stack, nullptr);
  SYMBOL_INFO* symbol = (SYMBOL_INFO *) calloc(sizeof(SYMBOL_INFO) + 256 * sizeof(char), 1);
  if (symbol == NULL) {
    return "";
  }
  symbol->MaxNameLen = 255;
  symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

  stringstream out;
  // start at one to skip this current frame.
  for(decltype(frames) i = 1; i < frames; i++) {
    if (SymFromAddr(process, (DWORD64)(stack[i]), nullptr, symbol)) {
      out << frames - i - 1 << ": " << symbol->Name << " = " << std::hex << symbol->Address;
    }
    IMAGEHLP_LINE64 line;
    DWORD displacement;
    if (SymGetLineFromAddr64(process, (DWORD64)stack[i], &displacement, &line)) {
      out << " (" << line.FileName << ": " << std::dec << line.LineNumber << ") ";
    }
    out << std::endl;
  }
  free(symbol);
  return out.str();
}

pid_t get_pid() {
  return _getpid();
}


}  // namespace os
}  // namespace wwiv
