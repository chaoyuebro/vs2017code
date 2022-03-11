#include "./codec.h"


//�ְ�
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	while (buf->readableBytes() >= kHeaderLen) // kHeaderLen == 4
	{
		// FIXME: use Buffer::peekInt32()
		const void* data = buf->peek();//͵�����ݣ�û�н�����ȡ��
		int32_t be32 = *static_cast<const int32_t*>(data); // SIGBUS
		const int32_t len = muduo::net::sockets::networkToHost32(be32);
		if (len > 65536 || len < 0)
		{
			LOG_ERROR << "Invalid length " << len;
			conn->shutdown();  // FIXME: disable reading
			break;
		}
		else if (buf->readableBytes() >= len + kHeaderLen)
		{
			buf->retrieve(kHeaderLen);
			muduo::string message(buf->peek(), len);
			messageCallback_(conn, message, receiveTime);
			buf->retrieve(len);
		}
		else
		{
			break;
		}
	}
}

//�ϳɰ����͡�
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& message)
{
	muduo::net::Buffer buf;
	buf.append(message.data(), message.size());
	int32_t len = static_cast<int32_t>(message.size());
	int32_t be32 = muduo::net::sockets::hostToNetwork32(len);
	buf.prepend(&be32, sizeof(be32));
	conn->send(&buf);
}