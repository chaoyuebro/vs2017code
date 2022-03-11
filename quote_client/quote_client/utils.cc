#include "utils.h"

const int maxSize = 5000;
//�����洢��ʼtick����,set�����ӿ��ѯ�ٶ�
std::unordered_set<muduo::string> tickSet;
std::list<muduo::string> tickList;
//��Ž���Ա��ڽ����ͻ��˷���
std::queue<resStruct> resQue;
MutexLock resMutex;
MutexLock tickMutex;
MutexLock mapMutex;
using timePair = std::pair < time_t, double >;
//��ʼ��һ���洢ʱ��,���¼۵�pair.
std::queue<timePair> timePriceQue;
//��map��key����ʾ���ĸ���Լ��1,3,5����,��queue���洢��ֵ�ԡ��Ա��ڱȽ�ʱ��ͬʱ�����Ǯ��
std::map<muduo::string, std::queue<std::pair<time_t, double>>> queueMap;//key,�����ʱ��+���¼�

void dealMessage()
{
	muduo::string itFront;
	muduo::string str, str1, str3, str5, outStr;
	double change1 = 0, change3 = 0, change5 = 0;//�����ǵ���
	double newPrice;
	time_t tradeTime;
	std::vector<muduo::string> strVec;
	std::stringstream ss;
	muduo::string resStr;
	resStruct res;
	bool outSuccess=false;//�ж��Ƿ�ɹ���listȡ��,�Ա����lock��Χ,����ȡ��Ч��
	while (1)
	{
		MutexLockGuard tickLock(tickMutex);
		if (!tickList.empty() && !tickSet.empty())
		{
			outSuccess = true;
			itFront = tickList.front();
			tickList.pop_front();
			tickSet.erase(itFront);
		}//tickLock����
		else
		{
			outSuccess = false;
		}
		if(outSuccess)//���ȡ���ɹ�
		{
			//cout << itFront<< endl;
			char *tmpStr = strtok(const_cast<char*>(itFront.c_str()), "|");
			while (tmpStr != NULL)
			{
				strVec.push_back(std::move(string(tmpStr)));
				tmpStr = strtok(NULL, "|");
			}
			//cout << strVec[1] << endl;
			str = strVec[1] + strVec[2] + strVec[3];
			str1 = str + '1';
			str3 = str + '3';
			str5 = str + '5';
			//�γɽ����struct,�Ա��ڸ�����˷���
			res.comodity = strVec[1];
			res.contractNo = strVec[2];
			res.exchange = strVec[3];
			outStr = strVec[1] + '|' + strVec[2] + '|' + strVec[3];
			tradeTime = str_to_time_t(strVec[4]);
			newPrice = std::stod(strVec[10].c_str());
			strVec.clear();
			MutexLockGuard mapLock(mapMutex);
			{
				auto it = queueMap.find(str1);
				if (it == queueMap.end())//���û�ҵ�,�ʹ�����Ӧ��1����,3����,5���Ӷ��С�
				{
					queueMap.insert(std::pair<muduo::string, std::queue<std::pair<time_t, double>>>(str1, timePriceQue));
					queueMap.insert(std::pair<muduo::string, std::queue<std::pair<time_t, double>>>(str3, timePriceQue));
					queueMap.insert(std::pair<muduo::string, std::queue<std::pair<time_t, double>>>(str5, timePriceQue));
				}
				//�����ж����ͷ��ʱ����Ƿ����60s,180s,300s.
				//1����
				while (!queueMap[str1].empty() && (difftime(tradeTime, queueMap[str1].front().first) > 60))
				{
					queueMap[str1].pop();
				}
				queueMap[str1].push(timePair(tradeTime, newPrice));
				change1 = round(((newPrice - queueMap[str1].front().second) / queueMap[str1].front().second) * 100);
				if (change1 == -0)
				{
					change1 = 0;
				}
				//3����
				while (!queueMap[str3].empty() && difftime(tradeTime, queueMap[str3].front().first) > 180)
				{
					queueMap[str3].pop();
				}
				queueMap[str3].push(timePair(tradeTime, newPrice));
				change3 = round(((newPrice - queueMap[str3].front().second) / queueMap[str3].front().second) * 100);
				if (change3 == -0)
				{
					change3 = 0;
				}
				//5����
				while (!queueMap[str5].empty() && difftime(tradeTime, queueMap[str5].front().first) > 300)
				{
					queueMap[str5].pop();
				}
				queueMap[str5].push(timePair(tradeTime, newPrice));
				change5 = round(((newPrice - queueMap[str5].front().second) / queueMap[str5].front().second) * 100);
				if (change5 == -0)
				{
					change5 = 0;
				}
			}//maplock����
			ss << outStr << "|" << change1 << "|" << change3 << "|" << change5;
			ss >> resStr;
			ss.clear();
			res.resStr = resStr;
			MutexLockGuard resLock(resMutex);
			resQue.push(res);
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

time_t str_to_time_t(const string& ATime)
{
	struct tm tm_Temp = {'\0'};
	time_t time_Ret=0;
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
	//�����ȷ����Ͱ���������ע�͵�;�����ȷ����Ͱ��������д���ſ�
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