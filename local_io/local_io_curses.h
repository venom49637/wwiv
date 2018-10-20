/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2017, WWIV Software Services            */
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
#ifndef __INCLUDED_LOCAL_IO_CURSES_H__
#define __INCLUDED_LOCAL_IO_CURSES_H__

#include <cstdint>
#include <cstdio>
#include <string>

#include "local_io/local_io.h"
#include "localui/colors.h"
#include "localui/curses_win.h"

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4100)
#endif

class CursesLocalIO : public LocalIO {
public:
  // Constructor/Destructor
  CursesLocalIO();
  explicit CursesLocalIO(int num_lines);
  CursesLocalIO(const LocalIO& copy) = delete;
  virtual ~CursesLocalIO();

  void GotoXY(int x, int y) override;
  int WhereX() const noexcept override;
  int WhereY() const noexcept override;
  void Lf() override;
  void Cr() override;
  void Cls() override;
  void ClrEol() override;
  void Backspace() override;
  void PutchRaw(unsigned char ch) override;
  // Overridden by TestLocalIO in tests
  void Putch(unsigned char ch) override;
  void Puts(const std::string& text) override;
  void PutsXY(int x, int y, const std::string& text) override;
  void PutsXYA(int x, int y, int a, const std::string& text) override;
  void set_protect(int l) override;
  void savescreen() override;
  void restorescreen() override;
  bool KeyPressed() override;
  unsigned char GetChar() override;
  void MakeLocalWindow(int x, int y, int xlen, int ylen) override;
  void SetCursor(int cursorStyle) override;
  void WriteScreenBuffer(const char* buffer) override;
  int GetDefaultScreenBottom() const noexcept override;

  void EditLine(char* s, int len, AllowedKeys allowed_keys, int* returncode,
                const char* ss) override;
  void UpdateNativeTitleBar(const std::string& system_name, int instance_number) override;
  virtual void ResetColors();

  virtual void DisableLocalIO();
  virtual void ReenableLocalIO();

private:
  void FastPuts(const std::string& text) override;
  virtual void SetColor(int color);
  int x_{0};
  int y_{0};

  std::unique_ptr<CursesWindow> window_;
};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif // _MSC_VER

#endif // __INCLUDED_LOCAL_IO_CURSES_H__
