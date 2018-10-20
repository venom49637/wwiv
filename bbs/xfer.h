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
#ifndef __INCLUDED_BBS_XFER_H__
#define __INCLUDED_BBS_XFER_H__

#include <string>

void zap_ed_info();
void get_ed_info();
unsigned long bytes_to_k(unsigned long lBytes);
int  check_batch_queue(const char *file_name);
bool check_ul_event(int directory_num, uploadsrec * upload_record);
bool okfn(const std::string& fileName);
void print_devices();
void get_arc_cmd(char *out_buffer, const char *pszArcFileName, int cmd, const char *ofn);
int  list_arc_out(const char *file_name, const char *pszDirectory);
bool ratio_ok();
bool dcs();
void dliscan1(int directory_num);
void dliscan();
void add_extended_description(const std::string& file_name, const std::string& description);
void delete_extended_description(const std::string&file_name);
std::string read_extended_description(const std::string& file_name);
void print_extended(const char *file_name, bool *abort, int numlist, int indent);
void align(char *file_name);
void align(std::string* file_name);
std::string aligns(const std::string& file_name);
bool compare(const char* pszFileName1, const char* pszFileName2);
void printinfo(uploadsrec * upload_record, bool *abort);
void printtitle(bool *abort);
std::string file_mask();
void listfiles();
void nscandir(uint16_t nDirNum, bool& need_title, bool *abort);
void nscanall();
void searchall();
int  recno(const std::string& file_mask);
int  nrecno(const std::string& file_mask, int nStartingRec);
int  printfileinfo(uploadsrec* upload_record, int directory_num);
void remlist(const char *file_name);
int FileAreaSetRecord(wwiv::core::File& file, int nRecordNumber);

#endif  // __INCLUDED_BBS_XFER_H__