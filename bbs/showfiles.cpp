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

#include <cstring>
#include <string>

#include "bbs/bbs.h"
#include "bbs/application.h"
#include "local_io/local_io.h"
#include "bbs/utility.h"
#include "bbs/xfer.h"
#include "core/wwivport.h"
#include "core/findfiles.h"
#include "core/strings.h"

using namespace wwiv::core;
using namespace wwiv::strings;

// Displays list of files matching filespec file_name in directory pszDirectoryName.
void show_files(const char *file_name, const char *pszDirectoryName) {
  char drive[MAX_PATH], direc[MAX_PATH], file[MAX_PATH], ext[MAX_PATH];

  auto c = (okansi()) ? '\xCD' : '=';
  bout.nl();
#if defined (_WIN32)
  _splitpath(pszDirectoryName, drive, direc, file, ext);
#else
  strcpy(direc, pszDirectoryName);
  strcpy(drive, "");
  strcpy(file, file_name);
  strcpy(ext, "");
#endif
  auto stripped_fn = ToStringLowerCase(stripfn(file_name));
  auto s = StringPrintf("|#7[|17|15 FileSpec: %s    Dir: %s%s |16|#7]", stripped_fn.c_str(), drive,
                        direc);
  int i = (a()->user()->GetScreenChars() - 1) / 2 - size_without_colors(s) / 2;
  bout << "|#7" << std::string(i, c) << s;
  i = a()->user()->GetScreenChars() - 1 - i - size_without_colors(s);
  bout << "|#7" << std::string(i, c);

  auto full_pathname = FilePath(pszDirectoryName, strlwr(stripfn(file_name)));
  FindFiles ff(full_pathname, FindFilesType::files);
  for (const auto& f : ff) {
    s = f.name;
    align(&s);
    full_pathname = StrCat("|#7[|#2", s, "|#7]|#1 ");
    if (bout.wherex() > static_cast<int>(a()->user()->GetScreenChars() - 15)) {
      bout.nl();
    }
    bout << full_pathname;
  }

  bout.nl();
  bout.Color(7);
  bout << std::string(a()->user()->GetScreenChars() - 1, c);
  bout.nl(2);
}
