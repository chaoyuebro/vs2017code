#pragma once

#include <json/json.h> //jsoncpp��ͷ�ļ�
#include <mysql/mysql.h>
#include <memory>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ctime>
#include <iostream>    //��дio c++��׼��
#include <fstream>     //��д�ļ� c++��׼��
#include <string>      //�ַ����� c++��׼��
#include <sstream>     //�ַ����� c++��׼��

#define TABLE_NAME "jsonServer"
using namespace std;

class MysqlConnection
{
public:
	MysqlConnection();
	~MysqlConnection();
	void mysqlInsert(const string&, const string&);
	void mysqlDelete(const string&);
	void mysqlQuery(const string&, string&);
	void setUtf8(char *);//����Ϊutf8��ʽ����
	bool connect(string ip,
		string user,
		string password,
		string dbname);
	void refreshAliveTime() { _alivetime = clock(); }
	void mysqlClose();
	bool mysqlPing();
	// ���ش���ʱ��
	clock_t getAliveeTime()const { return clock() - _alivetime; }
private:
	MYSQL *_conn; // ��ʾ��MySQL Server��һ������
	clock_t _alivetime; // ��¼�������״̬�����ʼ���ʱ��
};