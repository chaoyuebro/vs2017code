#include "./codec.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"

#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatClient : noncopyable
{
 public:
  ChatClient(EventLoop* loop, const InetAddress& serverAddr)
    : client_(loop, serverAddr, "ChatClient"),
      codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
  {
    client_.setConnectionCallback(
        std::bind(&ChatClient::onConnection, this, _1));
    client_.setMessageCallback(
        std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
    client_.enableRetry();
  }

  void connect()
  {
    client_.connect();
  }

  void disconnect()
  {
    client_.disconnect();
  }

  void write(const StringPiece& message)
  {
    MutexLockGuard lock(mutex_);
    if (connection_)
    {
      codec_.send(get_pointer(connection_), message);//get_pointer返回智能指针的原指针。
    }
  }//发送数据

 private:
  void onConnection(const TcpConnectionPtr& conn)
  {
    LOG_INFO << conn->localAddress().toIpPort() << " -> "
             << conn->peerAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");

    MutexLockGuard lock(mutex_);
    if (conn->connected())
    {
      connection_ = conn;
    }
    else
    {
      connection_.reset();
    }
  }

  void onStringMessage(const TcpConnectionPtr&,
                       const string& message,
                       Timestamp)
  {
    printf("<<< %s\n", message.c_str());
  }

  TcpClient client_;
  LengthHeaderCodec codec_;
  MutexLock mutex_;
  TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};



int main(int argc, char* argv[])
{
  LOG_INFO << "pid = " << getpid();
  if (argc > 2)
  {
    EventLoopThread loopThread;
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress serverAddr(argv[1], port);

    ChatClient client(loopThread.startLoop(), serverAddr);
    client.connect();
    std::string line;
    while (std::getline(std::cin, line))
    {
      client.write(line);
    }
    client.disconnect();
    CurrentThread::sleepUsec(1000*1000);  // wait for disconnect, see ace/logging/client.cc
  }
  else
  {
    printf("Usage: %s host_ip port\n", argv[0]);
  }
}

//#include <muduo/net/Channel.h>
//#include <muduo/net/TcpClient.h>
//
//#include <muduo/base/Logging.h>
//#include <muduo/net/EventLoop.h>
//#include <muduo/net/InetAddress.h>
//#include <functional>
//#include <stdio.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//class TestClient
//{
//public:
//	TestClient(EventLoop* loop, const InetAddress& listenAddr)
//		: loop_(loop),
//		client_(loop, listenAddr, "TestClient"),
//		stdinChannel_(loop, 0)
//	{
//		client_.setConnectionCallback(
//			std::bind(&TestClient::onConnection, this, _1));
//		client_.setMessageCallback(
//			std::bind(&TestClient::onMessage, this, _1, _2, _3));
//		//client_.enableRetry();
//		// 标准输入缓冲区中有数据的时候，回调TestClient::handleRead
//		stdinChannel_.setReadCallback(std::bind(&TestClient::handleRead, this));
//		stdinChannel_.enableReading();		// 关注可读事件
//	}
//
//	void connect()
//	{
//		client_.connect();
//	}
//
//private:
//	void onConnection(const TcpConnectionPtr& conn)
//	{
//		if (conn->connected())
//		{
//			printf("onConnection(): new connection [%s] from %s\n",
//				conn->name().c_str(),
//				conn->peerAddress().toIpPort().c_str());
//		}
//		else
//		{
//			printf("onConnection(): connection [%s] is down\n",
//				conn->name().c_str());
//		}
//	}
//
//	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
//	{
//		string msg(buf->retrieveAllAsString());
//		printf("onMessage(): recv a message [%s]\n", msg.c_str());
//		LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at " << time.toFormattedString();
//	}
//
//	// 标准输入缓冲区中有数据的时候，回调该函数
//	void handleRead()
//	{
//		char buf[1024] = { 0 };
//		fgets(buf, 1024, stdin);
//		buf[strlen(buf) - 1] = '\0';		// 去除\n
//		client_.connection()->send(buf);
//	}
//
//	EventLoop* loop_;
//	TcpClient client_;
//	Channel stdinChannel_;		// 标准输入Channel
//};
//
//int main(int argc, char* argv[])
//{
//	LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
//	EventLoop loop;
//	InetAddress serverAddr("localhost", 2007);
//	TestClient client(&loop, serverAddr);
//	client.connect();
//	loop.loop();
//}
//#include "muduo/base/Logging.h"
//#include "muduo/net/EventLoop.h"
//#include "muduo/net/InetAddress.h"
//#include "muduo/net/TcpClient.h"
//
//#include <utility>
//
//#include <stdio.h>
//#include <unistd.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//class ChargenClient : noncopyable
//{
//public:
//	ChargenClient(EventLoop* loop, const InetAddress& listenAddr)
//		: loop_(loop),
//		client_(loop, listenAddr, "ChargenClient")
//	{
//		client_.setConnectionCallback(
//			std::bind(&ChargenClient::onConnection, this, _1));
//		client_.setMessageCallback(
//			std::bind(&ChargenClient::onMessage, this, _1, _2, _3));
//		// client_.enableRetry();
//	}
//
//	void connect()
//	{
//		client_.connect();
//	}
//
//private:
//	void onConnection(const TcpConnectionPtr& conn)
//	{
//		LOG_INFO << conn->localAddress().toIpPort() << " -> "
//			<< conn->peerAddress().toIpPort() << " is "
//			<< (conn->connected() ? "UP" : "DOWN");
//
//		if (!conn->connected())
//			loop_->quit();
//	}
//
//	void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime)
//	{
//		buf->retrieveAll();
//	}
//
//	EventLoop* loop_;
//	TcpClient client_;
//};
//
//int main(int argc, char* argv[])
//{
//	LOG_INFO << "pid = " << getpid();
//	if (argc > 1)
//	{
//		EventLoop loop;
//		InetAddress serverAddr(8888);
//
//		ChargenClient chargenClient(&loop, serverAddr);
//		chargenClient.connect();
//		loop.loop();
//	}
//	else
//	{
//		printf("Usage: %s host_ip\n", argv[0]);
//	}
//}
