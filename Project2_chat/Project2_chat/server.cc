#include "./codec.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

#include <set>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
//
using namespace muduo;
using namespace muduo::net;
using namespace std;
//
//class ChatServer : noncopyable
//{
// public:
//  ChatServer(EventLoop* loop,
//             const InetAddress& listenAddr)
//  : server_(loop, listenAddr, "ChatServer"),
//    codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
//  {
//    server_.setConnectionCallback(
//        std::bind(&ChatServer::onConnection, this, _1));
//    server_.setMessageCallback(
//        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
//  }
//
//  void start()
//  {
//    server_.start();
//  }
//
// private:
//  void onConnection(const TcpConnectionPtr& conn)
//  {
//    LOG_INFO << conn->peerAddress().toIpPort() << " -> "
//             << conn->localAddress().toIpPort() << " is "
//             << (conn->connected() ? "UP" : "DOWN");
//
//    if (conn->connected())
//    {
//      connections_.insert(conn);
//    }
//    else
//    {
//      connections_.erase(conn);
//    }
//  }
//
//  void onStringMessage(const TcpConnectionPtr&,
//                       const string& message,
//                       Timestamp)
//  {
//    for (ConnectionList::iterator it = connections_.begin();
//        it != connections_.end();
//        ++it)
//    {
//      codec_.send(get_pointer(*it), message);
//    }
//  }
//
//  typedef std::set<TcpConnectionPtr> ConnectionList;
//  TcpServer server_;
//  LengthHeaderCodec codec_;
//  ConnectionList connections_;
//};
//
//int main(int argc, char* argv[])
//{
//  LOG_INFO << "pid = " << getpid();
//  if (argc > 1)
//  {
//    EventLoop loop;
//    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
//    InetAddress serverAddr(port);
//    ChatServer server(&loop, serverAddr);
//    server.start();
//    loop.loop();
//  }
//  else
//  {
//    printf("Usage: %s port\n", argv[0]);
//  }
//}
//
//#include "codec.h"

//#include <muduo/base/Logging.h>
//#include <muduo/base/Mutex.h>
#include "muduo/base/ThreadLocalSingleton.h"
//#include <muduo/net/EventLoop.h>
//#include <muduo/net/TcpServer.h>
//
//#include <boost/bind.hpp>
//#include <boost/shared_ptr.hpp>
//
//#include <set>
//#include <stdio.h>

#include "./codec.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/base/ThreadLocalSingleton.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"

#include <set>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatServer : noncopyable
{
public:
	ChatServer(EventLoop* loop,
		const InetAddress& listenAddr)
		: server_(loop, listenAddr, "ChatServer"),
		codec_(std::bind(&ChatServer::onStringMessage, this, _1, _2, _3))
	{
		server_.setConnectionCallback(
			std::bind(&ChatServer::onConnection, this, _1));
		server_.setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	}

	void setThreadNum(int numThreads)
	{
		server_.setThreadNum(numThreads);
	}

	void start()
	{
		server_.setThreadInitCallback(std::bind(&ChatServer::threadInit, this, _1));//设置线程初始化的绑定函数
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		if (conn->connected())
		{
			LocalConnections::instance().insert(conn);
		}
		else
		{
			LocalConnections::instance().erase(conn);
		}
	}

	void onStringMessage(const TcpConnectionPtr&,
		const string& message,
		Timestamp)
	{
		EventLoop::Functor f = std::bind(&ChatServer::distributeMessage, this, message);//设置接收信息时需要调用的函数
		LOG_DEBUG;

		MutexLockGuard lock(mutex_);
		for (std::set<EventLoop*>::iterator it = loops_.begin();//由于每个IO线程都有各自的连接，所以调用每个IO线程的loop
			it != loops_.end();
			++it)
		{
			(*it)->queueInLoop(f);//加入运行队列里
		}
		LOG_DEBUG;
	}

	typedef std::set<TcpConnectionPtr> ConnectionList;

	void distributeMessage(const string& message)
	{
		LOG_DEBUG << "begin";
		for (ConnectionList::iterator it = LocalConnections::instance().begin();//这是线程安全的，只有各自的线程才会调用这个函数
			it != LocalConnections::instance().end();
			++it)
		{
			codec_.send(get_pointer(*it), message);
		}
		LOG_DEBUG << "end";
	}

	void threadInit(EventLoop* loop)
	{
		assert(LocalConnections::pointer() == NULL);
		LocalConnections::instance();//创建单例
		assert(LocalConnections::pointer() != NULL);
		MutexLockGuard lock(mutex_);
		loops_.insert(loop);//加入loops的集合里
	}

	TcpServer server_;
	LengthHeaderCodec codec_;
	typedef ThreadLocalSingleton<ConnectionList> LocalConnections;//线程单例模式

	MutexLock mutex_;
	std::set<EventLoop*> loops_ GUARDED_BY(mutex_);
};

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	if (argc > 1)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		InetAddress serverAddr(port);
		ChatServer server(&loop, serverAddr);
		if (argc > 2)
		{
			server.setThreadNum(atoi(argv[2]));
		}
		server.start();
		loop.loop();
	}
	else
	{
		printf("Usage: %s port [thread_num]\n", argv[0]);
	}
}

