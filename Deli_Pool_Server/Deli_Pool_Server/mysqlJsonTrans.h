#ifndef __MYSQLJSONTRANS_H__
#define __MYSQLJSONTRANS_H__

#include <json/json.h> //jsoncpp��ͷ�ļ�
#include <mysql/mysql.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <iostream>    //��дio c++��׼��
#include <fstream>     //��д�ļ� c++��׼��
#include <string>      //�ַ����� c++��׼��
#include <sstream>     //�ַ����� c++��׼��

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
	void setUtf8(MYSQL*, char *);//����Ϊutf8��ʽ����
private:
	vector<string> strVec_;//�����ѯ����sql��䡣
};
#endif