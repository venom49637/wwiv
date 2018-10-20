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
// Always declare wwiv_windows.h first to avoid collisions on defines.
#include "core/wwiv_windows.h"

#include <cmath>

#include "bbs/datetime.h"
#include "local_io/wconstants.h"
#include "core/datetime.h"
#include "core/file.h"
#include "core/strings.h"
#include "core/wwivassert.h"

using std::string;
using namespace std::chrono;
using namespace wwiv::core;

//
// This kludge will get us through 2019 and should not interfere anywhere
// else.
//

void ToggleScrollLockKey() {
#if defined(_WIN32)
  // Simulate a key press
  keybd_event(VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
  // Simulate a key release
  keybd_event(VK_SCROLL, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
#endif // _WIN32
}

/* This function returns the status of scoll lock.  If scroll lock is active
 * (ie, the user has hit scroll lock + the light is lit if there is a
 * scoll lock LED), the sysop is assumed to be available.
 */
bool sysop1() {
#if defined(_WIN32)
  return (GetKeyState(VK_SCROLL) & 0x1);
#else
  return false;
#endif
}

bool isleap(int year) {
  WWIV_ASSERT(year >= 0);
  return year % 400 == 0 || (year % 4 == 0 && year % 100 != 0);
}

/*
 * Returns current time as string formatted like HH:MM:SS (01:13:00).
 */
std::string ctim(long d) {
  if (d < 0) {
    d += SECONDS_PER_DAY;
  }
  auto hour = (d / SECONDS_PER_HOUR);
  d -= (hour * SECONDS_PER_HOUR);
  auto minute = static_cast<long>(d / MINUTES_PER_HOUR);
  d -= (minute * MINUTES_PER_HOUR);
  auto second = static_cast<long>(d);
  return wwiv::strings::StringPrintf("%2.2ld:%2.2ld:%2.2ld", hour, minute, second);
}

std::string ctim(std::chrono::duration<double> d) {
  return ctim(static_cast<long>(std::chrono::duration_cast<seconds>(d).count()));
}

int years_old(int nMonth, int nDay, int nYear) {
  auto t = time_t_now();
  struct tm* pTm = localtime(&t);
  nYear = nYear - 1900;
  --nMonth; // Reduce by one because tm_mon is 0-11, not 1-12

  // Find the range of impossible dates (ie, pTm can't be
  // less than the input date)
  if (pTm->tm_year < nYear) {
    return 0;
  }
  if (pTm->tm_year == nYear) {
    if (pTm->tm_mon < nMonth) {
      return 0;
    }
    if (pTm->tm_mon == nMonth) {
      if (pTm->tm_mday < nDay) {
        return 0;
      }
    }
  }

  int nAge = pTm->tm_year - nYear;
  if (pTm->tm_mon < nMonth) {
    --nAge;
  } else if ((pTm->tm_mon == nMonth) && (pTm->tm_mday < nDay)) {
    --nAge;
  }
  return nAge;
}

system_clock::duration duration_since_midnight(system_clock::time_point now) {
  auto tnow = system_clock::to_time_t(now);
  tm* date = std::localtime(&tnow);
  date->tm_hour = 0;
  date->tm_min = 0;
  date->tm_sec = 0;
  auto midnight = system_clock::from_time_t(std::mktime(date));

  return now - midnight;
}

system_clock::time_point minutes_after_midnight(int minutes) {
  const auto tnow = time_t_now();
  tm* date = std::localtime(&tnow);
  date->tm_hour = minutes / 60;
  date->tm_min = minutes % 60;
  date->tm_sec = 0;

  auto t = system_clock::from_time_t(std::mktime(date));
  return t;
}

int minutes_since_midnight() {
  auto d = duration_since_midnight(system_clock::now());
  return duration_cast<minutes>(d).count();
}
