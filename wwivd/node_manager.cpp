/**************************************************************************/
/*                                                                        */
/*                          WWIV BBS Software                             */
/*               Copyright (C)2017, WWIV Software Services                */
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
#include "wwivd/node_manager.h"

#include "core/log.h"
#include "core/stl.h"
#include <memory>
#include <mutex>
#include <unordered_map>

using wwiv::stl::contains;

namespace wwiv {
namespace wwivd {

std::string to_string(ConnectionType t) {
  switch (t) {
  case ConnectionType::BINKP:
    return "BinkP";
  case ConnectionType::HTTP:
    return "HTTP";
  case ConnectionType::SSH:
    return "SSH";
  case ConnectionType::TELNET:
    return "Telnet";
  case ConnectionType::UNKNOWN:
    return "*UNKNOWN*";
  }
  return "*UNKNOWN*";
}

NodeManager::NodeManager(const std::string& name, ConnectionType type, int start, int end)
    : name_(name), type_(type), start_(start), end_(end) {
  for (auto i = start; i <= end; i++) {
    clear_node(i);
  }
}

NodeManager::~NodeManager() {}

std::string NodeManager::status_string(const NodeStatus& n) const {
  auto s = n.description;
  if (n.connected) {
    s += " [";
    s += to_string(n.type);
    s += "]";
  }
  return s;
}

std::vector<std::string> NodeManager::status_lines() const {
  std::lock_guard<std::mutex> lock(mu_);
  std::vector<std::string> v;
  for (const auto& n : nodes_) {
    std::ostringstream ss;
    ss << this->name_ << " ";
    if (n.first > 0) {
      ss << "Node #" << n.first << " ";
    }
    ss << status_string(n.second);
    v.push_back(ss.str());
  }

  return v;
}

NodeStatus& NodeManager::status_for_unlocked(int node) { return nodes_[node]; }

NodeStatus NodeManager::status_for_copy(int node) {
  std::lock_guard<std::mutex> lock(mu_);
  auto n = status_for_unlocked(node);
  return n;
}

void NodeManager::set_node(int node, ConnectionType type, const std::string& description) {
  std::lock_guard<std::mutex> lock(mu_);
  auto& n = status_for_unlocked(node);
  n.node = node;
  n.type = type;
  n.description = description;
  n.connected = true;
}

void NodeManager::clear_node(int node) {
  std::lock_guard<std::mutex> lock(mu_);
  auto& n = status_for_unlocked(node);
  n.node = node;
  n.type = type_;
  n.connected = false;
  n.description = "Waiting for Call";
}

int NodeManager::nodes_used() const {
  std::lock_guard<std::mutex> lock(mu_);
  auto count = 0;
  for (const auto& e : nodes_) {
    if (e.second.connected) {
      count++;
    }
  }
  return count;
}

bool NodeManager::AcquireNode(int& node) {
  std::lock_guard<std::mutex> lock(mu_);
  for (auto& e : nodes_) {
    if (!e.second.connected) {
      e.second.connected = true;
      e.second.type = type_;
      e.second.description = "Connecting...";
      node = e.second.node;
      return true;
    }
  }
  // None
  node = -1;
  return false;
}

bool NodeManager::ReleaseNode(int node) {
  std::lock_guard<std::mutex> lock(mu_);
  if (!contains(nodes_, node)) {
    return false;
  }
  auto& n = nodes_.at(node);
  if (!n.connected) {
    return false;
  }
  n.connected = false;
  n.type = type_;
  n.description = "Waiting For Call";
  return true;
}

ConcurrentConnections::ConcurrentConnections(int max_num) : max_num_(max_num) {}
ConcurrentConnections::~ConcurrentConnections() {}

bool ConcurrentConnections::aquire(const std::string& peer) {
  VLOG(1) << "ConcurrentConnections::aquire: " << peer;
  std::lock_guard<std::mutex> lock(connection_mu_);
  auto cur = map_[peer];
  VLOG(2) << "ConcurrentConnections: cur: " << cur << "; max_num_: " << max_num_;
  if (cur < max_num_) {
    map_[peer] = cur + 1;
    VLOG(2) << "ConcurrentConnections: (post increment) cur: " << cur << "; max_num_: " << max_num_;
    return true;
  }
  return false;
}

bool ConcurrentConnections::release(const std::string& peer) {
  std::lock_guard<std::mutex> lock(connection_mu_);
  auto cur = map_[peer] - 1;
  if (cur > 0) {
    map_[peer] = cur;
  } else {
    map_.erase(peer);
  }
  return true;
}


} // namespace wwivd
} // namespace wwiv
