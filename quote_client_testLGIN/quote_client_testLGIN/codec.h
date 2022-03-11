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

#include <sstream> 
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <iostream>
#include <memory>

const muduo::string lginBody("FL@G4AES8LJF3R3RT5POE9KFA@HFASF$;NASF(*&J;JFEASFJKJ;FEAF^FEAFHLL");
const std::string deli("@%&)");
const int deliLen = 4;
#pragma pack (1)
typedef struct
{
	char cmd[8];
	char format[4];
	char data[0];
}Header;
typedef struct {
	unsigned char cmd; // 网关用来区分该数据包是需要转发给客户端还是自己处理
	uint32_t   reqid; // 请求id
	uint32_t   size; // 包体长度
	unsigned char data[0];// 客户端的原始请求数据. protocl+detailType+data
}
front_packet_t;
//实时行情的数据结构。 发送的时候就填上交易所、品种和合约代码,前置网关收到以后就不用再解析行情的全文来获取这些信息
typedef struct {
	char exchange[8]; //交易所
	char comodity[4]; //品种
	char contractNo[8]; //合约
	char data[0];  //行情全文
}
hq_packet_t;
#pragma pack ()
const int headerLen = sizeof(Header);
const int frontLen = sizeof(front_packet_t);
const int packetLen = sizeof(hq_packet_t);
typedef std::function<void(const muduo::net::TcpConnectionPtr&,
	const muduo::string& message,
	muduo::Timestamp)> StringMessageCallback;

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
		const muduo::StringPiece&,const muduo::StringPiece&,const muduo::StringPiece&);
private:
	StringMessageCallback messageCallback_;
};

#endif  // MUDUO_EXAMPLES_SIMPLE_ECHO_ECHO_H