/*
 * MysqlRecordSet.h
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */

#ifndef MYSQLRECORDSET_H_
#define MYSQLRECORDSET_H_

#include "MysqlRecord.h"
/*
  * 1 记录集合
  * 2 [int ]操作 [""]操作
  * 3 表结构操作
  * 4 数据的插入修改
 */
 class CRecordSet
 {
 private :
	 /* 记录集 */
	 vector<CRecord> m_s;
	 /* 游标位置*/
	 unsigned long m_pos;
	 /* 记录数 */
	 int m_recordcount;
	 /* 字段数 */
	 int m_field_num;
	 /* 字段信息 */
	 CField  m_field;

	 MYSQL_RES * m_res ;
	 MYSQL_FIELD * m_fd ;
	 MYSQL_ROW m_row;
	 MYSQL* m_Data ;
 public :
	 CRecordSet();
	 CRecordSet(MYSQL *hSQL);
	 ~CRecordSet();

	 /* 处理返回多行的查询，返回影响的行数 */
	 int ExecuteSQL(const char *SQL);
	 /* 得到记录数目 */
	 int GetRecordCount();
	 /* 得到字段数目 */
	 int GetFieldNum();
	 /* 向下移动游标 */
	 long MoveNext();
	 /* 移动游标 */
	 long Move(long length);
	 /* 移动游标到开始位置 */
	 bool MoveFirst();
	 /* 移动游标到结束位置 */
	 bool MoveLast();
	 /* 获取当前游标位置 */
	 unsigned long GetCurrentPos()const;
	 /* 获取当前游标的对应字段数据 */
	 bool GetCurrentFieldValue(const char * sFieldName,char *sValue);
	 bool GetCurrentFieldValue(const int iFieldNum,char *sValue);
	 /* 获取游标的对应字段数据 */
	 bool GetFieldValue(long index,const char * sFieldName,char *sValue);
	 bool GetFieldValue(long index,int iFieldNum,char *sValue);
	 /* 是否到达游标尾部 */
	 bool IsEof();

	 /* 返回字段 */
	 CField* GetField();
	 /* 返回字段名 */
	 const char * GetFieldName(int iNum);
	 /* 返回字段类型 */
	 const int GetFieldType(char * sName);
  	const int GetFieldType(int iNum);
  	/* 返回指定序号的记录 */
  	CRecord operator[](int num);

 };


#endif /* MYSQLRECORDSET_H_ */
