#include "client.h"
#include "utils.h"
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
#include <mutex>

using std::cout;
using std::endl;

muduo::AsyncLogging* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len)
{
	g_asyncLog->append(msg, len);
}

int main(int argc, char* argv[])
{
	//if (access("./log/", F_OK) != 0)
	//{
	//	if (mkdir("./log/", 0777) == -1)
	//	{
	//		std::cerr << "����logĿ¼ʧ��" << std::endl;
	//		return 0;
	//	}
	//}
	////��ʼ���첽��־�Ķ���
	//muduo::AsyncLogging log("./log/Exchange", 500 * 1000 * 1000);
	//log.start();
	//g_asyncLog = &log;
	////������־�������
	//Logger::setLogLevel(Logger::DEBUG);
	////������־�����ʽ
	//Logger::setOutput(asyncOutput);
	//����ʱ��. ��־�����ʱ��
	TimeZone tzBeiJing("/usr/share/zoneinfo/Asia/Chongqing");
	Logger::setTimeZone(tzBeiJing);
	LOG_INFO << "pid = " << getpid();
	string host1;
	uint16_t port1;
	if (!loadConfigFile(host1,port1))
	{
		LOG_ERROR<<"loadConfigFile fail";
		exit(0);
	}
	EventLoopThread loopThread1;
	InetAddress serverAddr1(host1,port1);
	quoteClient client1(loopThread1.startLoop(), serverAddr1);
	/*thread heart(std::bind(&ChatClient::sendHeartBeat, &client));
	heart.detach();*/
	client1.connect();
	//usleep(1000 * 1000);
	//client1.write("LGIN", "TXT", lginBody);
	while (1)
	{	
		usleep(1000000);
	}
	client1.disconnect();
	CurrentThread::sleepUsec(1000 * 1000);  // wait for disconnect, see ace/logging/client.cc
	return 0;
}

