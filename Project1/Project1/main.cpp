#include "echo.h"


int main()
{
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(2007);
	EchoServer server(&loop, listenAddr);
	server.start();
	loop.loop();
}