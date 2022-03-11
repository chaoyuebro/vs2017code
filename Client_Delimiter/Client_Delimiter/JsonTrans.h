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
	string localStr_;//����json�ַ�����
	string mysqlStr_;//��Mysql���ݿⷵ�ص��ַ�����
	map<int, string> mergeMap_;
};
#endif
