#include "./codec.h"


//分包
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	//1. 一次有多个分隔符
	//2. 一次一个分隔符也没有
	//3.一次读完后还有剩下的
	while (buf->readableBytes() >= deliLen)//如果可读数据大于分隔符长度。
	{
		const char* data = buf->peek();//偷看数据，没有将数据取走
		std::string str(data);
		int pos = static_cast<int>(str.find(deli));
		if (pos == -1)
		{
			break;
		}
		else if (pos == 0)
		{
			buf->retrieve(deliLen);
		}
		else if (pos > 0)
		{
			muduo::string message(buf->peek(), pos);
			messageCallback_(conn, message, receiveTime);
			buf->retrieve(pos);
		}
	}

}

//合成包发送。
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& message)
{
	muduo::net::Buffer buf;
	buf.append(message.data(), message.size());
	muduo::StringPiece strPiece(deli);
	buf.append(strPiece.data(),strPiece.size());
	conn->send(&buf);
}
