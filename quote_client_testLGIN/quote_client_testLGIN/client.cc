#include "client.h"

void quoteClient::connect()
{
	client_.connect();
}

void quoteClient::disconnect()
{
	client_.disconnect();
}

void quoteClient::write(const StringPiece& cmd, const StringPiece& format,const StringPiece& body)
{
	MutexLockGuard lock(mutex_);
	if (connection_)
	{
		codec_.send(get_pointer(connection_), cmd,format, body);//get_pointer返回智能指针的原指针。
	}
}//发送数据


//void quoteClient::sendHeartBeat()
//{
//	while (1)
//	{
//		usleep(30000000);
//		write("HBET", "heart");
//	}
//}

void quoteClient::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

	MutexLockGuard lock(mutex_);
	if (conn->connected())
	{
		connection_ = conn;
		codec_.send(get_pointer(connection_), "LGIN", "TXT", lginBody);
	}
	else
	{
		connection_.reset();
	}
}

void quoteClient::onStringMessage(const TcpConnectionPtr&,
	const string& message,
	Timestamp)
{
	//将合约信息剥离出来并放入容器
	if (message.size() >= packetLen)
	{
		muduo::string cmdBuf(message.substr(0, 8));
		muduo::string cmd;
		if (cmdBuf.find('\0') > 0)
		{
			cmd = cmdBuf.substr(0, cmdBuf.find('\0'));
		}
		else
		{
			cmd = cmdBuf;
		}

		muduo::string format;
		muduo::string formatBuf(message.substr(8, 4));
		if (formatBuf.find('\0') > 0)
		{
			format = formatBuf.substr(0, formatBuf.find('\0'));
		}
		else
		{
			format = formatBuf;
		}

		muduo::string contractNo;
		muduo::string contractNoBuf(message.substr(12, 8));
		if (contractNoBuf.find('\0') > 0)
		{
			contractNo = contractNoBuf.substr(0, contractNoBuf.find('\0'));
		}
		else
		{
			contractNo = contractNoBuf;
		}

		muduo::string body(message.substr(packetLen));
		cout << cmd << " " << format << " " << contractNo << " " << body << endl;
	}
}
