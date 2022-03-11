#include "./codec.h"

using namespace std;

//分包
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	while (buf->readableBytes() >= frontLen) // kHeaderLen == 4
	{
		const void* data = buf->peek();//偷看数据，没有将数据取走
		front_packet_t* pHeader = (front_packet_t*)(data);
		const int32_t len = muduo::net::sockets::networkToHost32(pHeader->size);
		const int32_t reqid = muduo::net::sockets::networkToHost32(pHeader->reqid);
		unsigned char cmd = pHeader->cmd;
		if (len > 65536 || len < 0)
		{
			LOG_ERROR << "Invalid length " << len;
			conn->shutdown();  // FIXME: disable reading
			break;
		}
		else if (buf->readableBytes() >= len+frontLen)
		{
			buf->retrieve(frontLen);
			muduo::string msg(buf->peek(), len);
			buf->retrieve(len);
			messageCallback_(conn, msg, receiveTime);
		}
		else
		{
			break;
		}
	}
}
		

//合成包发送。
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& cmd, const muduo::StringPiece &format, const muduo::StringPiece& body)
{
	muduo::net::Buffer buf;
	if (cmd == "LGIN")
	{
		Header head = { '0' };
		strcpy(head.cmd, cmd.data());
		strcpy(head.format, format.data());
		buf.append(&head, headerLen);
		buf.append(body.data(), body.size());
		muduo::StringPiece strPiece(deli);//分隔符
		buf.append(strPiece.data(), strPiece.size());
	}
	conn->send(&buf);
}