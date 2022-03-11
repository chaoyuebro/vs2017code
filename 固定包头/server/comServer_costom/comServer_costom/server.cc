#include "./codec.h"
#include "./server.h"

using namespace muduo;
using namespace muduo::net;
using fun = std::function<void()>;


comServer::comServer(EventLoop* loop,
	const InetAddress& listenAddr,int threadNums)
	:
	server_(loop, listenAddr, "comServer"),
	codec_(std::bind(&comServer::onStringMessage, this, _1, _2, _3))
{
	server_.setConnectionCallback(
		std::bind(&comServer::onConnection, this, _1));
	server_.setMessageCallback(
		std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
	server_.setThreadNum(threadNums);
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
		connections_.insert(conn);//收到连接之后保存到集合里
	}
	else
	{
		connections_.erase(conn);//断开连接之后把该连接从集合删除
		conn->shutdown();
	}

}
void comServer::sendMessage(TcpConnectionPtr &conn, const string& msg)
{
	printf("current pid = %d, tid = %d send\n",
		getpid(), CurrentThread::tid());
	cout << msg << endl;
	mysqlTrans_.inputMysql(msg);
	codec_.send(get_pointer(conn), "success");
}
void comServer::onStringMessage(const TcpConnectionPtr& conn,
	const string& message,
	Timestamp)
{
	threadPool_.run(std::bind(&comServer::sendMessage,this, conn, message));
	printf("current pid = %d, tid = %d package\n",
		getpid(), CurrentThread::tid());
}

void comServer::initThreadPool(int threadNum)
{
	threadPool_.start(threadNum); //创建5个线程，并启动线程
}
