#ifndef __MYSQLJSONTRANS_H__
#define __MYSQLJSONTRANS_H__
#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库
#include <json/json.h> //jsoncpp的头文件
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql/mysql.h>

#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASSWD "chinese86"
#define DB_NAME "test_db"
#define TABLE_NAME "clientJson"
using namespace std;

class MysqlTrans
{
public:
	MysqlTrans();
	~MysqlTrans();
	void mysqlInsert(int,string);
	void mysqlDelete(int);
	void mysqlUpdate(int, string);
	void mysqlQuery(int,string&);
	void inputJson(string&);
private:
	MYSQL* mysql_;
};
#endif
