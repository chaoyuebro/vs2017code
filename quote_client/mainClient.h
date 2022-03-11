#ifndef __MAINCLIENT_H__
#define __MAINCLIENT_H__


#include "codec.h"
#include "client.h"

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"

#include <stdio.h>
#include <unistd.h>

#include <iostream>
#include <thread>
#include <unordered_set>
#include <list>
#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库

using namespace muduo;
using namespace muduo::net;

using std::cout;
using std::endl;


class mainClient : noncopyable
{
public:
	mainClient(string &host1, uint16_t &port1, string &host2, uint16_t &port2)
	:serverAddr1_(host1, port1)
	,serverAddr2_(host2, port2)
	,client1_(loopThread1_.startLoop(), serverAddr1_)
	,client2_(loopThread2_.startLoop(), serverAddr2_)
	{
		client1_.connect();
		client2_.connect();
	}
	void start();
	~mainClient()
	{
		client1_.disconnect();
		client2_.disconnect();
	}
private:
	EventLoopThread loopThread1_;
	EventLoopThread	loopThread2_;
	InetAddress serverAddr1_;
	InetAddress serverAddr2_;
	quoteClient client1_;
	quoteClient client2_;	
	std::unordered_set<muduo::string> tickSet_;
	std::list<muduo::string> tickList_;
	MutexLock mutex_;
};













#endif // !__MAINCLIENT_H__