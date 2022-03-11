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
		unsigned int);              //单列模式获取本类的对象
	void doPut(const string&, const string&);
	void doGet(const string&, string&);
	void setParameter(const char*,
		const char*,
		const char*,
		const char*,
		unsigned int);              //设置数据库参数
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

	MYSQL* createOneConnect();                    //创建一个新的连接对象
	MYSQL* getOneConnect();                       //获取一个连接对象
	void close(MYSQL* conn);                      //关闭连接对象
	bool isEmpty();                               //连接池队列池是否为空
	MYSQL* poolFront();                           //连接池队列的队头
	unsigned int poolSize();                      //获取连接池的大小
	void poolPop();                               //弹出连接池队列的队头
private:
	std::queue<MYSQL*> mysqlpool;                 //连接池队列
	const char*   _mysqlhost;                     //mysql主机地址
	const char*   _mysqluser;                     //mysql用户名
	const char*   _mysqlpwd;                      //mysql密码
	const char*   _databasename;                  //要使用的mysql数据库名字
	unsigned int  _initCount;					  //初始化的mysql连接数量	
	unsigned int  _maxCount;                      //同时允许最大连接对象数量
	atomic_int  _currentCount;                  //目前连接池的连接对象数量
	static std::mutex objectlock;                 //对象锁
	static std::mutex poollock;                   //连接池锁
	static MysqlPool* mysqlpool_object;           //类的对象
	MysqlTrans _mysqlTrans;
};
#endif
