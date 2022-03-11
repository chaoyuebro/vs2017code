#include "./codec.h"
#include "./server.h"

using namespace muduo;
using namespace muduo::net;
using fun = std::function<void()>;

using std::placeholders::_4;
comServer::comServer(EventLoop* loop,
	const InetAddress& listenAddr, int threadNums)
	:
	server_(loop, listenAddr, "comServer"),
	codec_(std::bind(&comServer::onStringMessage, this, _1, _2, _3))
{
	server_.setConnectionCallback(
		std::bind(&comServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	server_.setThreadNum(threadNums);
	mysqlPool_ = MysqlPool::getMysqlPoolObject();
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
		connections_.insert(conn);//�յ�����֮�󱣴浽������
	}
	else
	{
		std::lock_guard<mutex> locallock(connectionsLock_);
		connections_.erase(conn);//�Ͽ�����֮��Ѹ����ӴӼ���ɾ��
		conn->shutdown();
	}

}
void comServer::dealMessage(TcpConnectionPtr &conn, const string& msg)
{
	/*printf("current pid = %d, tid = %d deal message\n",
		getpid(), CurrentThread::tid());*/
	if (msg.size() >= headerLen)
	{
		muduo::string cmdBuf(msg.substr(0, 5));
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
		muduo::string userNameBuf(msg.substr(12, 13));
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
		shared_ptr<MysqlConnection> sqlConn = mysqlPool_->getOneConnect();
		sqlConn->mysqlInsert(userName,body);
		codec_.send(get_pointer(conn), string(), string(), string(), string());
	}
		
		//else if (cmd == "RQDPJ")//����0��get��ʾ����û������,������Ҫ��������
		//{
		//	string sqlQueryRes;
		//	shared_ptr<MysqlConnection> sqlConn = mysqlPool_->getOneConnect();
		//	sqlConn->mysqlQuery(userName, sqlQueryRes);
		//	if (sqlQueryRes.size() <= 0)
		//	{
		//		printf("QUERY FAILER");
		//		return;
		//	}
		//	cmd = "RSDPJ";
		//	if (sqlQueryRes == "FAIL")
		//	{
		//		format = "TXT";
		//		codec_.send(get_pointer(conn), cmd, format,userName, sqlQueryRes);

		//	}
		//	else
		//	{
		//		format = "GZIP";
		//		int bodySize = sqlQueryRes.size();
		//		int destLen = bodySize * 2;
		//		char* compressed = (char *)malloc(destLen);
		//		memset(compressed, 0, destLen);
		//		const char *source = sqlQueryRes.c_str();
		//		int gzSize = gzCompress(source, bodySize, compressed, destLen);
		//		if (gzSize <= 0)
		//		{
		//			printf("compress error.\n");
		//			return;
		//		}
		//		string compressBuf(compressed, gzSize);
		//		codec_.send(get_pointer(conn), cmd, format, userName, compressBuf);//����ѯ�����Ϊ�ַ������أ����Ҽ���getResͷ��
		//		free(compressed);
		//	}
		//}
		//������ֻ��put��get,get�Ļ���ֱ�ӷ��ز�ѯ����Ա����ڿͻ��˺ϲ�����ֱ�Ӹ���
	
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
	threadPool_.start(threadNum); //����threadNum���̣߳��������߳�
}

comServer::~comServer()
{
	cout << "~comServer()" << endl;
}
