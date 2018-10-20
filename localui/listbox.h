/**************************************************************************/
/*                                                                        */
/*                  WWIV Initialization Utility Version 5                 */
/*               Copyright (C)2014-2017, WWIV Software Services           */
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
#ifndef __INCLUDED_PLATFORM_LISTBOX_H__
#define __INCLUDED_PLATFORM_LISTBOX_H__

#include <algorithm>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "localui/curses_io.h"
#include "localui/curses_win.h"

#ifdef INSERT // defined in wconstants.h
#undef INSERT
#endif // INSERT

class ColorScheme;

class ListBoxItem {
public:
  ListBoxItem(const std::string& text, int hotkey = 0, int data = 0)
      : text_(text), hotkey_(hotkey), data_(data) {}
  ~ListBoxItem() {}

  const std::string& text() const noexcept { return text_; }
  int hotkey() const noexcept { return hotkey_; }
  int data() const noexcept { return data_; }

 private:
  std::string text_;
  int hotkey_;
  int data_;
};

enum class ListBoxResultType { NO_SELECTION, SELECTION, HOTKEY };
struct ListBoxResult {
  ListBoxResultType type;
  int selected;
  int hotkey;
};

// Curses implementation of a list box.
class ListBox {
public:
  // Constructor/Destructor
  ListBox(UIWindow* parent, const std::string& title, std::vector<ListBoxItem>& items);
  ListBox(const ListBox& copy) = delete;
  virtual ~ListBox() {}

  // Execute the listbox returning the index of the selected item.
  ListBoxResult Run() {
    this->DisplayFooter();
    ListBoxResult result = RunDialog();
    out->footer()->SetDefaultFooter();
    return result;
  }

  // Returns the index of the selected item.
  int selected() const { return selected_; }
  void set_selected(int s) { selected_ = s; }

  // List of additionally allowed hotkeys.
  void set_additional_hotkeys(const std::string& hotkeys) { hotkeys_.append(hotkeys); }
  // If true, a selection will return as a hotkey if a hotkey is set on the item.
  void selection_returns_hotkey(bool selection_returns_hotkey) {
    selection_returns_hotkey_ = selection_returns_hotkey;
  }
  // Sets the extra help items.
  void set_help_items(const std::vector<HelpItem> items) { help_items_ = items; }

private:
  ListBox(UIWindow* parent, const std::string& title, int max_x, int max_y,
          std::vector<ListBoxItem>& items, ColorScheme* scheme);
  ListBoxResult RunDialog();
  void DrawAllItems();
  void DisplayFooter();

  int selected_ = -1;
  const std::string title_;
  std::vector<ListBoxItem> items_;
  std::vector<HelpItem> help_items_;
  int window_top_ = 0;
  int width_ = 4;
  int height_ = 2;
  std::unique_ptr<CursesWindow> window_;
  ColorScheme* color_scheme_;
  const int window_top_min_;
  std::string hotkeys_;
  bool selection_returns_hotkey_{false};
};

#endif // __INCLUDED_PLATFORM_LISTBOX_H__
