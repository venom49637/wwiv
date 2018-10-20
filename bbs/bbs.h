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
#ifndef __INCLUDED_BBS_H__
#define __INCLUDED_BBS_H__

/*
 * @header WWIV 5 Main Application
 * Main Starting point of the WWIV 5 System.
 */

#include <memory>
#include <string>

#include "bbs/remote_io.h"
#include "bbs/application.h"
#include "sdk/status.h"
#include "sdk/user.h"

// Function Prototypes
Application* CreateSession(LocalIO* localIO);
Application* a();

#endif // __INCLUDED_BBS_H__


