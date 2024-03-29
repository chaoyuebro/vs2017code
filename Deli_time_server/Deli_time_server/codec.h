#ifndef __CODEC_H__
#define __CODEC_H__
#include "./ThreadPool.h"
#include "muduo/net/Endian.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/CurrentThread.h"
#include "muduo/base/Logging.h"

#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <iostream>
#include <gzip.h>
#include <string.h>

#include <memory>
#include <string>

using std::cout;
using std::endl;

#pragma pack (1)
typedef struct
{
	char cmd[8];
	char format[4];
	char userName[32];
}Header;
#pragma pack ()
const std::string deli("@%&)");
const int deliLen = 4;
const int headerLen = sizeof(Header);
typedef std::function<void(const muduo::net::TcpConnectionPtr&,
	const muduo::string&,muduo::Timestamp)> StringMessageCallback;




class LengthHeaderCodec
	: muduo::noncopyable
{
public:
	explicit LengthHeaderCodec(const StringMessageCallback& cb)
		: messageCallback_(cb)
	{
	}

	void onMessage(const muduo::net::TcpConnectionPtr& conn,
		muduo::net::Buffer* buf,
		muduo::Timestamp receiveTime);
	void send(muduo::net::TcpConnection*,
		const muduo::StringPiece&,const muduo::StringPiece&,const muduo::StringPiece&,const muduo::StringPiece&);
private:
	StringMessageCallback messageCallback_;
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H