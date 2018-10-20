/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*           Copyright (C)2014-2017, WWIV Software Services               */
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

#include "core/file.h"
#include "core/strings.h"
#include "core/version.h"
#include "core_test/file_helper.h"
#include "sdk/config.h"
#include "sdk/networks.h"
#include "sdk_test/sdk_helper.h"

using namespace std;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

class ConfigTest : public testing::Test {
public:
  SdkHelper helper;
};

TEST_F(ConfigTest, Helper_CreatedBBSRoot) {
  ASSERT_TRUE(ends_with(helper.root(), "bbs")) << helper.root();
}

TEST_F(ConfigTest, Config_CurrentDirectory) {
  ASSERT_EQ(0, chdir(helper.root().c_str()));

  Config config(File::current_directory());
  ASSERT_TRUE(config.IsInitialized());
  EXPECT_EQ(helper.data_, config.datadir());
}

TEST_F(ConfigTest, Config_DifferentDirectory) {
  Config config(helper.root());
  ASSERT_TRUE(config.IsInitialized());
  EXPECT_EQ(helper.data_, config.datadir());
}

TEST_F(ConfigTest, SetConfig_Stack) {
  Config config(helper.root());
  ASSERT_TRUE(config.IsInitialized());

  configrec c{};
  strcpy(c.systemname, "mysys");
  config.set_config(&c, true);
  ASSERT_EQ(c.systemname, config.system_name());
}

TEST_F(ConfigTest, SetConfig_Heap) {
  Config config(helper.root());
  ASSERT_TRUE(config.IsInitialized());

  configrec* c = new configrec();
  strcpy(c->systemname, "mysys");
  config.set_config(c, true);
  ASSERT_EQ(c->systemname, config.system_name());
  EXPECT_NE(nullptr, c);
  delete c;
}

TEST_F(ConfigTest, WrittenByNumVersion) {
  Config config(helper.root());

  ASSERT_EQ(wwiv_num_version, config.written_by_wwiv_num_version());
}

TEST_F(ConfigTest, Is5XXOrLater) {
  Config config(helper.root());

  ASSERT_TRUE(config.is_5xx_or_later());
}
