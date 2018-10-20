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
#ifndef __INCLUDED_SDK_PHONE_NUMBERS_H__
#define __INCLUDED_SDK_PHONE_NUMBERS_H__

#include <string>
#include <vector>
#include "sdk/config.h"
#include "sdk/vardec.h"

namespace wwiv {
namespace sdk {

class PhoneNumbers {
public:
  explicit PhoneNumbers(const Config& config);
  virtual ~PhoneNumbers();

  bool IsInitialized() const { return initialized_; }
  bool insert(int user_number, const std::string& phone_number);
  bool erase(int user_number, const std::string& phone_number);
  int find(const std::string& phone_number) const;

private:
  bool Load();
  bool Save();

  bool initialized_;
  std::string datadir_;
  std::vector<phonerec> phones_;
};


}
}


#endif  // __INCLUDED_SDK_PHONE_NUMBERS_H__
