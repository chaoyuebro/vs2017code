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
#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库
#include <iomanip>

using namespace muduo;
using namespace muduo::net;
using std::cout;
using std::endl;
extern const int maxSize;

extern std::unordered_set<muduo::string> tickSet;
extern std::list<muduo::string> tickList;
extern std::map<muduo::string, std::stack<std::pair<time_t, double>>> stackMap;
extern std::queue<muduo::string> resQue;
extern MutexLock resMutex;
extern MutexLock tickMutex;
extern MutexLock mapMutex;
extern void dealMessage();
extern bool loadConfigFile(string &, uint16_t &);//加载配置文件
extern time_t str_to_time_t(const string&);//将字符串转成时间
extern time_t NowTime();//当前时间
extern double round(double);
#endif
