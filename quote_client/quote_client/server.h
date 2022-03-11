#ifndef __SERVER_H__
#define __SERVER_H__

#include "codec.h"
#include "utils.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

#include <set>
#include <stdio.h>
#include <unistd.h>
#include <thread>

using namespace muduo;
using namespace muduo::net;

class QuoteServer : noncopyable
{
public:
	QuoteServer(EventLoop* loop,
		const InetAddress& listenAddr)
		: server_(loop, listenAddr, "QuoteServer"),
		codec_(std::bind(&QuoteServer::onStringMessage, this, _1, _2, _3))
	{
		server_.setConnectionCallback(
			std::bind(&QuoteServer::onConnection, this, _1));
		server_.setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		std::thread loopThread(std::bind(&QuoteServer::loopSend,this));
		loopThread.detach();
	}

	void start();
	void loopSend();
private:
	void onConnection(const TcpConnectionPtr& conn);
	void onStringMessage(const TcpConnectionPtr&,
		const string& message,
		Timestamp);
	typedef std::set<TcpConnectionPtr> ConnectionList;
	TcpServer server_;
	LengthHeaderCodec codec_;
	ConnectionList connections_;
	ConnectionList checkConns_;
	MutexLock mutex_;
	MutexLock checkMutex_;
};

#endif

