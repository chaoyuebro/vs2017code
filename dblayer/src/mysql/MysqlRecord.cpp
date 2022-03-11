/*
 * MysqlRecord.cpp
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */
#include "mysql/MysqlRecord.h"

/*
  * 1 单条记录
  * 2 [int ]操作 [""]操作
 */

 CRecord::CRecord()
 {
	 m_field = NULL;
 }

 CRecord::CRecord(CField * m_f)
 {
	 m_field = m_f;
 }

 CRecord::~CRecord()
 {
//	 free(m_field);
 }

 void CRecord::SetData(string value)
 {
	 m_rs.push_back (value);
 }

 /* [""]操作 */
 string CRecord::operator[](string s)
 {
	 return m_rs[m_field->GetField_NO(s)];
 }

 string CRecord::operator[](int num)
 {
	 return m_rs[num];
 }

 /* null值判断 */
 bool CRecord::IsNull(int num)
 {
	 //if("" == m_rs[num].c_str ())
	 if(m_rs[num].empty())
	 {
		 return true;
	 }
	 else
	 {
		 return false;
	 }
 }

 bool CRecord::IsNull(string s)
 {
	 //if("" == m_rs[m_field->GetField_NO(s)].c_str())
	 if(m_rs[m_field->GetField_NO(s)].empty())
		 return true;
	 else
		 return false;
 }

 /* 主要-功能:用 value tab value 的形式 返回结果 */
 string CRecord::GetTabText()
 {
	 string temp;
	 for(unsigned int i = 0; i < m_rs.size(); i++)
	 {
		 temp += m_rs[i];
		 if(i < m_rs.size () - 1)
			 temp += "\t";
	 }
	 return temp;
 }

