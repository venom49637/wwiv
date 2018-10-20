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
#include <chrono>
#include <cmath>
#include <string>

#include "bbs/bgetch.h"
#include "bbs/remote_io.h"
#include "bbs/bbs.h"
#include "bbs/com.h"
#include "bbs/crc.h"
#include "bbs/datetime.h"
#include "local_io/keycodes.h"
#include "bbs/sr.h"
#include "bbs/utility.h"

#include "bbs/xfer.h"

#include "core/strings.h"

using std::string;
using namespace wwiv::core;
using namespace std::chrono;
using namespace wwiv::strings;

// From zmwwiv.cpp
bool NewZModemReceiveFile(const char *file_name);

// from sr.cpp
extern unsigned char checksum;


char modemkey(int *tout) {
  if (bkbhitraw()) {
    char ch = bgetchraw();
    calc_CRC(ch);
    return ch;
  }
  if (*tout) {
    return 0;
  }
  auto d1 = steady_clock::now();
  while (steady_clock::now() - d1 < milliseconds(500) && !bkbhitraw() && !a()->hangup_) {
    CheckForHangup();
  }
  if (bkbhitraw()) {
    char ch = bgetchraw();
    calc_CRC(ch);
    return ch;
  }
  *tout = 1;
  return 0;
}



int receive_block(char *b, unsigned char *bln, bool use_crc) {
  bool abort = false;
  unsigned char ch = gettimeout(5, &abort);
  int err = 0;

  if (abort) {
    return CF;
  }
  int tout = 0;
  if (ch == 0x81) {
    unsigned char bn = modemkey(&tout);
    unsigned char bn1 = modemkey(&tout);
    if ((bn ^ bn1) == 0xff) {
      b[0] = bn;
      *bln = bn;
      return 8;
    } else {
      return 3;
    }
  } else if (ch == 1) {
    unsigned char bn = modemkey(&tout);
    unsigned char bn1 = modemkey(&tout);
    if ((bn ^ bn1) != 0xff) {
      err = 3;
    }
    *bln = bn;
    crc = 0;
    checksum = 0;
    for (int i = 0; (i < 128) && (!a()->hangup_); i++) {
      b[i] = modemkey(&tout);
    }
    if (!use_crc && !a()->hangup_) {
      unsigned char cs1 = checksum;
      bn1 = modemkey(&tout);
      if (bn1 != cs1) {
        err = 2;
      }
    } else if (!a()->hangup_) {
      int cc1 = crc;
      bn = modemkey(&tout);
      bn1 = modemkey(&tout);
      if ((bn != (unsigned char)(cc1 >> 8)) || (bn1 != (unsigned char)(cc1 & 0x00ff))) {
        err = 2;
      }
    }
    if (tout) {
      return 7;
    }
    return err;
  } else if (ch == 2) {
    unsigned char bn = modemkey(&tout);
    unsigned char bn1 = modemkey(&tout);
    crc = 0;
    checksum = 0;
    if ((bn ^ bn1) != 0xff) {
      err = 3;
    }
    *bln = bn;
    for (int i = 0; (i < 1024) && (!a()->hangup_); i++) {
      b[i] = modemkey(&tout);
    }
    if (!use_crc && !a()->hangup_) {
      unsigned char cs1 = checksum;
      bn1 = modemkey(&tout);
      if (bn1 != cs1) {
        err = 2;
      }
    } else if (!a()->hangup_) {
      int cc1 = crc;
      bn = modemkey(&tout);
      bn1 = modemkey(&tout);
      if ((bn != (unsigned char)(cc1 >> 8)) || (bn1 != (unsigned char)(cc1 & 0x00ff))) {
        err = 2;
      }
    }
    if (tout) {
      return 7;
    }
    return (err == 0) ? 1 : err;
  } else if (ch == CX) {
    return 4;
  } else if (ch == 4) {
    return 5;
  } else if (ch == 0) {
    return 7;
  } else {
    return 9;
  }
}

