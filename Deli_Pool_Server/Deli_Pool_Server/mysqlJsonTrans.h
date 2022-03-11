#ifndef __MYSQLJSONTRANS_H__
#define __MYSQLJSONTRANS_H__

#include <json/json.h> //jsoncpp的头文件
#include <mysql/mysql.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库

#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASSWD "Ejia7.com"
#define DB_NAME "ejia7_three"
#define TABLE_NAME "jsonServer"
using namespace std;

class MysqlTrans
{
public:
	MysqlTrans();
	~MysqlTrans();
	void mysqlInsert(MYSQL *, const string&, const string&);
	void mysqlDelete(MYSQL *, const string&);
	//void increaseOne(MYSQL*);
	void mysqlQuery(MYSQL *, const string&, string&);
	void setUtf8(MYSQL*, char *);//设置为utf8格式传输
private:
	vector<string> strVec_;//储存查询到的sql语句。
};
#endif