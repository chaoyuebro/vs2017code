#ifndef __UTILS_H__
#define __UTILS_H__

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/base/AsyncLogging.h"
#include "muduo/net/EventLoop.h"
#include "muduo/base/TimeZone.h"
#include "muduo/base/Mutex.h"

#include <time.h>
#include <queue>
#include <unordered_set>
#include <list>
#include <map>
#include <vector>
#include <mutex>
#include <stack>
#include <iostream>    //��дio c++��׼��
#include <fstream>     //��д�ļ� c++��׼��
#include <string>      //�ַ����� c++��׼��
#include <sstream>     //�ַ����� c++��׼��
#include <iomanip>

using namespace muduo;
using namespace muduo::net;
using std::cout;
using std::endl;
extern const int maxSize;

extern std::unordered_set<muduo::string> tickSet;
extern std::list<muduo::string> tickList;
extern std::map<muduo::string, std::stack<std::pair<time_t, double>>> stackMap;

extern MutexLock resMutex;
extern MutexLock tickMutex;
extern MutexLock mapMutex;
extern void dealMessage();
extern bool loadConfigFile(string &, uint16_t &, string &, uint16_t &,uint16_t &);//���������ļ�
extern time_t str_to_time_t(const string&);//���ַ���ת��ʱ��
extern time_t NowTime();//��ǰʱ��
extern double round(double);
typedef struct {
	string exchange; //������
	string comodity; //Ʒ��
	string contractNo; //��Լ
	string resStr;  //����ȫ��
}
resStruct;
extern std::queue<resStruct> resQue;
#endif
