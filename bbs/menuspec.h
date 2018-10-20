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
/**************************************************************************/
#ifndef __INCLUDED_BBS_MENUSPEC_H__
#define __INCLUDED_BBS_MENUSPEC_H__

int MenuDownload(const std::string& pszDirFName, const std::string& pszFName, bool bFreeDL,
                 bool bTitle);
bool MenuRunDoorName(const char *pszDoor, bool bFree);
bool MenuRunDoorNumber(int nDoorNumber, bool bFree);
int  FindDoorNo(const char *pszDoor);
bool ValidateDoorAccess(int nDoorNumber);
void ChangeSubNumber();
void ChangeDirNumber();
void SetMsgConf(int iConf);
void SetDirConf(int iConf);
void EnableConf();
void DisableConf();
void SetNewScanMsg();

#endif  // __INCLUDED_BBS_MENUSPEC_H__