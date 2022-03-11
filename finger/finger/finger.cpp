#include "muduo/net/TcpServer.h"
#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"


using namespace muduo;
using namespace muduo::net;
void onConnection(const TcpConnectionPtr &conn)
{
	if (conn->connected())
	{
		conn->shutdown();
	}
}

int main()
{
	EventLoop loop;
	TcpServer server(&loop, InetAddress(1079), "Finger");
	server.start();
	loop.loop();
}