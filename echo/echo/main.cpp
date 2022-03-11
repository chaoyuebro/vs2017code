#include "./echo.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"



 using namespace muduo;
 using namespace muduo::net;

int main(int argc,char *argv[])
{
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(2007);
	EchoServer server(&loop, listenAddr);
	server.start();
	loop.loop();
}
