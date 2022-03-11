#include "utils.h"

const int maxSize = 5000;
//用来存储初始tick数据,set用来加快查询速度
std::unordered_set<muduo::string> tickSet;
std::list<muduo::string> tickList;
//存放结果以便于交给客户端发送
MutexLock tickMutex;
MutexLock queMutex;
using timePair = std::pair < time_t, double >;
//初始化一个存储时间,最新价的pair.
std::queue<timePair> timePriceQue;



void dealMessage()
{
	muduo::string itFront;
	muduo::string str,outStr;
	muduo::string comodity, contractNo;
	double change = 0;
	double newPrice;
	time_t tradeTime;
	std::vector<muduo::string> strVec;
	std::stringstream ss;
	muduo::string resStr;
	resStruct res;
	bool outSuccess=false;//判断是否成功从list取出,以便减少lock范围,增加取出效率
	while (1)
	{
		MutexLockGuard tickLock(tickMutex);
		if (!tickList.empty() && !tickSet.empty())
		{
			outSuccess = true;
			itFront = tickList.front();
			tickList.pop_front();
			tickSet.erase(itFront);
		}//tickLock结束
		else
		{
			outSuccess = false;
		}
		if(outSuccess)//如果取出成功
		{
			//cout << itFront<< endl;
			char *tmpStr = strtok(const_cast<char*>(itFront.c_str()), "|");
			while (tmpStr != NULL)
			{
				strVec.push_back(std::move(string(tmpStr)));
				tmpStr = strtok(NULL, "|");
			}
			tradeTime = str_to_time_t(strVec[4]);
			newPrice = std::stod(strVec[10].c_str());
			//cout << newPrice << endl;
			str = strVec[1] + strVec[2] + strVec[3];
			comodity = strVec[2];
			contractNo = strVec[3];
			strVec.clear();
			if (comodity == "I"&&contractNo == "2205")
			{	
				MutexLockGuard queLock(queMutex);
				{
					while (!timePriceQue.empty() && (difftime(tradeTime, timePriceQue.front().first) > 60))
					{
						timePriceQue.pop();
					}
					timePriceQue.push(timePair(tradeTime, newPrice));
					change = round(((newPrice - timePriceQue.front().second) / timePriceQue.front().second) * 100);
					if (change == -0)
					{
						change = 0;
					}
					cout << change << endl;
					//cout << newPrice << endl;
				}
			}		
		}
	}
}


bool loadConfigFile(string &host1, uint16_t &port1, string &host2, uint16_t &port2, uint16_t &serverPort)
{
	string filename("client.conf");
	std::ifstream ifs(filename);
	if (!ifs)
	{
		LOG_ERROR << "open file error!";
		return false;
	}
	string line;
	string key, value;
	while (getline(ifs, line))
	{
		std::istringstream iss(line);
		iss >> key >> value;
		if (key == "ip1")
		{
			host1 = value;
		}
		else if (key == "port1")
		{
			port1 = static_cast<uint16_t>(atoi(value.c_str()));

		}
		else if (key == "ip2")
		{
			host2 = value;
		}
		else if (key == "port2")
		{
			port2 = static_cast<uint16_t>(atoi(value.c_str()));

		}
		else if (key == "serverPort")
		{
			serverPort = static_cast<uint16_t>(atoi(value.c_str()));
		}
		else
		{
			LOG_ERROR << "wrong conf format";
			return false;
		}
	}
	return true;
}

time_t str_to_time_t(const string& str)
{
	char *cha = (char*)str.data();             // 将string转换成char*。
	tm tm_;                                    // 定义tm结构体。
	int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// 将string存储的日期时间，转换为int临时变量。
	tm_.tm_year = year - 1900;                 // 年，由于tm结构体存储的是从1900年开始的时间，所以tm_year为int临时变量减去1900。
	tm_.tm_mon = month - 1;                    // 月，由于tm结构体的月份存储范围为0-11，所以tm_mon为int临时变量减去1。
	tm_.tm_mday = day;                         // 日。
	tm_.tm_hour = hour;                        // 时。
	tm_.tm_min = minute;                       // 分。
	tm_.tm_sec = second;                       // 秒。
	tm_.tm_isdst = 0;                          // 非夏令时。
	time_t t_ = mktime(&tm_);                  // 将tm结构体转换成time_t格式。
	return t_;                                 // 返回值。
}

double round(double src)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << src;
	ss >> src;
	return src;
}