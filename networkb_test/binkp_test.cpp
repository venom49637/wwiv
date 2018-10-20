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
#include "networkb/binkp.h"
#include "networkb/binkp_commands.h"
#include "networkb/binkp_config.h"
#include "sdk/callout.h"
#include "networkb/transfer_file.h"
#include "networkb_test/fake_connection.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <thread>

using std::clog;
using std::endl;
using std::string;
using std::thread;
using std::unique_ptr;
using wwiv::sdk::Callout;
using namespace wwiv::core;
using namespace wwiv::net;
using namespace wwiv::strings;

static const string ANSWERING_ADDRESS = "20000/20000:1";
static const int ORIGINATING_ADDRESS = 2;

class BinkTest : public testing::Test {
protected:
  void StartBinkpReceiver() {
    files_.Mkdir("network");
    files_.Mkdir("gfiles");
    const string line("@1 example.com");
    files_.CreateTempFile("binkp.net", line);
    const string network_dir = files_.DirName("network");
    const string gfiles_dir = files_.DirName("gfiles");
    memset(&wwiv_config_, 0, sizeof(configrec));
    strcpy(wwiv_config_.systemname, "Test System");
    strcpy(wwiv_config_.sysopname, "Test Sysop");
    strcpy(wwiv_config_.gfilesdir, gfiles_dir.c_str());
    wwiv::sdk::Config config(File::current_directory());
    config.set_config(&wwiv_config_, true);
    config.set_initialized_for_test(true);
    net_networks_rec net{};
    net.dir = network_dir;
    to_char_array(net.name, "Dummy Network");
    net.type = network_type_t::wwivnet;
    net.sysnum = 0;
    BinkConfig* dummy_config = new BinkConfig(ORIGINATING_ADDRESS, config, network_dir);
    std::unique_ptr<Callout> dummy_callout = std::make_unique<Callout>(net);
    BinkP::received_transfer_file_factory_t null_factory = [](const string&, const string& filename) { 
      return new InMemoryTransferFile(filename, "");
    };
    dummy_config->callouts()["wwivnet"] = std::move(dummy_callout);
    binkp_.reset(new BinkP(&conn_, dummy_config, BinkSide::ANSWERING, ANSWERING_ADDRESS, null_factory));
    CommandLine cmdline({ "networkb_tests.exe" }, "");
    thread_ = thread([&]() { binkp_->Run(cmdline); });
  } 

  void Stop() {
    thread_.join();
  }

  unique_ptr<BinkP> binkp_;
  FakeConnection conn_;
  std::thread thread_;
  FileHelper files_;
  configrec wwiv_config_;
};

TEST_F(BinkTest, ErrorAbortsSession) {
  StartBinkpReceiver();
  conn_.ReplyCommand(BinkpCommands::M_ERR, "Doh!");
  Stop();
  
  while (conn_.has_sent_packets()) {
    clog << conn_.GetNextPacket().debug_string() << endl;
  }
}

static int node_number_from_address_list(const std::string& addresses, const string& network_name) {
  auto a = ftn_address_from_address_list(addresses, network_name);
  return wwivnet_node_number_from_ftn_address(a);
}

TEST(NodeFromAddressTest, SingleAddress) {
  const string address = "20000:20000/1234@foonet";
  EXPECT_EQ(1234, node_number_from_address_list(address, "foonet"));
  EXPECT_EQ(WWIVNET_NO_NODE, node_number_from_address_list(address, "wwivnet"));
}

TEST(NodeFromAddressTest, MultipleAddresses) {
  const string address = "1:369/23@fidonet 20000:20000/1234@foonet 20000:369/24@dorknet";
  EXPECT_EQ("20000:20000/1234@foonet", ftn_address_from_address_list(address, "foonet"));
  EXPECT_EQ(1234, node_number_from_address_list(address, "foonet"));
  EXPECT_EQ(WWIVNET_NO_NODE, node_number_from_address_list(address, "wwivnet"));
  EXPECT_EQ(WWIVNET_NO_NODE, node_number_from_address_list(address, "fidonet"));
  EXPECT_EQ(WWIVNET_NO_NODE, node_number_from_address_list(address, "dorknet"));
}

TEST(NodeFromAddressTest, MultipleAddresses_SameNetwork) {
  const string address = "1:369/-1@coolnet 1:369/23@coolnet";
  EXPECT_EQ("1:369/23@coolnet", ftn_address_from_address_list(address, "coolnet"));
}

TEST(NetworkNameFromAddressTest, SingleAddress) {
  const string address = "1:369/23@fidonet";
  EXPECT_EQ("fidonet", network_name_from_single_address(address));
}

// string expected_password_for(Callout* callout, int node)
TEST(ExpectedPasswordTest, Basic) {
  net_call_out_rec n{ "20000:20000/1234", 1234, 1, unused_options_sendback, 2, 3, 4, "pass", 5, 6 };
  Callout callout({ n });
  string actual = expected_password_for(&callout, 1234);
  EXPECT_EQ("pass", actual);
}

TEST(ExpectedPasswordTest, WrongNode) {
  net_call_out_rec n{"20000:20000/1234", 1234, 1, unused_options_sendback, 2, 3, 4, "pass", 5, 6 };
  Callout callout({ n });
  string actual = expected_password_for(&callout, 12345);
  EXPECT_EQ("-", actual);
}
