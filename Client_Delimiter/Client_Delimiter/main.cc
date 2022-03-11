#include"./client.cc"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/TimeZone.h"
#include <unistd.h>
#include <thread>
#include <sys/stat.h>

int main(int argc, char* argv[])
{
	TimeZone tzBeiJing("/usr/share/zoneinfo/Asia/Chongqing");
	Logger::setTimeZone(tzBeiJing);
	LOG_INFO << "pid = " << getpid();
	if (argc > 2)
	{
		EventLoopThread loopThread;
		uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
		InetAddress serverAddr(argv[1], port);
		ChatClient client(loopThread.startLoop(), serverAddr);
		client.connect();
		sleep(1);
		std::thread heart(std::bind(&ChatClient::sendHeartBeat, &client));
		heart.detach();
		std::string line;
		cout << "please select put or get" << endl;
		while (std::getline(std::cin, line))
		{
			if (line == "put")
			{
				JsonTrans jsonTrans;
				string jsonStr;
				jsonTrans.inputJson(jsonStr);
				client.write("RQUPJ", "GZIP",jsonStr);
			}
			else if (line == "get")//如果是get,就要选择是合并还是直接替换。
			{
				client.write("RQDPJ", "GZIP",string());
			}
		}
		client.disconnect();
		CurrentThread::sleepUsec(1000 * 1000);  // wait for disconnect, see ace/logging/client.cc
	}
	else
	{
		printf("Usage: %s host_ip port\n", argv[0]);
	}
}

