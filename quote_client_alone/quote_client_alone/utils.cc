#include "utils.h"

const int maxSize = 5000;
//�����洢��ʼtick����,set�����ӿ��ѯ�ٶ�
std::unordered_set<muduo::string> tickSet;
std::list<muduo::string> tickList;
//��Ž���Ա��ڽ����ͻ��˷���
MutexLock tickMutex;
MutexLock queMutex;
using timePair = std::pair < time_t, double >;
//��ʼ��һ���洢ʱ��,���¼۵�pair.
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
	char *cha = (char*)str.data();             // ��stringת����char*��
	tm tm_;                                    // ����tm�ṹ�塣
	int year, month, day, hour, minute, second;// ����ʱ��ĸ���int��ʱ������
	sscanf(cha, "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);// ��string�洢������ʱ�䣬ת��Ϊint��ʱ������
	tm_.tm_year = year - 1900;                 // �꣬����tm�ṹ��洢���Ǵ�1900�꿪ʼ��ʱ�䣬����tm_yearΪint��ʱ������ȥ1900��
	tm_.tm_mon = month - 1;                    // �£�����tm�ṹ����·ݴ洢��ΧΪ0-11������tm_monΪint��ʱ������ȥ1��
	tm_.tm_mday = day;                         // �ա�
	tm_.tm_hour = hour;                        // ʱ��
	tm_.tm_min = minute;                       // �֡�
	tm_.tm_sec = second;                       // �롣
	tm_.tm_isdst = 0;                          // ������ʱ��
	time_t t_ = mktime(&tm_);                  // ��tm�ṹ��ת����time_t��ʽ��
	return t_;                                 // ����ֵ��
}

double round(double src)
{
	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << src;
	ss >> src;
	return src;
}