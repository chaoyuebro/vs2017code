#include "./server.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/base/Logging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/TimeZone.h"
#include <unistd.h>
#include <sys/stat.h>


using namespace muduo;
using namespace muduo::net;
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
			std::cerr << "����logĿ¼ʧ��" << std::endl;
			return 0;
		}
	}
	//��ʼ���첽��־�Ķ���
	muduo::AsyncLogging log("./log/Exchange", 500 * 1000 * 1000);
	log.start();
	g_asyncLog = &log;
	//������־�������
	Logger::setLogLevel(Logger::DEBUG);
	//������־�����ʽ
	Logger::setOutput(asyncOutput);
	//����ʱ��. ��־�����ʱ��
	TimeZone tzBeiJing("/usr/share/zoneinfo/Asia/Chongqing");
	Logger::setTimeZone(tzBeiJing);
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
		server.initThreadPool(4);
		server.start();
		loop.loop();
	}
	else
	{
		printf("Usage: %s port\n", argv[0]);
	}
}
