/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*                Copyright (C)2018, WWIV Software Services               */
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

#include <iostream>
#include <memory>
#include <string>

#include "bbs/bbs.h"
#include "bbs/input.h"
#include "bbs_test/bbs_helper.h"
#include "core/strings.h"
#include "core_test/file_helper.h"

using std::cout;
using std::endl;
using std::string;

using wwiv::sdk::User;

class InputTest : public ::testing::Test {
protected:
  void SetUp() override {}
};

// std::vector<std::set<char>> create_allowed_charmap(int64_t minv, int64_t maxv) 

TEST_F(InputTest, Smoke) { 
}
