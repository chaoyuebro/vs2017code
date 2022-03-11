#include "client.h"
#include "utils.h"
#include "server.h"
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
	string host1,host2;
	uint16_t port1,port2,serverPort;
	if (!loadConfigFile(host1,port1,host2,port2,serverPort))
	{
		LOG_ERROR<<"loadConfigFile fail";
		exit(0);
	}
	EventLoopThread loopThread1;
	EventLoopThread	loopThread2;
	InetAddress serverAddr1(host1,port1);
	InetAddress serverAddr2(host2,port2);
	quoteClient client1(loopThread1.startLoop(), serverAddr1);
	quoteClient client2(loopThread2.startLoop(), serverAddr2);
	//��ʼ��������
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(serverPort);
	QuoteServer server(&loop, listenAddr);
	/*thread heart(std::bind(&ChatClient::sendHeartBeat, &client));
	heart.detach();*/
	client1.connect();
	client2.connect();
	usleep(1000 * 1000);
	std::thread dealMsg1(dealMessage);
	dealMsg1.detach();
	std::thread dealMsg2(dealMessage);
	dealMsg2.detach();
	/*client1.write("LGIN", "TXT", lginBody);
	client2.write("LGIN", "TXT", lginBody);*/
	server.start();
	loop.loop();
	while (1)
	{	
		usleep(1000000);
	}
	client1.disconnect();
	client2.disconnect();
	CurrentThread::sleepUsec(1000 * 1000);  // wait for disconnect, see ace/logging/client.cc
	return 0;
}

