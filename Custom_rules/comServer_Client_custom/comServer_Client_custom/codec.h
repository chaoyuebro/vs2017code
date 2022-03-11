#ifndef __CODEC_H__
#define __CODEC_H__

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
#include <memory>

//#pragma pack(1)
typedef struct
{
	int32_t bodyLen;
	char checksum;
	int32_t id;
	unsigned char body[0];
}Header;
//#pragma pack()

typedef std::function<void(const muduo::net::TcpConnectionPtr&,
	const muduo::string& message,
	muduo::Timestamp)> StringMessageCallback;
 // usleep
//消息的字节流定义成这种形式 0xXX 0xXX 0xXX 0xXX XXXXXX，前面4个字节表示消息的长度，后面是消息实体。
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
	void send(muduo::net::TcpConnection* conn,
		const muduo::StringPiece& message);
private:
	StringMessageCallback messageCallback_;
	const static size_t kHeaderLen = sizeof(Header);
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H