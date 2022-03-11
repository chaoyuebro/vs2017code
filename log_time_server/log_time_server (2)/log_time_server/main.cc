#include "./server.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/TimeZone.h"
#include <unistd.h>
#include <sys/stat.h>

using namespace muduo;
using namespace muduo::net;
//异步日志对象
muduo::AsyncLogging* g_asyncLog = NULL;
void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);

}
int main(int argc, char* argv[])
{
	if (access("./log/", F_OK) != 0)
	{
		if (mkdir("./log/", 0777) == -1)
		{
			std::cerr << "创建log目录失败" << std::endl;
			return 0;
		}
	}
	//初始化异步日志的对象
	muduo::AsyncLogging log("./log/Exchange", 500 * 1000 * 1000);
	log.start();
	g_asyncLog = &log;
	//设置日志输出级别
	Logger::setLogLevel(Logger::DEBUG);
	//设置日志输出方式
	Logger::setOutput(asyncOutput);
	//设置时区. 日志里面的时间
	TimeZone tzBeiJing("/usr/share/zoneinfo/Asia/Chongqing");
	Logger::setTimeZone(tzBeiJing);
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
		server.initThreadPool(4);
		server.start();
		loop.loop();
	}
	else
	{
		LOG_WARN << "Usage:" << argv[0] << "port";
	}
}
