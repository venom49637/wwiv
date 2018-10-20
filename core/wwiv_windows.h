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
#ifndef __INCLUDED_CORE_WWIV_WINDOWS_H__
#define __INCLUDED_CORE_WWIV_WINDOWS_H__

// Wrapper header file for including windows.h from wwiv.  This sets all of
// the numerous #defines to remove much of the windows header files since the
// surface area used by wwiv is tiny.

#ifdef _WIN32

#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif  // MOUSE_MOVED

#ifdef CY
#undef CY
#endif

#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOATOM
#define NOCLIPBOARD
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NOCRYPT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

#ifdef min
#undef min
#endif  // min

#ifdef max
#undef max
#endif  // max

#ifdef CopyFile
#undef CopyFile
#endif  // CopyFile

#ifdef MOUSE_MOVED
#undef MOUSE_MOVED
#endif  // MOUSE_MOVED

#ifdef CY
#undef CY
#endif

#endif // _WIN32

#endif  // __INCLUDED_CORE_WWIV_WINDOWS_H__