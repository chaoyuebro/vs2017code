/*
 * MysqlCField.cpp
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */

#include "mysql/MysqlField.h"

CField::CField(){}
 CField::~CField(){}
 /*
  * 是否是数字
 */
 bool CField::IsNum(int num)
 {
	 if(IS_NUM(m_type[num]))
		 return true;
	 else
		 return false;
 }
 /*
  * 是否是数字
 */
 bool CField::IsNum(string num)
 {
	 if(IS_NUM(m_type[GetField_NO(num)]))
		 return true;
	 else
		 return false;
 }
 /*
  * 是否是日期
 */
 bool CField::IsDate(int num)
 {
	 if( FIELD_TYPE_DATE == m_type[num] ||
			 FIELD_TYPE_DATETIME == m_type[num] )
		 return true;
	 else
		 return false;
 }
 /* 是否是日期 */
 bool CField::IsDate(string num)
 {
	 int temp;
	 temp = GetField_NO(num);
	 if(FIELD_TYPE_DATE == m_type[temp] ||
			 FIELD_TYPE_DATETIME == m_type[temp] )
		 return true;
	 else
		 return false;
 }
 /*
  * 是否是字符
 */
 bool CField::IsChar(int num)
 {
	 if(m_type[num] == FIELD_TYPE_STRING  ||
		  m_type[num] == FIELD_TYPE_VAR_STRING ||
		  m_type[num] == FIELD_TYPE_CHAR )
		 return true;
	 else
		 return false;
 }
 /*
  * 是否是字符
 */
 bool CField::IsChar(string num)
 {
	 int temp;
	 temp = this->GetField_NO (num);
	 if(m_type[temp] == FIELD_TYPE_STRING  ||
			 m_type[temp] == FIELD_TYPE_VAR_STRING ||
			 m_type[temp] == FIELD_TYPE_CHAR )
		 return true;
	 else
		 return false;
 }
 /*
  * 是否为二进制数据
 */
 bool CField::IsBlob(int num)
 {
	 if(IS_BLOB(m_type[num]))
		 return true;
	 else
		 return false;
 }
 /*
  * 是否为二进制数据
 */
 bool CField::IsBlob(string num)
 {
	 if(IS_BLOB(m_type[GetField_NO(num)]))
		 return true;
	 else
		 return false;
 }
 /*
  * 得到指定字段的序号
 */
 int CField::GetField_NO(string Field_name)
 {
	 for(unsigned int i = 0; i < m_name.size (); i++)
	 {
		 if(!m_name[i].compare(Field_name))
			 return i;
	 }
	 return -1;
 }
