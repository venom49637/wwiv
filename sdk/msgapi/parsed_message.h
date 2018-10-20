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
#ifndef __INCLUDED_SDK_PARSED_MESSAGE_H__
#define __INCLUDED_SDK_PARSED_MESSAGE_H__

#include <functional>
#include <string>
#include <vector>

namespace wwiv {
namespace sdk {
namespace msgapi {

/**
 * Representes a parsed (split into lines) message.  The format can either be in
 * WWIV native (control lines start with ^D0) or FTN (control lines start with ^A),
 * depending on the control_char specified.
 */
class ParsedMessageText {
public:
  typedef std::function<std::vector<std::string>(const std::string&)> splitfn;

  ParsedMessageText(const std::string& control_char, const std::string& text, splitfn s, const std::string& eol);
  virtual ~ParsedMessageText();
  bool add_control_line_after(const std::string& near_line, const std::string& line);
  bool add_control_line(const std::string& line);
  /** 
   * Removes a single control line starting with start_of_line 
   * returns true if a line was removed, false otherwise
   */
  bool remove_control_line(const std::string& start_of_line);
  std::string to_string() const;

private:
  const std::string control_char_;
  std::vector<std::string> lines_;
  splitfn split_fn_;
  const std::string eol_;
};

/**
 * Representes a parsed (split into lines) message  in WWIV format (control
 * lines start with ^D0) and end of line is "\r\n".
 */
class WWIVParsedMessageText : public ParsedMessageText {
public:
  WWIVParsedMessageText(const std::string& text);
  ~WWIVParsedMessageText();
};

} // namespace msgapi
} // namespace sdk
} // namespace wwiv

#endif // __INCLUDED_SDK_PARSED_MESSAGE_H__
