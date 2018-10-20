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
#include "bbs/output.h"

#include "bbs/bgetch.h"
#include "bbs/bbs.h"
#include "bbs/bbsutl1.h"
#include "bbs/com.h"
#include "bbs/interpret.h"
#include "local_io/keycodes.h"
#include "bbs/pause.h"
#include "bbs/utility.h"
#include "bbs/remote_io.h"
#include "local_io/wconstants.h"
#include "bbs/application.h"
#include "local_io/local_io.h"
#include "core/log.h"
#include "core/strings.h"

#include <algorithm>
#include <string>

using namespace wwiv::strings;

/**
 * This function outputs one character to the screen, and if output to the
 * com port is enabled, the character is output there too.  ANSI graphics
 * are also trapped here, and the ansi function is called to execute the
 * ANSI codes
 */
int Output::bputch(char c, bool use_buffer) {
  int displayed = 0;

  if (c == SOFTRETURN && needs_color_reset_at_newline_) {
    Color(0);
    needs_color_reset_at_newline_ = false;
  }

  if (a()->context().outcom() && c != TAB) {
    if (c == SOFTRETURN) {
#ifdef __unix__
      rputch('\r', use_buffer);
#endif  // __unix__
      rputch('\n', use_buffer);
    } else {
      rputch(c, use_buffer);
    }
    displayed = 1;
  }
  if (c == TAB) {
    int screen_pos = wherex();
    for (int i = screen_pos; i < (((screen_pos / 8) + 1) * 8); i++) {
      displayed += bputch(SPACE);
    }
  } else {
    displayed = 1;
    // Pass through to SDK ansi interpreter.
    auto last_state = ansi_->state();
    ansi_->write(c);

    if (ansi_->state() == wwiv::sdk::ansi::AnsiMode::not_in_sequence &&
        last_state == wwiv::sdk::ansi::AnsiMode::not_in_sequence) {
      // Only add to the current line if we're not in an ansi sequence.
      // Otherwise we get gibberish ansi strings that will be displayed
      // raw to the user.
      current_line_.push_back({c, static_cast<uint8_t>(curatr())});
    }

    const auto screen_width = a()->user()->GetScreenChars();
    if (c == BACKSPACE) {
      --x_;  
      if (x_ < 0) {
        x_ = screen_width - 1;
      }
    } else {
      ++x_;
    }
    // Wrap at screen_width
    if (x_ >= static_cast<int>(screen_width)) {
      x_ %= screen_width;
    }

    if (c == SOFTRETURN) {
      current_line_.clear();
      x_ = 0;
      bout.lines_listed_++;
      // change Build3 + 5.0 to fix message read.
      if (bout.lines_listed() >= (a()->screenlinest - 1)) {
        if (a()->user()->HasPause()) {
          pausescr();
        }
        bout.clear_lines_listed();   // change Build3
      }
    }
  }

  return displayed;
}


/* This function outputs a string to the com port.  This is mainly used
 * for modem commands
 */
void Output::rputs(const char *text) {
  // Rushfan fix for COM/IP weirdness
  if (a()->context().ok_modem_stuff()) {
    a()->remoteIO()->write(text, strlen(text));
  }
}

void Output::flush() {
  if (bputch_buffer_.empty()) {
    return;
  }

  a()->remoteIO()->write(bputch_buffer_.c_str(), bputch_buffer_.size());
  bputch_buffer_.clear();
}

void Output::rputch(char ch, bool use_buffer_) {
  if (a()->context().ok_modem_stuff() && nullptr != a()->remoteIO()) {
    if (use_buffer_) {
      if (bputch_buffer_.size() > 1024) {
        flush();
      }
      bputch_buffer_.push_back(ch);
    } else if (!bputch_buffer_.empty()) {
      // If we have stuff in the buffer, and now are asked
      // to send an unbuffered character, we must send the
      // contents of the buffer 1st.
      bputch_buffer_.push_back(ch);
      flush();
    }
    else {
      a()->remoteIO()->put(ch);
    }
  }
}

