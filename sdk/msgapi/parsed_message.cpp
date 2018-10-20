/**************************************************************************/
/*                                                                        */
/*                            WWIV Version 5                              */
/*               Copyright (C)2018, WWIV Software Services                */
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
#include "sdk/msgapi/parsed_message.h"

#include "core/stl.h"
#include "core/strings.h"

#include <functional>
#include <string>
#include <vector>

namespace wwiv {
namespace sdk {
namespace msgapi {

using std::string;
using namespace wwiv::stl;
using namespace wwiv::strings;

constexpr char CZ = 26;

static std::vector<std::string> split_wwiv_style_message_text(const std::string& s) {
  std::string temp(s);
  // Instead of splitting on \r\n, we remove the \n and then
  // split on just \r.  This also is great that it handles
  // the cases where we end in only \r properly.
  temp.erase(std::remove(temp.begin(), temp.end(), 10), temp.end());
  // Use SplitString(..., false) so we don't skip blank lines.
  return SplitString(temp, "\r", false);
}

ParsedMessageText::ParsedMessageText(const std::string& control_char, const std::string& text,
                                     splitfn split_fn, const std::string& eol)
    : control_char_(control_char), split_fn_(split_fn), eol_(eol) {
  if (text.empty()) {
    return;
  }
  if (text.back() == CZ) {
    auto t = text;
    t.pop_back();
    lines_ = split_fn(t);
  } else {
    lines_ = split_fn(text);
  }
  // TODO, lines needs to be a structure thatincludes metadata
  // like  centered line, or soft-wrapped line.  WWIV and
  // FTN have different ways of soft-wrapping and need
  // to handle both.
}

ParsedMessageText::~ParsedMessageText() = default;

bool ParsedMessageText::add_control_line_after(const std::string& near_line,
                                               const std::string& line) {
  for (auto it = std::begin(lines_); it != std::end(lines_); ) {
    auto l = *it;
    if (!l.empty() && starts_with(l, control_char_)) {
      l = l.substr(control_char_.size());
      if (l.find(near_line) != string::npos) {
        // current item has it.
        if (it == lines_.end()) {
          // at the end of the list, add to the end.
          lines_.push_back(StrCat(control_char_, line));
        } else {
          // not at the end of the list, add it *after* the current item.
          it++;
          lines_.insert(it, StrCat(control_char_, line));
        }
        return true;
      }
    }
    it++;
  }
  return false;
}

bool ParsedMessageText::add_control_line(const std::string& line) {
  auto it = lines_.begin();
  bool found_control_line = false;
  while (it != lines_.end()) {
    auto l = *it;
    if (l.empty()) {
      it++;
      continue;
    }
    if (starts_with(l, control_char_)) {
      found_control_line = true;
    } else if (found_control_line) {
      // We've seen control lines before, and now we don't.
      // Insert here so we're at the end of the control lines.
      lines_.insert(it, StrCat(control_char_, line));
      return true;
    }
    it++;
  }
  lines_.push_back(StrCat(control_char_, line));
  return true;
}

bool ParsedMessageText::remove_control_line(const std::string& start_of_line) {
  for (auto it = std::begin(lines_); it != std::end(lines_);) {
    auto l = *it;
    if (!l.empty() && starts_with(l, control_char_)) {
      l = l.substr(control_char_.size());
      if (l.find(start_of_line) != string::npos) {
        it = lines_.erase(it);
        return true;
      }
    }
    it++;
  }
  return false;
}


std::string ParsedMessageText::to_string() const {
  return JoinStrings(lines_, eol_) + static_cast<char>(CZ);
}

WWIVParsedMessageText::WWIVParsedMessageText(const std::string& text)
    : ParsedMessageText("\004"
                        "0",
                        text, split_wwiv_style_message_text,
                        "\r\n") {}
  
WWIVParsedMessageText::~WWIVParsedMessageText() {}

} // namespace msgapi
} // namespace sdk
} // namespace wwiv
