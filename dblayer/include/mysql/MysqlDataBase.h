/*
 * MysqlDataBase.h
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */

#ifndef MYSQLDATABASE_H_
#define MYSQLDATABASE_H_
#include "MysqlRecordSet.h"

/*
  * 1 负责数据库的连接关闭
  * 2 执行sql 语句(不返回结果)
  * 3 处理事务
 */
 class CDataBase
 {
 public :
	 CDataBase();
	 ~CDataBase();
 private :
	 /* msyql 连接句柄 */
	 MYSQL* m_Data;
 public :
	 /* 返回句柄 */
	 MYSQL * GetMysql();
	 /* 连接数据库 */
	 int Connect(string host, string user,
			 string passwd, string db,
			 unsigned int port,
			 unsigned long client_flag);

	 /* 关闭数据库连接 */
	 void DisConnect();
	 /* 执行非返回结果查询 */
	 int ExecQuery(string sql);
	 /* 测试mysql服务器是否存活 */
	 int Ping();
	 /* 关闭mysql 服务器 */
	 int ShutDown();
	 /* 主要功能:重新启动mysql 服务器 */
	 int ReBoot();

	 /*
	  * 说明:事务支持InnoDB or BDB表类型
      */
	 /* 主要功能:开始事务 */
	 int Start_Transaction();
	 /* 主要功能:提交事务 */
	 int Commit();
	 /* 主要功能:回滚事务 */
	 int Rollback();
	 /* 得到客户信息 */
	 const char * Get_client_info();
	 /* 主要功能:得到客户版本信息 */
	 const unsigned long  Get_client_version();
	 /* 主要功能:得到主机信息 */
	 const char * Get_host_info();
	 /* 主要功能:得到服务器信息 */
	 const char * Get_server_info();
	 /*主要功能:得到服务器版本信息*/
	 const unsigned long  Get_server_version();
	 /*主要功能:得到 当前连接的默认字符集*/
	 const char *  Get_character_set_name();
	 /* 主要功能:得到错误代码 */
	 const unsigned int Get_errno();
	 /* 主要功能:得到错误信息 */
	 const char * Get_error();

	 /* 主要功能返回单值查询  */
	 char * ExecQueryGetSingValue(string sql);

	 /* 得到系统时间 */
	 const char * GetSysTime();
  	/* 建立新数据库 */
	 int Create_db(string name);
	 /* 删除制定的数据库*/
	 int Drop_db(string name);
 };

#endif /* MYSQLDATABASE_H_ */
