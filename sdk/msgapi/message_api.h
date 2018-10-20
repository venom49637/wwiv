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
#ifndef __INCLUDED_SDK_MSGAPI_MESSAGE_API_H__
#define __INCLUDED_SDK_MSGAPI_MESSAGE_API_H__

#include <memory>
#include <string>
#include <vector>

#include "sdk/net.h"
#include "sdk/subxtr.h"
#include "sdk/msgapi/message_area.h"

namespace wwiv {
namespace sdk {
namespace msgapi {

class MessageArea;

enum class OverflowStrategy {
  delete_one, delete_all, delete_none
};

struct MessageApiOptions {
  wwiv::sdk::msgapi::OverflowStrategy overflow_strategy = wwiv::sdk::msgapi::OverflowStrategy::delete_one;
};

class bad_message_area : public ::std::runtime_error {
public:
  bad_message_area(const ::std::string& sub_filename) : ::std::runtime_error(sub_filename) {}
};

class MessageApi {
public:
  MessageApi(
    const MessageApiOptions& options,
    const std::string& root_directory,
    const std::string& subs_directory,
    const std::string& messages_directory,
    const std::vector<net_networks_rec>& net_networks);

  /** Checks to see if the files for a subboard exist. */
  virtual bool Exist(const wwiv::sdk::subboard_t& sub) const = 0;
  /** Opens a message area, throwing bad_message_area if it does not exist. */
  virtual bool Create(const wwiv::sdk::subboard_t& sub, int subnum) = 0;
  /** Opens a message area, throwing bad_message_area if it does not exist. */
  virtual MessageArea* Open(const wwiv::sdk::subboard_t& sub, int subnum) = 0;
  /** Creates or Opens a message area. throwing bad_message_area if it exists but can not be opened.*/
  virtual MessageArea* CreateOrOpen(const wwiv::sdk::subboard_t& sub, int subnum);
  /** Deletes the message area identified by filename: name */
  virtual bool Remove(const std::string& name) = 0;

  const std::vector<net_networks_rec>& network() const { return net_networks_; }
  const std::string root_directory() const { return root_directory_; }
  const MessageApiOptions options() const { return options_; }

  // TODO(rushfan): Here's where we add hooks to the lastread system
  // so that messageapi's created inside the bbs will share *qsc with
  // the legacy code until it's all removed.
protected:
  const MessageApiOptions options_;

  std::string root_directory_;
  std::string subs_directory_;
  std::string messages_directory_;
  std::vector<net_networks_rec> net_networks_;
};

}  // namespace msgapi
}  // namespace sdk
}  // namespace wwiv

#endif  // __INCLUDED_SDK_MSGAPI_MESSAGE_API_H__
