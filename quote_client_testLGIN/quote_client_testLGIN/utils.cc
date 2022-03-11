#include "utils.h"

const int maxSize = 5000;
std::unordered_set<muduo::string> tickSet;
std::list<muduo::string> tickList;
std::queue<muduo::string> resQue;
MutexLock resMutex;
MutexLock tickMutex;
MutexLock mapMutex;
std::queue<std::pair<time_t, double>> timePriceQue;
using timePair = std::pair < time_t, double >;
std::map<muduo::string, std::queue<std::pair<time_t, double>>> queueMap;//key,后面存时间+最新价

bool loadConfigFile(string &host1, uint16_t &port1)
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
		else
		{
			LOG_ERROR << "wrong conf format";
			return false;
		}
	}
	return true;
}

time_t str_to_time_t(const string& ATime)
{
	struct tm tm_Temp;
	time_t time_Ret;
	const string& AFormat = "%d-%d-%d %d:%d:%d";
	try
	{
		sscanf(ATime.c_str(), AFormat.c_str(),// "%d/%d/%d %d:%d:%d" ,       
			&(tm_Temp.tm_year),
			&(tm_Temp.tm_mon),
			&(tm_Temp.tm_mday),
			&(tm_Temp.tm_hour),
			&(tm_Temp.tm_min),
			&(tm_Temp.tm_sec));

		tm_Temp.tm_year -= 1900;
		tm_Temp.tm_mon--;
		//如果精确到秒就把下面四行注释掉;如果精确到天就把下面四行代码放开
		//tm_Temp.tm_hour = 0;
		//tm_Temp.tm_min = 0;
		//tm_Temp.tm_sec = 0;
		//tm_Temp.tm_isdst = 0;
		time_Ret = mktime(&tm_Temp);
		return time_Ret;
	}
	catch (...) {
		return 0;
	}
}

time_t NowTime()
{
	time_t t_Now = time(0);
	struct tm* tm_Now = localtime(&t_Now);
	//如果精确到秒就把下面四行注释掉;如果精确到天就把下面四行代码放开
	//tm_Now->tm_hour = 0;
	//tm_Now->tm_min = 0;
	//tm_Now->tm_sec = 0;
	return  mktime(tm_Now);
}

double round(double src)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << src;
	ss >> src;
	return src;
}