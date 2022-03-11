#include "./server.h"

#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"



using namespace muduo;
using namespace muduo::net;

int main(int argc, char* argv[])
{
	LOG_INFO << "pid = " << getpid();
	if (argc > 1)
	{
		EventLoop loop;
		uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
		int threadNums;
		InetAddress serverAddr(port);
		if (argc > 2)
		{
			threadNums=atoi(argv[2]);
		}
		else
		{
			threadNums = 4;
		}
		comServer server(&loop, serverAddr,threadNums);	
		server.initThreadPool(14);
		server.start();
		loop.loop();
	}
	else
	{
		printf("Usage: %s port\n", argv[0]);
	}
}
