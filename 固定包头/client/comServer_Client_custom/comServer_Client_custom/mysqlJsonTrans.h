#ifndef __MYSQLJSONTRANS_H__
#define __MYSQLJSONTRANS_H__
#include <iostream>    //��дio c++��׼��
#include <fstream>     //��д�ļ� c++��׼��
#include <string>      //�ַ����� c++��׼��
#include <sstream>     //�ַ����� c++��׼��
#include <json/json.h> //jsoncpp��ͷ�ļ�
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
