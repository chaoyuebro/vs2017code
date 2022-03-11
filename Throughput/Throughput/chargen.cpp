#include "chargen.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"

#include <stdio.h>

using namespace muduo;
using namespace muduo::net;

ChargenServer::ChargenServer(EventLoop* loop,
	const InetAddress& listenAddr,
	bool print)
	: server_(loop, listenAddr, "ChargenServer"),
	transferred_(0),
	startTime_(Timestamp::now())
{
	server_.setConnectionCallback(
		std::bind(&ChargenServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&ChargenServer::onMessage, this, _1, _2, _3));
	server_.setWriteCompleteCallback(
		std::bind(&ChargenServer::onWriteComplete, this, _1));

	//�������ݣ�ÿ��72���ַ����ܹ�94��
	string line;
	for (int i = 33; i < 127; ++i)
	{
		line.push_back(char(i)); //line: 33,34,35,... ...,126
	}
	line += line; //line: 33,34,35,... ...,126,33,34,35,... ...,126

	for (size_t i = 0; i < 127 - 33; ++i)
	{
		//line.substr(i, 72)����ʾsubstr����ַ���line�У��ӵ�iλ��ʼ�ĳ���Ϊ72���ַ���	
		message_ += line.substr(i, 72) + '\n';
	}
	/*
	message_:����string
	  33,34,35,... ...,126,\n
	  34,35,36,... ...,33,\n
	  35,36,37,... ...,34,\n
	  ...
	  ...
	  126,33,34,... ...,125,\n
	  */

	if (print) //ÿ��3s����printThroughput������ӡ������
	{
		loop->runEvery(3.0, std::bind(&ChargenServer::printThroughput, this));
	}
}

void ChargenServer::start()
{
	server_.start();
}

void ChargenServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << "ChargenServer - " << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");
	if (conn->connected())
	{
		conn->setTcpNoDelay(true);
		conn->send(message_);
	}
}

//����message_���ݷ�����Ϻ󣬻�ص�onWriteComplete����
void ChargenServer::onWriteComplete(const TcpConnectionPtr& conn)
{
	transferred_ += message_.size();
	conn->send(message_); //��������send����message_
}

void ChargenServer::printThroughput() //���㲢��ӡ������
{
	Timestamp endTime = Timestamp::now();
	double time = timeDifference(endTime, startTime_);
	printf("%4.3f MiB/s\n", static_cast<double>(transferred_) / time / 1024 / 1024);
	transferred_ = 0;
	startTime_ = endTime;
}

void ChargenServer::onMessage(const TcpConnectionPtr& conn,
	Buffer* buf,
	Timestamp time)
{
	string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << " discards " << msg.size()
		<< " bytes received at " << time.toString();
}