void xymodem_receive(const std::string& file_name, bool *received, bool use_crc) {
  char b[1025], x[81], ch;
  unsigned char bln;
  int i1, i2, i3;

  File::Remove(file_name);
  bool ok = true;
  bool lastcan = false;
  bool lasteot = false;
  int  nTotalErrors = 0;
  int  nConsecErrors = 0;

  File file(file_name);
  if (!file.Open(File::modeBinary | File::modeCreateFile | File::modeReadWrite)) {
    bout << "\r\n\nDOS error - Can't create file.\r\n\n";
    *received = false;
    return;
  }
  long pos = 0L;
  long reallen = 0L;
  time_t filedatetime = 0L;
  unsigned int bn = 1;
  bool done = false;
  double tpb = (12.656) / ((double)(a()->modem_speed_));
  bout << "\r\n-=> Ready to receive, Ctrl+X to abort.\r\n";
  int nOldXPos = a()->localIO()->WhereX();
  int nOldYPos = a()->localIO()->WhereY();
  a()->localIO()->PutsXY(52, 0, "\xB3 Filename :               ");
  a()->localIO()->PutsXY(52, 1, "\xB3 Xfer Time:               ");
  a()->localIO()->PutsXY(52, 2, "\xB3 File Size:               ");
  a()->localIO()->PutsXY(52, 3, "\xB3 Cur Block: 1 - 1k        ");
  a()->localIO()->PutsXY(52, 4, "\xB3 Consec Errors: 0         ");
  a()->localIO()->PutsXY(52, 5, "\xB3 Total Errors : 0         ");
  a()->localIO()->PutsXY(52, 6,
                                       "\xC0\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4\xC4");
  a()->localIO()->PutsXY(65, 0, stripfn(file_name));
  int nNumStartTries = 0;
  do {
    if (nNumStartTries++ > 9) {
      *received = false;
      file.Close();
      file.Delete();
      return;
    }
    if (use_crc) {
      bout.rputch('C');
    } else {
      bout.rputch(CU);
    }

    auto d1 = steady_clock::now();
    while (steady_clock::now() - d1 < seconds(10) && !bkbhitraw() && !a()->hangup_) {
      CheckForHangup();
      if (a()->localIO()->KeyPressed()) {
        ch = a()->localIO()->GetChar();
        if (ch == 0) {
          a()->localIO()->GetChar();
        } else if (ch == ESC) {
          done = true;
          ok = false;
        }
      }
    }
  } while (!bkbhitraw() && !a()->hangup_);

  int i = 0;
  do {
    bln = 255;
    a()->localIO()->PutsXY(69, 4, StringPrintf("%d  ", nConsecErrors));
    a()->localIO()->PutsXY(69, 5, std::to_string(nTotalErrors));
    a()->localIO()->PutsXY(65, 3, StringPrintf("%ld - %ldk", pos / 128 + 1, pos / 1024 + 1));
    const string t = ctim(std::lround((reallen - pos) * tpb));
    if (reallen) {
      a()->localIO()->PutsXY(65, 1, t);
    }
    i = receive_block(b, &bln, use_crc);
    if (i == 0 || i == 1) {
      if (bln == 0 && pos == 0L) {
        i1 = strlen(b) + 1;
        i3 = i1;
        while (b[i3] >= '0' && b[i3] <= '9' && (i3 - i1) < 15) {
          x[i3 - i1] = b[i3];
          i3++;
        }
        x[i3 - i1] = '\0';
        reallen = to_number<long>(x);
        a()->localIO()->PutsXY(
            65, 2, StringPrintf("%ld - %ldk", (reallen + 127) / 128, bytes_to_k(reallen)));
        while ((b[i1] != SPACE) && (i1 < 64)) {
          ++i1;
        }
        if (b[i1] == SPACE) {
          ++i1;
          while ((b[i1] >= '0') && (b[i1] <= '8')) {
            filedatetime = (filedatetime * 8) + static_cast<long>(b[i1] - '0');
            ++i1;
          }
        }
        bout.rputch(CF);
      } else if ((bn & 0x00ff) == static_cast<unsigned int>(bln)) {
        file.Seek(pos, File::Whence::begin);
        long lx = reallen - pos;
        i2 = (i == 0) ? 128 : 1024;
        if ((static_cast<long>(i2) > lx) && reallen) {
          i2 = static_cast<int>(lx);
        }
        file.Write(b, i2);
        pos += static_cast<long>(i2);
        ++bn;
        bout.rputch(CF);
      } else if (((bn - 1) & 0x00ff) == static_cast<unsigned int>(bln)) {
        bout.rputch(CF);
      } else {
        bout.rputch(CX);
        ok = false;
        done = true;
      }
      nConsecErrors = 0;
    } else if (i == 2 || i == 7 || i == 3) {
      if (pos == 0L && reallen == 0L && use_crc) {
        bout.rputch('C');
      } else {
        bout.rputch(CU);
      }
      ++nConsecErrors;
      ++nTotalErrors;
      if (nConsecErrors > 9) {
        bout.rputch(CX);
        ok = false;
        done = true;
      }
    } else if (i == CF) {
      ok = false;
      done = true;
      bout.rputch(CX);
    } else if (i == 4) {
      if (lastcan) {
        ok = false;
        done = true;
        bout.rputch(CF);
      } else {
        lastcan = true;
        bout.rputch(CU);
      }
    } else  if (i == 5) {
      lasteot = true;
      if (lasteot) {
        done = true;
        bout.rputch(CF);
      } else {
        lasteot = true;
        bout.rputch(CU);
      }
    } else if (i == 8) {
      // This used to be where the filetype was set.
      //*ft = bln;
      bout.rputch(CF);
      nConsecErrors = 0;
    } else if (i == 9) {
      bout.dump();
    }
    if (i != 4) {
      lastcan = false;
    }
    if (i != 5) {
      lasteot = false;
    }
  } while (!a()->hangup_ && !done);
  a()->localIO()->GotoXY(nOldXPos, nOldYPos);
  if (ok) {
    file.Close();
    if (filedatetime) {
      file.set_last_write_time(filedatetime);
    }
    *received = true;
  } else {
    file.Close();
    file.Delete();
    *received = false;
  }
}

void zmodem_receive(const string& filename, bool *received) {
  string local_filename(filename);
  wwiv::strings::StringRemoveWhitespace(&local_filename);

  bool bOldBinaryMode = a()->remoteIO()->binary_mode();
  a()->remoteIO()->set_binary_mode(true);
  bool bResult = NewZModemReceiveFile(local_filename.c_str());
  a()->remoteIO()->set_binary_mode(bOldBinaryMode);

  *received = (bResult) ? true : false;
}
