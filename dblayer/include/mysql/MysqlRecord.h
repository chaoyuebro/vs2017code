/*
 * MysqlRecord.h
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */

#ifndef MYSQLRECORD_H_
#define MYSQLRECORD_H_
#include "MysqlField.h"
 /*
  * 1 单条记录
  * 2 [int ]操作 [""]操作
 */
 class CRecord
 {
 public:
	 /* 结果集 */
	 vector<string> m_rs;
	 /* 字段信息 占用4字节的内存 当记录数很大是会产生性能问题 */
	 CField *m_field;
 public :
	 CRecord();
	 CRecord(CField* m_f);
	 ~CRecord();


	 void SetData(string value);
	 /* [""]操作 */
	 string operator[](string s);
	 string operator[](int num);
	 /* null值判断 */
	 bool IsNull(int num);
	 bool IsNull(string s);
	 /* 用 value tab value 的形式 返回结果 */
	 string GetTabText();
 };


#endif /* MYSQLRECORD_H_ */
