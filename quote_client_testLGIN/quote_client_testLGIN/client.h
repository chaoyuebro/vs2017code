#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "codec.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <mutex>
using namespace muduo;
using namespace muduo::net;

class quoteClient : noncopyable
{
public:
	quoteClient(EventLoop* loop, const InetAddress& serverAddr)
		: client_(loop, serverAddr, "quoteClient"),
		codec_(std::bind(&quoteClient::onStringMessage, this, _1, _2, _3))
	{
		client_.setConnectionCallback(
			std::bind(&quoteClient::onConnection, this, _1));
		client_.setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		client_.enableRetry();
	}
	void connect();
	void disconnect();
	void write(const StringPiece&, const StringPiece&, const StringPiece&);
private:
	void onConnection(const TcpConnectionPtr&);
	void onStringMessage(const TcpConnectionPtr&,
		const string&,
		Timestamp);
private:
	TcpClient client_;
	MutexLock mutex_;
	LengthHeaderCodec codec_;
	TcpConnectionPtr connection_ GUARDED_BY(mutex_);
};

#endif