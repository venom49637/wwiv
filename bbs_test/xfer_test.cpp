/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*           Copyright (C)2007-2017, WWIV Software Services               */
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
#include "gtest/gtest.h"

#include <string>

using std::cout;
using std::endl;
using std::string;

// TODO(rushfan): Make xfer.h
bool okfn(const string& fileName);

TEST(XferTest, Okfn) {
    EXPECT_TRUE( !okfn("") );
    EXPECT_TRUE( okfn("foo") );
    EXPECT_TRUE( okfn("foo.bar") );
    EXPECT_TRUE( !okfn("/foo") );
    EXPECT_TRUE( !okfn("<foo") );
    EXPECT_TRUE( !okfn(">foo") );
    EXPECT_TRUE( !okfn("`foo") );
    EXPECT_TRUE( !okfn("-foo") );
    EXPECT_TRUE( !okfn(" foo") );
    EXPECT_TRUE( !okfn("@foo") );
    EXPECT_TRUE( !okfn(".foo") );
    EXPECT_TRUE( !okfn("COM1") );
    EXPECT_TRUE( !okfn("PRN") );
    EXPECT_TRUE( !okfn("KBD$") );
    EXPECT_TRUE( okfn("COM1A") );
}
