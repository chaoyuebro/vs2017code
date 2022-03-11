#include "./codec.h"


//�ְ�
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	while (buf->readableBytes() >= deliLen)
	{
		const char* data = buf->peek();//͵�����ݣ�û�н�����ȡ��
		muduo::string strData(data, buf->readableBytes());
		int pos = static_cast<int>(strData.find(deli));
		if (pos >= 0)
		{
			muduo::string msg(buf->peek(), pos);
			buf->retrieve(pos + deliLen);
			messageCallback_(conn, msg, receiveTime);
		}
		else 
		{
			break;
		}
	}
}

//�ϳɰ����͡�
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& cmd, const muduo::StringPiece& format,const muduo::StringPiece& userName,const muduo::StringPiece& body)
{
	muduo::net::Buffer buf;
	Header head;
	strcpy(head.cmd, cmd.data());
	strcpy(head.format, format.data());
	strcpy(head.userName, userName.data());
	buf.append(&head, headerLen);
	buf.append(body.data(), body.size());
	muduo::StringPiece strPiece(deli);//�ָ���
	buf.append(strPiece.data(), strPiece.size());
	conn->send(&buf);
}
