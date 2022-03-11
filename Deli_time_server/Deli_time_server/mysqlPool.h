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
	static MysqlPool* getMysqlPoolObject();		 //单列模式获取本类的对象
	shared_ptr<MysqlConnection> getOneConnect(); 			//获取一个连接对象
	//设置数据库参数
	~MysqlPool();
private:
	MysqlPool();
	bool loadConfigFile();

	bool isEmpty();				  //连接池队列池是否为空
	MysqlConnection *poolFront(); //连接池队列的队头
	unsigned int poolSize();	  //获取连接池的大小
	void poolPop();				  //弹出队头
	// 运行在独立的线程中，专门负责生产新连接
	void produceConnectionTask();
	// 扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	void scannerConnectionTask();

private:
	std::queue<MysqlConnection *> _connQueue; //连接池队列
	string _mysqlhost;						  // mysql主机地址
	string _mysqluser;						  // mysql用户名
	string _mysqlpwd;						  // mysql密码
	string _databasename;					  //要使用的mysql数据库名字
	int _initCount;				  //初始化的mysql连接数量
	int _maxCount;					  //同时允许最大连接对象数量
	atomic_int _currentCount;				  //目前连接池的连接对象数量

	int _maxIdleTime;		// 连接池最大空闲时间
	int _connectionTimeout; // 连接池获取连接的超时时间

	static std::mutex _queueLock;		//对象锁
	static std::mutex _poolLock;		//连接池锁
	static shared_ptr<MysqlPool> mysqlpool_object; //类的对象
	condition_variable cv;
};
