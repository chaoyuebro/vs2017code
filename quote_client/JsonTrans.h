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
#include <vector>
#include <map>
//#define MYSQL_HOST "localhost"
//#define MYSQL_USER "root"
//#define MYSQL_PASSWD "chinese86"
//#define DB_NAME "test_db"
//#define TABLE_NAME "clientJson"
using namespace std;

class JsonTrans
{
public:
	JsonTrans();
	void inputJson(string&);
	void coverJson(string&);
	void mergeJson(string&);
	void setLocalStr(string str);
	void setMysqlStr(string str);
private:
	Json::Value root_;
	Json::Reader reader_;
	string localStr_;//本地json字符串。
	string mysqlStr_;//从Mysql数据库返回的字符串。
	map<int, string> mergeMap_;
};
#endif
