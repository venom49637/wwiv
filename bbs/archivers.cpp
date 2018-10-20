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
#include "bbs/archivers.h"

#include <cstring>
#include <string>

#include "bbs/bbs.h"
#include "local_io/wconstants.h"
#include "bbs/application.h"
#include "core/strings.h"
#include "core/file.h"

using std::string;
using namespace wwiv::core;
using namespace wwiv::strings;

static int check_arc(const char* filename) {
  File f(filename);
  if (!f.Open(File::modeReadOnly)) {
    return COMPRESSION_UNKNOWN;
  }

  char header[10];
  auto num_read = f.Read(&header, 10);
  if (num_read < 10) {
    return COMPRESSION_UNKNOWN;
  }

  switch (header[0]) {
    case 0x60:
      if ((unsigned char) header[1] == (unsigned char) 0xEA)
        return COMPRESSION_ARJ;
      break;
    case 0x1a:
      return COMPRESSION_PAK;
    case 'P':
      if (header[1] == 'K')
        return COMPRESSION_ZIP;
      break;
    case 'R':
      if (header[1] == 'a')
        return COMPRESSION_RAR;
      break;
    case 'Z':
      if (header[1] == 'O' && header[2] == 'O')
        return COMPRESSION_ZOO;
      break;
  }
  if (header[0] == 'P') {
    return COMPRESSION_UNKNOWN;
  }
  header[9] = 0;
  if (strstr(header, "-lh")) {
    return COMPRESSION_LHA;
  }

  // Guess on type, using extension to guess
  const char* ext = strstr(filename, ".");
  if (!ext) {
    return COMPRESSION_UNKNOWN;
  }
  ++ext;
  if (iequals(ext, "ZIP")) {
    return COMPRESSION_ZIP;
  } else if (iequals(ext, "LHA")) {
    return COMPRESSION_LHA;
  } else if (iequals(ext, "LZH")) {
    return COMPRESSION_LHA;
  } else if (iequals(ext, "ZOO")) {
    return COMPRESSION_ZOO;
  } else if (iequals(ext, "ARC")) {
    return COMPRESSION_PAK;
  } else if (iequals(ext, "PAK")) {
    return COMPRESSION_PAK;
  } else if (iequals(ext, "ARJ")) {
    return COMPRESSION_ARJ;
  } else if (iequals(ext, "RAR")) {
    return COMPRESSION_RAR;
  }
  return COMPRESSION_UNKNOWN;
}

// Returns which archive as a number in your archive config
// Returns the first archiver you have listed if unknown
// One thing to note, if an 'arc' is found, it uses pak, and returns that
// The reason being, PAK does all ARC does, plus a little more, I belive
// PAK has its own special modes, but still look like an ARC, thus, an ARC
// Will puke if it sees this
int match_archiver(const char *filename) {
  char type[4];
  int x = check_arc(filename);
  if (x == COMPRESSION_UNKNOWN) {
    return 0;
  }
  switch (x) {
    case COMPRESSION_ZIP:
      to_char_array(type, "ZIP");
      break;
    case COMPRESSION_LHA:
      to_char_array(type, "LHA");
      break;
    case COMPRESSION_ARJ:
      to_char_array(type, "ARJ");
      break;
    case COMPRESSION_PAK:
      to_char_array(type, "PAK");
      break;
    case COMPRESSION_ZOO:
      to_char_array(type, "ZOO");
      break;
    case COMPRESSION_RAR:
      to_char_array(type, "RAR");
      break;
    default:
      to_char_array(type, "ZIP");
      break;
  }

  x = 0;
  while (x < 4) {
    if (iequals(type, a()->arcs[x].extension)) {
      return x;
    }
    ++x;
  }
  return 0;
}
