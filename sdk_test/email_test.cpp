/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)2016-2017, WWIV Software Services             */
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

#include <ctime>
#include <iostream>
#include <memory>
#include <string>

#include "core/file.h"
#include "core/strings.h"
#include "core_test/file_helper.h"
#include "sdk/config.h"
#include "core/datetime.h"
#include "sdk/filenames.h"
#include "sdk/networks.h"
#include "sdk/msgapi/email_wwiv.h"
#include "sdk/msgapi/msgapi.h"
#include "sdk/msgapi/message_api_wwiv.h"
#include "sdk_test/sdk_helper.h"

using namespace std;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::sdk::msgapi;
using namespace wwiv::strings;

class EmailTest: public testing::Test {
public:
  void SetUp() override {
    config = make_unique<Config>(helper.root());
    wwiv::sdk::msgapi::MessageApiOptions options;
    options.overflow_strategy = wwiv::sdk::msgapi::OverflowStrategy::delete_none;
    api.reset(new WWIVMessageApi(options, *config, {}, new NullLastReadImpl()));
    email = std::unique_ptr<WWIVEmail>(((WWIVMessageApi*)api.get())->OpenEmail());
  }

  bool Add(int from, int to, const std::string& title, const std::string& text) {
    EmailData e{};
    e.title = title;
    e.text = text;
    e.daten = time_t_to_daten(time(nullptr));
    e.from_user = from;
    e.user_number = to;
    return email->AddMessage(e);
  }

  SdkHelper helper;
  unique_ptr<MessageApi> api;
  unique_ptr<Config> config;
  unique_ptr<WWIVEmail> email;
};

TEST_F(EmailTest, BasicCase) {
  // new area.
  EXPECT_EQ(0, email->number_of_messages());

  EXPECT_TRUE(File::Exists(wwiv::core::FilePath(helper.data(), EMAIL_DAT)));
  EXPECT_TRUE(File::Exists(wwiv::core::FilePath(helper.msgs(), EMAIL_DAT)));
}

TEST_F(EmailTest, Create) {
  // new area.
  EXPECT_EQ(0, email->number_of_messages());

  // Add a new message, expect it to be there.
  ASSERT_TRUE(Add(1, 2, "Title", "Text"));
  EXPECT_EQ(1, email->number_of_messages()) << wwiv::core::FilePath(helper.data(), EMAIL_DAT);

  // Read it back and make sure.
  mailrec nm{};
  ASSERT_TRUE(email->read_email_header(0, nm));
  EXPECT_STREQ("Title", nm.title);
}

TEST_F(EmailTest, Delete) {
  ASSERT_TRUE(Add(1, 2, "Title", "Text"));
  ASSERT_TRUE(Add(1, 2, "Title2", "Text2"));
  ASSERT_TRUE(Add(1, 3, "Title3", "Text3"));
  EXPECT_EQ(3, email->number_of_messages()) << wwiv::core::FilePath(helper.data(), EMAIL_DAT);

  email->DeleteAllMailToOrFrom(2);
  EXPECT_EQ(1, email->number_of_messages()) << wwiv::core::FilePath(helper.data(), EMAIL_DAT);
  mailrec nm{};
  EXPECT_FALSE(email->read_email_header(0, nm));
  EXPECT_FALSE(email->read_email_header(1, nm));
  EXPECT_TRUE(email->read_email_header(2, nm));
}
