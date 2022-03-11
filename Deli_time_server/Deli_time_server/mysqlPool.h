#pragma once
#include "mysqlConnection.h"

#include <mysql/mysql.h>

#include <atomic>
#include <iostream>
#include <queue>
#include <map>
#include <vector>
#include <utility>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>

#define LOG(str) \
	cout << __FILE__ << ":" << __LINE__ << " " << __TIMESTAMP__ << " : " << str << endl;
using std::shared_ptr;

class MysqlPool
{
public:
	static MysqlPool* getMysqlPoolObject();		 //����ģʽ��ȡ����Ķ���
	shared_ptr<MysqlConnection> getOneConnect(); 			//��ȡһ�����Ӷ���
	//�������ݿ����
	~MysqlPool();
private:
	MysqlPool();
	bool loadConfigFile();

	bool isEmpty();				  //���ӳض��г��Ƿ�Ϊ��
	MysqlConnection *poolFront(); //���ӳض��еĶ�ͷ
	unsigned int poolSize();	  //��ȡ���ӳصĴ�С
	void poolPop();				  //������ͷ
	// �����ڶ������߳��У�ר�Ÿ�������������
	void produceConnectionTask();
	// ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
	void scannerConnectionTask();

private:
	std::queue<MysqlConnection *> _connQueue; //���ӳض���
	string _mysqlhost;						  // mysql������ַ
	string _mysqluser;						  // mysql�û���
	string _mysqlpwd;						  // mysql����
	string _databasename;					  //Ҫʹ�õ�mysql���ݿ�����
	int _initCount;				  //��ʼ����mysql��������
	int _maxCount;					  //ͬʱ����������Ӷ�������
	atomic_int _currentCount;				  //Ŀǰ���ӳص����Ӷ�������

	int _maxIdleTime;		// ���ӳ�������ʱ��
	int _connectionTimeout; // ���ӳػ�ȡ���ӵĳ�ʱʱ��

	static std::mutex _queueLock;		//������
	static std::mutex _poolLock;		//���ӳ���
	static shared_ptr<MysqlPool> mysqlpool_object; //��Ķ���
	condition_variable cv;
};
