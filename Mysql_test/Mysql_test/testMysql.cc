//#include "jdbc/mysql_connection.h"
//#include "jdbc/mysql_driver.h"
//#include "jdbc/cppconn/statement.h"
//#include <iostream>
//using namespace sql;
//using namespace std;
//void RunConnectMySQL()
//{
//	mysql::MySQL_Driver *driver;
//	Connection *con;
//	Statement *state;
//	ResultSet *result;
//	// 初始化驱动  
//	driver = sql::mysql::get_mysql_driver_instance();
//	// 建立链接  
//	con = driver->connect("tcp://127.0.0.1:3306", "root", "chinese86");
//	state = con->createStatement();
//	state->execute("use test");
//	// 查询  
//	result = state->executeQuery("select * from testuser where id < 1002");
//	// 输出查询  
//	while (result->next())
//	{
//		int id = result->getInt("ID");
//		string name = result->getString("name");
//		cout << id << " : " << name << endl;
//	}
//	delete state;
//	delete con;
//}
//int main(int argc, char* argv[])
//{
//	RunConnectMySQL();
//	getchar();
//	return 0;
//}
#include <mysql/jdbc.h>
#pragma comment(lib, "mysqlcppconn.lib")

#include<iostream>
using namespace std;

int main()
{
	//连接字符串
	const char* host = "tcp://192.168.0.199:3306/test";

	const char* user = "root";//用户名
	const char* passwd = "root";//密码

	try
	{
		//获取驱动实例
		sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();

		//connect连接数据库，返回Connection对象指针
		sql::Connection* conn = driver->connect(host, user, passwd);

		if (conn != nullptr && conn->isClosed() == 0) {
			cout << "数据库连接成功！" << endl;
		}

		//设置客户端地连接编码格式为GBK
		sql::Statement *stmt = conn->createStatement();
		stmt->execute("SET character set gbk");

		
			//插入数据
			if (true)
			{
				//创建PreparedStatement准备sql语句
				const sql::SQLString  sql = "INSERT  INTO Student VALUES(?,?,?);";
				sql::PreparedStatement  * pstmt = conn->prepareStatement(sql);

				//填充？参数
				pstmt->setInt(1, 1000);
				pstmt->setString(2, "学习");
				pstmt->setInt(3, 22);

				//执行sql语句
				int ret = pstmt->executeUpdate(); // executeUpdate适用于UPDATE  INSERT  DELETE操作
				if (ret == 1) {
					cout << "插入数据成功！" << endl;
				}
				//关闭语句
				pstmt->close();

			}
		
			//查询数据
			if (true)
			{
				const sql::SQLString  sql = "SELECT  *  FROM   Student ;";
				sql::ResultSet * rs = stmt->executeQuery(sql);// executeQuery适用于 SELECT

				while (rs != nullptr && rs->next()) {

					int id = rs->getInt("id");//按 列名获取数据
					sql::SQLString  name = rs->getString(2);//按序号方式获取列数据
					int age = rs->getInt(3);

					cout << id << "\t" << name.c_str() << "\t" << age << endl;
				}

				//关闭结果集
				rs->close();
			}
		
			//修改数据
			if (true)
			{
				const sql::SQLString  sql = "UPDATE  Student  SET  name='天天向上'  WHERE id =1000;";
				int  effectRows = stmt->executeUpdate(sql);// executeUpdate适用于 UPDATE INSERT  DELETE
				cout << "修改数据成功" << effectRows << "行受影响！" << endl;
			}

		//
		//删除数据
		if (true)
		{
			/*写法一
			const sql::SQLString  sql = "DELETE  FROM Student WHERE id =1000;";
			int  effectRows = stmt->executeUpdate(sql);// executeUpdate适用于 UPDATE INSERT  DELETE
			cout << "删除数据成功" << effectRows << "行受影响！" << endl;
			*/


			const sql::SQLString  sql = "DELETE  FROM Student WHERE id =1000;";
			// const sql::SQLString  sql = "SELECT  * FROM Student WHERE id =1000;";
			bool  ret = stmt->execute(sql);// execute适用于 UPDATE INSERT  DELETE  SELECT
			if (ret)
			{//代表是SELECT 操作，可以获取结果集
				sql::ResultSet * rs = stmt->getResultSet();
				cout << "查询操作，返回了数据集" << rs << endl;
			}
			else
			{//代表是UPDATE INSERT  DELETE操作，可以获取受影响的行数
				int  effectRows = stmt->getUpdateCount();
				cout << "删除数据成功" << effectRows << "行受影响！" << endl;
			}

		}
		//


		//关闭语句
		stmt->close();

		//关闭数据库连接
		conn->close();
		cout << "关闭数据库连接成功！" << endl;

		delete conn;
	}
	catch (sql::SQLException& e) {
		std::cout << "错误: " << e.getErrorCode() << "  " << e.what();
	}
}
