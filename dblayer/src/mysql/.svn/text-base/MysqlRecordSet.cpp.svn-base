/*
 * MysqlCRecordSet.cpp
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */

#include "mysql/MysqlRecordSet.h"
 /*
  * 1 记录集合
  * 2 [int ]操作 [""]操作
  * 3 表结构操作
  * 4 数据的插入修改
 */
 CRecordSet::CRecordSet()
 {
	 m_Data = NULL;
	 m_res = NULL;
	 m_row = NULL;
	 m_pos = 0;
	 m_fd = NULL;
	 m_recordcount = 0;
	 m_field_num = 0;
 }
 CRecordSet::CRecordSet(MYSQL *hSQL)
 {
	 m_res = NULL;
	 m_row = NULL;
	 m_Data = hSQL;
	 m_pos = 0;
	 m_fd = NULL;
	 m_recordcount = 0;
	 m_field_num = 0;
 }
 CRecordSet::~CRecordSet()
 {
 }
 /*
  * 处理返回多行的查询，返回影响的行数
  * 成功返回行数，失败返回-1
 */
 int CRecordSet::ExecuteSQL(const char *SQL)
 {
	 if ( !mysql_real_query(m_Data,SQL,strlen(SQL)))
	 {
		 // init result
		 m_s.clear();
		 //保存查询结果
		 m_res = mysql_store_result(m_Data );
		 //得到记录数量
		 m_recordcount = (int)mysql_num_rows(m_res) ;
		 //得到字段数量
		 m_field_num = mysql_num_fields(m_res) ;
		 for (int x = 0 ; (m_fd = mysql_fetch_field(m_res)); x++)
		 {
			 m_field.m_name.push_back(m_fd->name);
			 m_field.m_type.push_back(m_fd->type);
		 }
		 //保存所有数据
		 while ((m_row = mysql_fetch_row(m_res)))
		 {
			 CRecord temp(&m_field);
			 for (int k = 0 ; k < m_field_num ; k++ )
			 {

				 if(m_row[k] == NULL || (!strlen(m_row[k])))
				 {
					 temp.SetData ("");
				 }
				 else
				 {
					 temp.SetData(m_row[k]);
				 }
			 }
			 //添加新记录
			 m_s.push_back (temp);
		 }
		 mysql_free_result(m_res ) ;

		 return m_s.size();
	 }
	 return -1;
 }
 /*
  * 向下移动游标
  * 返回移动后的游标位置
 */
 long CRecordSet::MoveNext()
 {
	 return (++m_pos);
 }
 /* 移动游标 */
 long  CRecordSet::Move(long length)
 {
	 int l = m_pos + length;

	 if(l<0)
	 {
		 m_pos = 0;
		 return 0;
	 }
	 else
	 {
		 if(l >= (int)m_s.size())
		 {
			 m_pos = m_s.size()-1;
			 return m_pos;
		 }
		 else
		 {
			 m_pos = l;
			 return m_pos;
		 }
	 }
 }
 /* 移动游标到开始位置 */
 bool CRecordSet::MoveFirst()
 {
	 m_pos = 0;
	 return true;
 }
 /* 移动游标到结束位置 */
 bool CRecordSet::MoveLast()
 {
	 m_pos = m_s.size()-1;
	 return true;
 }
 /* 获取当前游标位置 */
 unsigned long CRecordSet::GetCurrentPos()const
 {
	 return m_pos;
 }
 /* 获取当前游标的对应字段数据 */
 bool CRecordSet::GetCurrentFieldValue(const char * sFieldName, char *sValue)
 {
	 strcpy(sValue, m_s[m_pos][sFieldName].c_str());
	 return true;
 }
 bool CRecordSet::GetCurrentFieldValue(const int iFieldNum, char *sValue)
 {
	 strcpy(sValue, m_s[m_pos][iFieldNum].c_str());
	 return true;
 }
 /* 获取游标的对应字段数据 */
 bool CRecordSet::GetFieldValue(long index,const char * sFieldName, char *sValue)
 {
	 strcpy(sValue,	m_s[index][sFieldName].c_str());
	 return true;
 }
 bool CRecordSet::GetFieldValue(long index, int iFieldNum, char *sValue)
 {
	 strcpy(sValue, m_s[index][iFieldNum].c_str());
	 return true;
 }
 /* 是否到达游标尾部 */
 bool CRecordSet::IsEof()
 {
	 return (m_pos == m_s.size())?true:false;
 }
 /*
  * 得到记录数目
 */
 int CRecordSet::GetRecordCount()
 {
	 return m_recordcount;
 }
 /*
  * 得到字段数目
 */
 int CRecordSet::GetFieldNum()
 {
	 return m_field_num;
 }
 /*
  * 返回字段
 */
 CField * CRecordSet::GetField()
 {
	 return &m_field;
 }
 /* 返回字段名 */
 const char * CRecordSet::GetFieldName(int iNum)
 {
	 return m_field.m_name.at(iNum).c_str();
 }
 /* 返回字段类型 */
 const int CRecordSet::GetFieldType(char * sName)
 {
	 int i = m_field.GetField_NO(sName);
	 return m_field.m_type.at(i);
 }
 const int CRecordSet::GetFieldType(int iNum)
 {
	 return m_field.m_type.at(iNum);
 }
 /*
  * 返回指定序号的记录
 */
 CRecord CRecordSet::operator[](int num)
 {
	 return m_s[num];
 }


