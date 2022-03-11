#include "./echo.h"
#include <iostream>


using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

// using namespace muduo;
// using namespace muduo::net;

EchoServer::EchoServer(muduo::net::EventLoop* loop,
	const muduo::net::InetAddress& listenAddr)
	: server_(loop, listenAddr, "EchoServer")
{
	server_.setConnectionCallback(
		std::bind(&EchoServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&EchoServer::onMessage, this, _1, _2, _3));
}

void EchoServer::start()
{
	server_.start();
}

void EchoServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << "EchoServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
}

void EchoServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp time)
{
	muduo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
		<< "data received at " << time.toString();
	//vecStr_.push_back(msg);
	conn->send(msg);
	std::cout << msg << std::endl;
	//while (true)
	//{
	//	//buf������Ұ��ķָ���  ������
	//	//�ָ��һ�����������ύ���̳߳�
	//	break;
	//}
}
void EchoServer::sendString(muduo::string str)
{
	std::cout << str << std::endl;
}
//void EchoServer::ThreadPool(int maxSize)
//{
//	LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
//	muduo::ThreadPool pool("MainThreadPool");
//	pool.setMaxQueueSize(maxSize); //����������еĴ�С
//	pool.start(5); //����5���̣߳��������߳�
//	muduo::string getMsg;
//	LOG_WARN << "Adding";
//	//���̳߳�������Զ�����޲ε�print���񣬲������̳߳��еĿ����߳�
//	pool.run(std::bind(&sendString,&getMsg));
//	LOG_WARN << "Done";
//
//	//���̳߳������CountDownLatch���еĳ�Ա����countDown()���������̳߳��еĿ����߳�
//	muduo::CountDownLatch latch(1);
//	pool.run(std::bind(&muduo::CountDownLatch::countDown, &latch));
//	latch.wait();
//
//	pool.stop();
//}

//void test(int maxSize)
//{
//
//}