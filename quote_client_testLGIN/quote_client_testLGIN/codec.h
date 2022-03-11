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
	unsigned char cmd; // �����������ָ����ݰ�����Ҫת�����ͻ��˻����Լ�����
	uint32_t   reqid; // ����id
	uint32_t   size; // ���峤��
	unsigned char data[0];// �ͻ��˵�ԭʼ��������. protocl+detailType+data
}
front_packet_t;
//ʵʱ��������ݽṹ�� ���͵�ʱ������Ͻ�������Ʒ�ֺͺ�Լ����,ǰ�������յ��Ժ�Ͳ����ٽ��������ȫ������ȡ��Щ��Ϣ
typedef struct {
	char exchange[8]; //������
	char comodity[4]; //Ʒ��
	char contractNo[8]; //��Լ
	char data[0];  //����ȫ��
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