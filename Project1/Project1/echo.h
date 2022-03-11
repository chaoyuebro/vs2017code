#include "muduo/net/TcpServer.h"
#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include <iostream>
#include <functional>

class EchoServer
{
public:
	EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr);
	void start();
private:
	void onConnection(const muduo::net::TcpConnectionPtr &conn);
	void onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
		muduo::Timestamp time);
private:
	muduo::net::EventLoop *_loop;
	muduo::net::TcpServer _server;
};