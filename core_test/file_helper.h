/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2017, WWIV Software Services             */
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
#ifndef __INCLUDED_FILE_HELPER_H__
#define __INCLUDED_FILE_HELPER_H__

#include <cstdio>
#include <string>

/**
 * Helper class for tests requing local filesystem access.  
 *
 * Note: This class can not use File since it is used by the tests for File.
 */
class FileHelper {
public:
  FileHelper();
  // Returns a fully qualified path name to "name" under the temporary directory.
  const std::string DirName(const std::string& name) const;
  // Creates a directory under TempDir.
  bool Mkdir(const std::string& name) const;
  std::string CreateTempFilePath(const std::string& name);
  FILE* OpenTempFile(const std::string& name, std::string* path);
  std::string CreateTempFile(const std::string& name, const std::string& contents);
  const std::string& TempDir() const { return tmp_; }
  const std::string ReadFile(const std::string name) const;
  static void set_wwiv_test_tempdir(const std::string& d) { basedir_ = d; }
private:
  static std::string GetTestTempDir();
  static std::string CreateTempDir(const std::string base);
  std::string tmp_;
  static std::string basedir_;
};

#endif // __INCLUDED_FILE_HELPER_H__