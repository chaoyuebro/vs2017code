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
		connections_.insert(conn);//�յ�����֮�󱣴浽������
	}
	else
	{
		connections_.erase(conn);//�Ͽ�����֮��Ѹ����ӴӼ���ɾ��
		conn->shutdown();
	}

}
void comServer::sendMessage(TcpConnectionPtr &conn, const string& msg)
{
	printf("current pid = %d, tid = %d send\n",
		getpid(), CurrentThread::tid());
	codec_.send(get_pointer(conn), msg);
}
void comServer::onStringMessage(const TcpConnectionPtr& conn,
	const string& message,
	Timestamp)
{
	std::cout << message << std::endl;
	threadPool_.run(std::bind(&comServer::sendMessage,this, conn, message));
	printf("current pid = %d, tid = %d package\n",
		getpid(), CurrentThread::tid());
}

void comServer::initThreadPool(int maxQueSize, int threadNum)
{
	threadPool_.start(threadNum); //����5���̣߳��������߳�
}
