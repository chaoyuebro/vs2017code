#pragma once

#include <json/json.h> //jsoncpp的头文件
#include <mysql/mysql.h>
#include <memory>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "muduo/base/Logging.h"


#include <ctime>
#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库
#include <unistd.h>
#define TABLE_NAME "jsonServer"

using namespace std;

class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();
	void mysqlInsert(const string&, const string&);
	void mysqlQuery(const string&, string&);
	bool connect(string ip,
		int port,
		string user,
		string password,
		string dbname);
	void refreshAliveTime() { _alivetime = clock(); }
	void mysqlClose();
	bool mysqlPing();
	// 返回存活的时间
	clock_t getAliveeTime()const { return clock() - _alivetime; }
private:
	MYSQL *_conn; // 表示和MySQL Server的一条连接
	clock_t _alivetime; // 记录进入空闲状态后的起始存活时间
};