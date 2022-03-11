#include "./codec.h"
#include "./server.h"

using namespace muduo;
using namespace muduo::net;

using fun = std::function<void()>;

comServer::comServer(EventLoop* loop,
	const InetAddress& listenAddr, int threadNums)
	:
	server_(loop, listenAddr, "comServer"),
	codec_(std::bind(&comServer::onStringMessage, this, _1, _2, _3)),
	mysqlPool_(NULL)
{
	server_.setConnectionCallback(
		std::bind(&comServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	mysqlPool_ = MysqlPool::getMysqlPoolObject();
	server_.setThreadNum(threadNums);//开启IO线程	
}
void comServer::start()
{
	server_.start();
}

void comServer::onConnection(const TcpConnectionPtr& conn)
{
	LOG_INFO << conn->localAddress().toIpPort() << " -> "
		<< conn->peerAddress().toIpPort() << " is "
		<< (conn->connected() ? "UP" : "DOWN");

	if (conn->connected())
	{
		std::lock_guard<mutex> locallock(connectionsLock_);
		connections_.insert(conn);//收到连接之后保存到集合里
	}
	else
	{
		std::lock_guard<mutex> locallock(connectionsLock_);
		connections_.erase(conn);//断开连接之后把该连接从集合删除
		conn->shutdown();
	}

}
void comServer::dealMessage(TcpConnectionPtr &conn, const string& msg)
{
	/*printf("current pid = %d, tid = %d deal message\n",
		getpid(), CurrentThread::tid());*/
	if (msg.size() >= headerLen)
	{
		muduo::string cmdBuf(msg.substr(0, 8));
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
		muduo::string formatBuf(msg.substr(8, 4));
		if (formatBuf.find('\0') > 0)
		{
			format = formatBuf.substr(0, formatBuf.find('\0'));
		}
		else
		{
			format = formatBuf;
		}
		muduo::string userNameBuf(msg.substr(12, 32));
		muduo::string userName;
		if (userNameBuf.find('\0') > 0)
		{
			userName = userNameBuf.substr(0, userNameBuf.find('\0'));
		}
		else
		{
			userName = userNameBuf;
		}
		muduo::string body(msg.substr(headerLen));
		if (cmd == "RQUPJ"&&format == "GZIP")
		{
			shared_ptr<MysqlConnection> sqlConn = mysqlPool_->getOneConnect();
			if (sqlConn == nullptr)
			{
				LOG_WARN << "Emptr connect pointer";
				return;
			}
			else {
				sqlConn->mysqlInsert(userName, body);
			}
		}
		else if (cmd == "RQDPJ")//等于0加get表示后面没跟数据,并且需要返回数据
		{
			string sqlQueryRes;
			shared_ptr<MysqlConnection> sqlConn = mysqlPool_->getOneConnect();
			sqlConn->mysqlQuery(userName, sqlQueryRes);
			if (sqlQueryRes.size() <= 0)
			{
				LOG_WARN<<"QUERY FAILER";
				return;
			}
			cmd = "RSDPJ";
			if (sqlQueryRes == "FAIL")
			{
				format = "TXT";
				codec_.send(get_pointer(conn), cmd, format,userName, sqlQueryRes);

			}
			else
			{
				format = "GZIP";
				codec_.send(get_pointer(conn), cmd, format, userName, sqlQueryRes);//将查询结果作为字符串返回，并且加上getRes头
			}
		}
		else if (cmd != "HBET")
		{
			std::lock_guard<mutex> locallock(connectionsLock_);
			connections_.erase(conn);//如果既不是RQDPJ也不是RQUPJ也不是HBET,直接断开。
			conn->shutdown();
		}
		//进来的只有put和get,get的话就直接返回查询结果以便于在客户端合并或者直接覆盖
	}
}
void comServer::onStringMessage(const TcpConnectionPtr& conn,
	const string& message,
	Timestamp)
{
	threadPool_.run(std::bind(&comServer::dealMessage, this, conn, message));
	/*printf("current pid = %d, tid = %d package\n",
		getpid(), CurrentThread::tid());*/
}

void comServer::initThreadPool(int threadNum)
{
	threadPool_.start(threadNum); //创建threadNum个线程，并启动线程
}

comServer::~comServer()
{
	cout << "~comServer()" << endl;
}
