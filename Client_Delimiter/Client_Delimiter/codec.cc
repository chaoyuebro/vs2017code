#include "./codec.h"
#include "gzip.h"
using namespace std;

//分包
void LengthHeaderCodec::onMessage(const muduo::net::TcpConnectionPtr& conn,
	muduo::net::Buffer* buf,
	muduo::Timestamp receiveTime)
{
	while (buf->readableBytes() >= deliLen)
	{
		const char* data = buf->peek();//偷看数据，没有将数据取走
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
		

//合成包发送。
void LengthHeaderCodec::send(muduo::net::TcpConnection* conn,
	const muduo::StringPiece& cmd, const muduo::StringPiece& format, const muduo::StringPiece& userName,
	const muduo::StringPiece& key, const muduo::StringPiece& side, const muduo::StringPiece& body)
{
	muduo::net::Buffer buf;
	Header head;
	memset(&head, '\0', headerLen);
	memcpy(head.cmd, cmd.data(),cmd.size());
	memcpy(head.format, format.data(),format.size());
	memcpy(head.userName, userName.data(),userName.size());
	memcpy(head.key, key.data(),key.size());
	memcpy(head.side, side.data(),side.size());
	buf.append(&head, headerLen);
	if (cmd == "RQUPJ")
	{
		int bodySize = body.size();
		cout << "bodySize= " << bodySize << endl;
		int destLen = bodySize * 2;
		char* compressed = (char *)malloc(destLen);
		memset(compressed, 0, destLen);
		const char *source = body.data();
		int gzSize = gzCompress(source, bodySize, compressed, destLen);
		char* uncompressed = (char *)malloc(1000);
		memset(uncompressed, 0, 1000);
		int ret = gzDecompress(compressed, gzSize, uncompressed, 1000);
		if (gzSize <= 0)
		{
			printf("compress error.\n");
			return;
		}
		cout << compressed << endl;
		buf.append(compressed, gzSize);
		free(compressed);
	}
	else if (cmd == "HBET")
	{
		buf.append(cmd.data(), cmd.size());
		buf.append(body.data(), body.size());
	}
	muduo::StringPiece strPiece(deli);//分隔符
	buf.append(strPiece.data(), strPiece.size());
	conn->send(&buf);

}