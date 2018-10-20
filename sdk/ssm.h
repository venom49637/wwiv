/**************************************************************************/
/*                                                                        */
/*                            WWIV Version 5                              */
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
#ifndef __INCLUDED_SDK_SSM_H__
#define __INCLUDED_SDK_SSM_H__

#include <string>
#include <vector>

#include "sdk/config.h"
#include "sdk/net.h"
#include "sdk/vardec.h"
#include "sdk/usermanager.h"

namespace wwiv {
namespace sdk {

class SSM {
public:
  SSM(const wwiv::sdk::Config& config, wwiv::sdk::UserManager& user_manager);
  virtual ~SSM();

  bool send_local(uint32_t user_number, const std::string& text);
  bool send_remote(const net_networks_rec& net, uint16_t system_number, uint32_t from_user_number, uint32_t user_number, const std::string& text);
  bool delete_local_to_user(uint32_t user_number);
private:
  const std::string data_directory_;
  wwiv::sdk::UserManager& user_manager_;
};


}
}

#endif  // __INCLUDED_SDK_SSM_H__
