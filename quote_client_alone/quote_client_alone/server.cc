#include "server.h"
#include "muduo/base/Logging.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

void QuoteServer::start()
{
	server_.start();
}

void QuoteServer::onConnection(const muduo::net::TcpConnectionPtr& conn)
{
	LOG_INFO << conn->peerAddress().toIpPort() << " -> "
		<< conn->localAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

	if (conn->connected())
	{
		MutexLockGuard lock(mutex_);
		connections_.insert(conn);
	}
	else
	{
		MutexLockGuard lock(mutex_);
		connections_.erase(conn);
	}
}

void QuoteServer::onStringMessage(const TcpConnectionPtr&conn,
	const string& message,
	Timestamp)
{
	if (message.size() >= headerLen)
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
		muduo::string body(message.substr(headerLen,message.length()-headerLen));
		if (cmd =="LGIN"&&format=="TXT"&&body==lginBody)
		{
			MutexLockGuard lock(checkMutex_);
			checkConns_.insert(conn);
		}
	}
	
}
void QuoteServer::loopSend()
{
	resStruct res;
	while (1)
	{
		MutexLockGuard resLock(resMutex);
		if (!resQue.empty())
		{
			res = resQue.front();
			resQue.pop();
			//cout << "after pop" << endl;
			for (ConnectionList::iterator it = checkConns_.begin();
				it != checkConns_.end();
				++it)
			{
				//cout << "send" << endl;
				codec_.send(get_pointer(*it), res.comodity,res.contractNo,res.exchange,res.resStr);
			}
		}
	}
}