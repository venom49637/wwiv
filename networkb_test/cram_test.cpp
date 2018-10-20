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
#include "gtest/gtest.h"
#include "core/strings.h"
#include "core_test/file_helper.h"
#include "networkb/cram.h"

#include <cstdint>
#include <string>

using std::endl;
using std::string;
using std::unique_ptr;
using namespace wwiv::net;
using namespace wwiv::strings;


TEST(CramTest, Basic) {
  Cram c;
  string h = c.CreateHashedSecret("cafebabecafebabecafebabecafebabe", "WELCOME");
  // fidopoll from mystic returned bfd5323f395243161863e7a9cd1de854
  EXPECT_EQ("bfd5323f395243161863e7a9cd1de854", h);
}
