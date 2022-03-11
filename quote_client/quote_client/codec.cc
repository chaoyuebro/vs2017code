#include "./codec.h"

using namespace std;

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
	const muduo::StringPiece& cmd, const muduo::StringPiece &format, const muduo::StringPiece& body,const muduo::StringPiece& resStr)
{
	muduo::net::Buffer buf;
	if (cmd == "LGIN")
	{
		Header head = { '0' };
		strcpy(head.cmd, cmd.data());
		strcpy(head.format, format.data());
		buf.append(&head, headerLen);
		buf.append(body.data(), body.size());
		muduo::StringPiece strPiece(deli);//�ָ���
		buf.append(strPiece.data(), strPiece.size());
	}
	else {
		//���������ʱ������ȫ��ǰ�������QuoteID
		hq_packet_t hq_pkt;
		::memset(&hq_pkt, 0, sizeof(hq_packet_t));
		memcpy(hq_pkt.exchange, cmd.data(), cmd.size());
		memcpy(hq_pkt.comodity, format.data(), format.size());
		memcpy(hq_pkt.contractNo, body.data(),body.size());
		//ƴ�Ӱ�ͷ
		front_packet_t packet;
		packet.cmd = 6;
		packet.reqid = 0;
		packet.size = (uint32_t)htonl(sizeof(hq_packet_t) + resStr.size());
		buf.append(&packet, sizeof(packet));
		buf.append(&hq_pkt, sizeof(hq_packet_t));
		buf.append(resStr);
	}
	conn->send(&buf);
}