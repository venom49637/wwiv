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
#pragma once
#ifndef __INCLUDED_NETWORKB_WFILE_TRANSFER_FILE_H__
#define __INCLUDED_NETWORKB_WFILE_TRANSFER_FILE_H__

#include <chrono>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <core/file.h>

#include "sdk/fido/fido_util.h"
#include "networkb/transfer_file.h"

namespace wwiv {
namespace net {
  
class WFileTransferFile : public TransferFile {
public:
  WFileTransferFile(const std::string& filename, std::unique_ptr<wwiv::core::File>&& file);
  virtual ~WFileTransferFile();

  virtual int file_size() const override final;
  bool Delete() override final;
  bool GetChunk(char* chunk, std::size_t start, std::size_t size) override final;
  bool WriteChunk(const char* chunk, std::size_t size) override final;
  virtual bool Close() override final;
  void set_flo_file(std::unique_ptr<wwiv::sdk::fido::FloFile>&& f) { flo_file_ = std::move(f); }

 private:
  std::unique_ptr<wwiv::core::File> file_; 
  std::unique_ptr<wwiv::sdk::fido::FloFile> flo_file_;
};


}  // namespace net
}  // namespace wwiv

#endif  // __INCLUDED_NETWORKB_WFILE_TRANSFER_FILE_H__
