#ifndef __MYSQLPOOL_H_
#define __MYSQLPOOL_H_
#include"mysqlJsonTrans.h"

#include<mysql/mysql.h>
#include<atomic>

#include<iostream>
#include<queue>
#include<map>
#include<vector>
#include<utility>
#include<string>
#include<mutex>
#include<thread>



class MysqlPool {
public:

	static MysqlPool* getMysqlPoolObject(const char*,
		const char*, const char*, const char*, unsigned int,
		unsigned int);              //����ģʽ��ȡ����Ķ���
	void doPut(const string&, const string&);
	void doGet(const string&, string&);
	void setParameter(const char*,
		const char*,
		const char*,
		const char*,
		unsigned int);              //�������ݿ����
	static void destroy()
	{
		if (mysqlpool_object != NULL)
		{
			objectlock.lock();
			if (mysqlpool_object != NULL)
			{
				delete mysqlpool_object;
				mysqlpool_object = NULL;
			}
		}
		objectlock.unlock();
	}
private:
	MysqlPool(const char*,
		const char*,
		const char*,
		const char*,
		unsigned int,
		unsigned int);
	~MysqlPool();

	MYSQL* createOneConnect();                    //����һ���µ����Ӷ���
	MYSQL* getOneConnect();                       //��ȡһ�����Ӷ���
	void close(MYSQL* conn);                      //�ر����Ӷ���
	bool isEmpty();                               //���ӳض��г��Ƿ�Ϊ��
	MYSQL* poolFront();                           //���ӳض��еĶ�ͷ
	unsigned int poolSize();                      //��ȡ���ӳصĴ�С
	void poolPop();                               //�������ӳض��еĶ�ͷ
private:
	std::queue<MYSQL*> mysqlpool;                 //���ӳض���
	const char*   _mysqlhost;                     //mysql������ַ
	const char*   _mysqluser;                     //mysql�û���
	const char*   _mysqlpwd;                      //mysql����
	const char*   _databasename;                  //Ҫʹ�õ�mysql���ݿ�����
	unsigned int  _initCount;					  //��ʼ����mysql��������	
	unsigned int  _maxCount;                      //ͬʱ����������Ӷ�������
	atomic_int  _currentCount;                  //Ŀǰ���ӳص����Ӷ�������
	static std::mutex objectlock;                 //������
	static std::mutex poollock;                   //���ӳ���
	static MysqlPool* mysqlpool_object;           //��Ķ���
	MysqlTrans _mysqlTrans;
};
#endif
