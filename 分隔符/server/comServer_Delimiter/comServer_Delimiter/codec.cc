#include "./codec.h"


//�ְ�
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	//1. һ���ж���ָ���
	//2. һ��һ���ָ���Ҳû��
	//3.һ�ζ������ʣ�µ�
	while (buf->readableBytes() >= deliLen)//����ɶ����ݴ��ڷָ������ȡ�
	{
		const char* data = buf->peek();//͵�����ݣ�û�н�����ȡ��
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

//�ϳɰ����͡�
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& message)
{
	muduo::net::Buffer buf;
	buf.append(message.data(), message.size());
	muduo::StringPiece strPiece(deli);
	buf.append(strPiece.data(),strPiece.size());
	conn->send(&buf);
}
