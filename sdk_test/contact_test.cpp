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
#include "core/datetime.h"
#include "core/strings.h"
#include "core_test/file_helper.h"
#include "sdk/contact.h"
#include "gtest/gtest.h"

#include <cstdint>
#include <string>

using std::string;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::strings;

class ContactTest : public testing::Test {
protected:
  ContactTest() {
    now = time_t_now();
    then = now - 100;
  }
  NetworkContact c1{1};
  NetworkContact c2{2};
  time_t now;
  time_t then;
};

TEST_F(ContactTest, SimpleCase) {
  Contact c({}, {c1, c2});
  NetworkContact* ncr1 = c.contact_rec_for(1);

  c.add_connect(1, then, 100, 200);

  EXPECT_EQ(then, ncr1->lastcontact());
  EXPECT_EQ(then, ncr1->lastcontactsent());
  EXPECT_EQ(then, ncr1->lasttry());
  EXPECT_EQ(1, ncr1->numcontacts());
  EXPECT_EQ(0, ncr1->numfails());
  EXPECT_EQ(100, ncr1->bytes_sent());
  EXPECT_EQ(200, ncr1->bytes_received());
  EXPECT_EQ(0, ncr1->bytes_waiting());
}

TEST_F(ContactTest, MultipleConnects) {
  Contact c({}, {c1, c2});
  NetworkContact* ncr1 = c.contact_rec_for(1);

  c.add_connect(1, then, 100, 200);
  c.add_connect(1, now, 200, 300);

  EXPECT_EQ(now, ncr1->lastcontact());
  EXPECT_EQ(now, ncr1->lastcontactsent());
  EXPECT_EQ(now, ncr1->lasttry());
  EXPECT_EQ(then, ncr1->firstcontact());
  EXPECT_EQ(2, ncr1->numcontacts());
  EXPECT_EQ(0, ncr1->numfails());
  EXPECT_EQ(300, ncr1->bytes_sent());
  EXPECT_EQ(500, ncr1->bytes_received());
  EXPECT_EQ(0, ncr1->bytes_waiting());
}

TEST_F(ContactTest, WithFailure) {
  Contact c({}, {c1, c2});
  NetworkContact* ncr1 = c.contact_rec_for(1);

  c.add_connect(1, then, 100, 200);
  c.add_failure(1, now);

  EXPECT_EQ(now, ncr1->lastcontact());
  EXPECT_EQ(now, ncr1->lasttry());
  EXPECT_EQ(then, ncr1->lastcontactsent());
  EXPECT_EQ(then, ncr1->firstcontact());
  EXPECT_EQ(2, ncr1->numcontacts());
  EXPECT_EQ(1, ncr1->numfails());
  EXPECT_EQ(100, ncr1->bytes_sent());
  EXPECT_EQ(200, ncr1->bytes_received());
  EXPECT_EQ(0, ncr1->bytes_waiting());
}

TEST_F(ContactTest, EnsureBytesWaitingClears) {
  Contact c({}, {c1, c2});
  NetworkContact* ncr1 = c.contact_rec_for(1);

  ncr1->set_bytes_waiting(100);
  c.add_connect(1, then, 100, 200);
  EXPECT_EQ(0, ncr1->bytes_waiting());

  ncr1->set_bytes_waiting(100);
  c.add_failure(1, now);
  EXPECT_EQ(100, ncr1->bytes_waiting());

  c.add_connect(1, now + 1, 100, 200);
  EXPECT_EQ(0, ncr1->bytes_waiting());
}

