#ifndef __SERVER_H__
#define __SERVER_H__
#include "./codec.h"
#include "./mysqlJsonTrans.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/ThreadLocalSingleton.h"
#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <set>
using namespace muduo;
using namespace muduo::net;
typedef std::set<TcpConnectionPtr> ConnectionList;//保存所有连接。


class comServer : noncopyable
{
public:
	comServer(EventLoop* loop,
		const InetAddress& listenAddr,int threadNums);

	void start();
	void sendMessage(TcpConnectionPtr&, const string&);
	void initThreadPool(int,int);
private:
	void onConnection(const TcpConnectionPtr& conn);
	void onStringMessage(const TcpConnectionPtr&,
		const string& message,
		Timestamp);
	
	TcpServer server_;
	LengthHeaderCodec codec_;
	ConnectionList connections_;
	MutexLock mutex_;
	ThreadPool threadPool_;
	MysqlTrans mysqlTrans_;
};
#endif