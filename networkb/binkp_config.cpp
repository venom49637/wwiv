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
#include "networkb/binkp_config.h"

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#include "core/file.h"
#include "core/inifile.h"
#include "core/strings.h"
#include "sdk/fido/fido_address.h"
#include "sdk/fido/fido_callout.h"
#include "sdk/networks.h"

using std::endl;
using std::map;
using std::string;
using std::stringstream;
using std::unique_ptr;
using std::vector;
using wwiv::core::IniFile;
using namespace wwiv::strings;
using namespace wwiv::sdk;
using namespace wwiv::sdk::fido;

namespace wwiv {
namespace net {

BinkConfig::BinkConfig(const std::string& callout_network_name, const Config& config,
                       const Networks& networks)
    : config_(config), callout_network_name_(callout_network_name), networks_(networks) {
  // network names will alwyas be compared lower case.
  StringLowerCase(&callout_network_name_);
  system_name_ = config.system_name();
  if (system_name_.empty()) {
    system_name_ = "Unnamed WWIV BBS";
  }
  sysop_name_ = config.sysop_name();
  if (sysop_name_.empty()) {
    sysop_name_ = "Unknown WWIV SysOp";
  }
  gfiles_directory_ = config.gfilesdir();

  if (networks.contains(callout_network_name)) {
    const net_networks_rec& net = networks[callout_network_name];
    if (net.type == network_type_t::wwivnet) {
      callout_wwivnet_node_ = net.sysnum;
      if (callout_wwivnet_node_ == 0) {
        throw config_error(
            StringPrintf("NODE not specified for network: '%s'", callout_network_name.c_str()));
      }
      binkp_.reset(new Binkp(net.dir));
    } else if (net.type == network_type_t::ftn) {
      callout_fido_node_ = net.fido.fido_address;
      if (callout_fido_node_.empty()) {
        throw config_error(
            StringPrintf("NODE not specified for network: '%s'", callout_network_name.c_str()));
      }
    } else {
      throw config_error("BinkP is not supported for this network type.");
    }
    // TODO(rushfan): This needs to be a shim binkp that reads from the nodelist
    // or overrides.
    binkp_.reset(new Binkp(net.dir));
  }
}

const net_networks_rec& BinkConfig::network(const std::string& network_name) const {
  return networks_[network_name];
}

const net_networks_rec& BinkConfig::callout_network() const {
  return network(callout_network_name_);
}

const std::string BinkConfig::network_dir(const std::string& network_name) const {
  return network(network_name).dir;
}

static net_networks_rec test_net(const string& network_dir) {
  net_networks_rec net;
  net.sysnum = 1;
  strcpy(net.name, "wwivnet");
  net.type = network_type_t::wwivnet;
  net.dir = network_dir;
  return net;
}

// For testing
BinkConfig::BinkConfig(int callout_node_number, const wwiv::sdk::Config& config,
                       const string& network_dir)
    : config_(config), callout_network_name_("wwivnet"), callout_wwivnet_node_(callout_node_number),
      networks_({test_net(network_dir)}) {
  binkp_.reset(new Binkp(network_dir));
  system_name_ = config.system_name();
  sysop_name_ = config.sysop_name();
  gfiles_directory_ = config.gfilesdir();
}

BinkConfig::~BinkConfig() {}

const binkp_session_config_t* BinkConfig::binkp_session_config_for(const std::string& node) const {
  static binkp_session_config_t static_session{};

  if (callout_network().type == network_type_t::wwivnet) {
    if (!binkp_) {
      return nullptr;
    }
    return binkp_->binkp_session_config_for(node);
  } else if (callout_network().type == network_type_t::ftn) {
    try {
      FidoAddress address(node);
      FidoCallout fc(config_, callout_network());
      if (!fc.IsInitialized())
        return nullptr;
      auto fido_node = fc.fido_node_config_for(address);

      if (fido_node.binkp_config.host.empty()) {
        // We must have a host at least, otherwise we know this
        // is a completely empty record and we must return nullptr
        // since the node config is not found.
        return nullptr;
      }

      static_session = fido_node.binkp_config;
      if (static_session.port == 0) {
        // Set to default port.
        static_session.port = 24554;
      }

      if (static_session.port == 0 && static_session.host.empty()) {
        return nullptr;
      }

      return &static_session;
    } catch (const std::exception&) {
      return nullptr;
    }
  }
  return nullptr;
}

const binkp_session_config_t* BinkConfig::binkp_session_config_for(uint16_t node) const {
  return binkp_session_config_for(std::to_string(node));
}

} // namespace net
} // namespace wwiv
