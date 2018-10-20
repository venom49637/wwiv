/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)2014-2017, WWIV Software Services             */
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
#ifndef __INCLUDED_SAVE_QSCAN_H__
#define __INCLUDED_SAVE_QSCAN_H__

#include <cstdint>
#include <memory>

namespace wwiv {
namespace bbs {

class SaveQScanPointers {
public:
  SaveQScanPointers();
  virtual ~SaveQScanPointers();
  void restore() { restore_ = true; }
private:
  bool restore_;
  std::unique_ptr<uint32_t[]> save_qsc_p_;
};

}  // namespace bbs
}  // namespace wwiv

#endif  // __INCLUDED_SAVE_QSCAN_H__
