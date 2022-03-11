/*
 * MysqlCDataBase.cpp
 *
 *  Created on: May 8, 2014
 *      Author: wangpf
 */
#include "mysql/MysqlDataBase.h"
#include "etp/benchmark.h"

 /*
 * 1 负责数据库的连接关闭
 * 2 执行sql 语句(不返回结果)
 * 3 处理事务
 */
 CDataBase::CDataBase()
 {
	 m_Data = NULL;
 }
 CDataBase::~CDataBase()
 {
	 if(NULL != m_Data)
	 {
		 DisConnect();
	 }
 }
 /* 返回句柄 */
 MYSQL * CDataBase::GetMysql()
 {
	 return m_Data;
 }
 /*
  * 主要功能:连接数据库
  * 参数说明:
  * 1 host 主机ip地址或者时主机名称
  * 2 user 用户名
  * 3 passwd 密码
  * 4 db 欲连接的数据库名称
  * 5 port 端口号
  * 6 uinx 嵌套字
  * 7 client_flag 客户连接参数
  * 返回值: 0成功 -1 失败
 */
 int CDataBase::Connect(string host, string user,
        string passwd, string db,
        unsigned int port,
        unsigned long client_flag)
 {
	 BEGIN_BENCHMARK();
	 char arg = 1;
	 int time_out = 1;
	 if((m_Data = mysql_init(NULL))
			 && (0 == mysql_options(m_Data, MYSQL_OPT_RECONNECT, &arg))
			 && (0 == mysql_options(m_Data, MYSQL_OPT_READ_TIMEOUT, &time_out))
			 && mysql_real_connect( m_Data, host.c_str(),
					 user.c_str(), passwd.c_str(),
					 db.c_str(),port , NULL,
					 client_flag))
	 {
		 mysql_set_character_set(m_Data, "gb2312");
		 //选择制定的数据库失败
		 if ( mysql_select_db( m_Data, db.c_str () ) < 0 )
		 {
			 mysql_close( m_Data) ;
			 return -1 ;
		 }
	 }
	 else
	 {
		 //初始化mysql结构失败
		 mysql_close( m_Data );
		 return -1 ;
	 }
	END_BENCHMARK();
	std::cout << "connect success time:" << TIME_INTERVAL_NS() << std::endl;
	 //成功
	 return 0;
 }
 /*
  * 关闭数据库连接
 */
 void CDataBase::DisConnect( )
 {
	 mysql_close(m_Data) ;
	 m_Data = NULL;
 }
 /*
  * 主要功能: 执行非返回结果查询
  * 参数:sql 待执行的查询语句
  * 返回值; n为成功 表示受到影响的行数 -1 为执行失败
 */
 int CDataBase::ExecQuery(string sql)
 {
	 if(!mysql_real_query(m_Data,sql.c_str (),(unsigned long)sql.length()) )
	 {
		 //得到受影响的行数
		 return (int)mysql_affected_rows(m_Data) ;
	 }
	 else
	 {
		 //执行查询失败
		 return -1;
	 }
 }
 /*
  * 主要功能:测试mysql服务器是否存活
  * 返回值:0 表示成功 -1 失败
 */
 int CDataBase::Ping()
 {
//    BEGIN_BENCHMARK();
	 if(m_Data != NULL && !mysql_ping(m_Data))
	 {
//		END_BENCHMARK();
//		std::cout << "ping success time:" << TIME_INTERVAL_NS() << std::endl;
		 return 0;
	 }
	 else
	 {
//		END_BENCHMARK();
//		std::cout << "ping faild time:" << TIME_INTERVAL_NS() << std::endl;
		 return -1;
	 }
 }
 /*
  *  主要功能:关闭mysql 服务器
  * 返回值;0成功 -1 失败
 */
 int CDataBase::ShutDown()
 {
	 if(!mysql_shutdown(m_Data,SHUTDOWN_DEFAULT))
		 return 0;
	 else
		 return -1;
 }
 /*
  * 主要功能:重新启动mysql 服务器
  * 返回值;0表示成功 -1 表示失败
 */
 int CDataBase::ReBoot()
 {
	 if(!mysql_reload(m_Data))
		 return 0;
	 else
		 return -1;
 }

 /*
  * 说明:事务支持InnoDB or BDB表类型
    */
    /*
  * 主要功能:开始事务
 */
 int CDataBase::Start_Transaction()
 {
	 if(!mysql_real_query(m_Data, "START TRANSACTION" ,
			 (unsigned long)strlen("START TRANSACTION") ))
	 {
		 return 0;
	 }
	 else
		 //执行查询失败
		 return -1;
 }
 /*
  * 主要功能:提交事务
  * 返回值:0 表示成功 -1 表示失败
 */
 int CDataBase::Commit()
 {
	 if(!mysql_real_query( m_Data, "COMMIT",
			 (unsigned long)strlen("COMMIT") ) )
	 {
		 return 0;
	 }
	 else
		 //执行查询失败
		 return -1;
 }
 /*
  * 主要功能:回滚事务
  * 返回值:0 表示成功 -1 表示失败
 */
 int CDataBase::Rollback()
 {
	 if(!mysql_real_query(m_Data, "ROLLBACK",
			 (unsigned long)strlen("ROLLBACK") ) )
		 return 0;
	 else
		 //执行查询失败
		 return -1;
 }
 /* 得到客户信息 */
 const char * CDataBase::Get_client_info()
 {
	 return mysql_get_client_info();
 }
 /*主要功能:得到客户版本信息*/
 const unsigned long CDataBase::Get_client_version()
 {
	 return mysql_get_client_version();
 }
 /* 主要功能:得到主机信息 */
 const char * CDataBase::Get_host_info()
 {
	 return mysql_get_host_info(m_Data);
 }
 /* 主要功能:得到服务器信息 */
 const char * CDataBase::Get_server_info()
 {
	 return mysql_get_server_info( m_Data );
 }
 /* 主要功能:得到服务器版本信息 */
 const unsigned long CDataBase::Get_server_version()
 {
	 return mysql_get_server_version(m_Data);
 }
 /*主要功能:得到 当前连接的默认字符集*/
 const char * CDataBase::Get_character_set_name()
 {
	 return mysql_character_set_name(m_Data);
 }
 /* 主要功能:得到错误代码 */
 const unsigned int CDataBase::Get_errno()
 {
	 return mysql_errno(m_Data);
 }
 /* 主要功能:得到错误信息 */
 const char * CDataBase::Get_error()
 {
	 return mysql_error(m_Data);
 }
 /*
  * 主要功能返回单值查询
 */
 char * CDataBase::ExecQueryGetSingValue(string sql)
 {
	 MYSQL_RES * res;
	 MYSQL_ROW row ;
	 char *p = NULL;
	 if(!mysql_real_query( m_Data, sql.c_str(),(unsigned long)sql.length()))
	 {
		 //保存查询结果
		 res = mysql_store_result( m_Data ) ;
		 row = mysql_fetch_row( res ) ;
		 p = const_cast<char*>(((row[0]==NULL)||(!strlen(row[0])))?"-1":row[0]);
		 mysql_free_result( res );
	 }
	 else
	 {
		 //执行查询失败
		 string strP = "-1";
		 p = const_cast<char*>(strP.c_str());
	 }
	 return p;
 }
 /*
  * 得到系统时间
 */
 const char * CDataBase::GetSysTime()
 {
	 return ExecQueryGetSingValue("select now()");
 }
 /*
  *  主要功能:建立新数据库
  * 参数:name 为新数据库的名称
  * 返回:0成功 -1 失败
 */
 int CDataBase::Create_db(string name)
 {
	 string temp ;
	 temp = "CREATE CDataBase ";
	 temp += name;
	 if(!mysql_real_query( m_Data, temp.c_str () ,
			 (unsigned long)temp.length ()) )
		 return 0;
	 else
		 //执行查询失败
		 return -1;
 }
 /*
  * 主要功能:删除制定的数据库
  * 参数:name 为欲删除数据库的名称
  * 返回:0成功 -1 失败
 */
 int CDataBase::Drop_db(string name)
 {
	 string temp ;
	 temp = "DROP CDataBase ";
	 temp += name;
	 if(!mysql_real_query( m_Data, temp.c_str () ,
			 (unsigned long)temp.length ()) )
		 return 0;
	 else
		 //执行查询失败
		 return -1;
 }

