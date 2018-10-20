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
#include "wwivutil/net/dump_callout.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include "core/log.h"
#include "core/strings.h"
#include "sdk/config.h"
#include "sdk/callout.h"
#include "sdk/config.h"
#include "sdk/networks.h"

using std::cout;
using std::endl;
using std::map;
using std::string;
using namespace wwiv::sdk;
using namespace wwiv::strings;

namespace wwiv {
namespace wwivutil {

std::string DumpCalloutCommand::GetUsage() const {
  std::ostringstream ss;
  ss << "Usage:   callout" << endl;
  ss << "Example: callout" << endl;
  return ss.str();
}

int DumpCalloutCommand::Execute() {
  Networks networks(*config()->config());
  if (!networks.IsInitialized()) {
    LOG(ERROR) << "Unable to load networks.";
    return 1;
  }

  map<const string, Callout> callouts;
  for (const auto net : networks.networks()) {
    string lower_case_network_name(net.name);
    StringLowerCase(&lower_case_network_name);
    callouts.emplace(lower_case_network_name, Callout(net));
  }

  for (const auto& c : callouts) {
    cout << "callout.net information: : " << c.first << endl;
    cout << "===========================================================" << endl;
    cout << c.second.ToString() << endl;
  }

  return 0;
}


}
}
