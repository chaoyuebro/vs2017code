#include "mysqlPool.h"
#include "muduo/base/Logging.h"

using namespace muduo;
using namespace muduo::net;

shared_ptr<MysqlPool> MysqlPool::mysqlpool_object = NULL;
std::mutex MysqlPool::_queueLock;
std::mutex MysqlPool::_poolLock;
// connectPool启动时先启动initCount个连接
MysqlPool::MysqlPool()
	:_initCount(0),
	_maxCount(0),
	_currentCount(0)
{
	// 加载配置项了
	if (loadConfigFile())
	{
		// 创建初始数量的连接
		for (int i = 0; i < _initCount; ++i)
		{
			MysqlConnection *p = new MysqlConnection();
			bool connect = p->connect(_mysqlhost, _port,_mysqluser, _mysqlpwd, _databasename);
			if (connect)
			{
				p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
				_connQueue.push(p);
				_currentCount++;
			}
		}
	}
	if (_currentCount == 0)
	{
		LOG_ERROR << "Can't Connect Mysql";
		exit(0);
	}
	// 启动一个新的线程，作为连接的生产者 linux thread => pthread_create
	thread produce(std::bind(&MysqlPool::produceConnectionTask, this));
	produce.detach();

	// 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
	thread scanner(std::bind(&MysqlPool::scannerConnectionTask, this));
	scanner.detach();
}

/*
 *配置数据库参数
 */
bool MysqlPool::loadConfigFile()
{

	string filename("mysql.conf");
	ifstream ifs(filename);
	if (!ifs)
	{
		LOG_ERROR << "open file error!";
		return false;
	}
	string line;
	string key, value;
	while (getline(ifs, line))
	{
		istringstream iss(line);
		iss >> key >> value;
		if (key == "ip")
		{
			_mysqlhost = value;
		}
		else if (key == "username")
		{
			_mysqluser = value;
		}
		else if (key == "password")
		{
			_mysqlpwd = value;
		}
		else if (key == "dbname")
		{
			_databasename = value;
		}
		else if (key == "initSize")
		{
			_initCount = atoi(value.c_str());
		}
		else if (key == "port")
		{
			_port = atoi(value.c_str());
		}
		else if (key == "maxSize")
		{
			_maxCount = atoi(value.c_str());
		}
		else if (key == "maxIdleTime")
		{
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut")
		{
			_connectionTimeout = atoi(value.c_str());
		}
		else
		{
			LOG_ERROR << "wrong conf format";
			return false;
		}
	}
	return true;
}
/*
 *有参的单例函数，用于第一次获取连接池对象，初始化数据库信息。
 */
MysqlPool* MysqlPool::getMysqlPoolObject()
{
	if (mysqlpool_object == NULL)
	{ //双检锁机制
		_poolLock.lock();
		if (mysqlpool_object == NULL)
		{
			mysqlpool_object.reset(new MysqlPool());
		}
		_poolLock.unlock();
	}
	return mysqlpool_object.get();
}

void MysqlPool::produceConnectionTask()
{
	for (;;)
	{
		unique_lock<mutex> lock(_queueLock);
		while (!isEmpty())
		{
			cv.wait(lock); // 队列不空，此处生产线程进入等待状态
		}
		// 连接数量没有到达上限，继续创建新的连接
		if ((int)_currentCount < _maxCount)
		{
			MysqlConnection *p = new MysqlConnection();
			p->connect(_mysqlhost, _port,_mysqluser, _mysqlpwd, _databasename);
			p->refreshAliveTime(); // 刷新一下开始空闲的起始时间
			_connQueue.push(p);
			_currentCount++;
		}

		// 通知消费者线程，可以消费连接了
		cv.notify_all();
	}
}

/*
 *判断当前MySQL连接池的是否空
 */
bool MysqlPool::isEmpty()
{
	return _connQueue.empty();
}
/*
 *获取当前连接池队列的队头
 */
MysqlConnection *MysqlPool::poolFront()
{
	return _connQueue.front();
}
/*
 *
 */
unsigned int MysqlPool::poolSize()
{
	return _connQueue.size();
}
/*
 *弹出当前连接池队列的队头
 */
void MysqlPool::poolPop()
{
	_connQueue.pop();
}
/*
 *获取连接对象，如果连接池中有连接，就取用;没有，就重新创建一个连接对象。
 *同时注意到MySQL的连接的时效性，即在连接队列中,连接对象在超过一定的时间后没有进行操作，
 *MySQL会自动关闭连接，当然还有其他原因，比如：网络不稳定，带来的连接中断。
 *所以在获取连接对象前，需要先判断连接池中连接对象是否有效。
 *考虑到数据库同时建立的连接数量有限制，在创建新连接需提前判断当前开启的连接数不超过设定值。
 */
shared_ptr<MysqlConnection> MysqlPool::getOneConnect()
{
	unique_lock<mutex> lock(_queueLock);
	while (_connQueue.empty())
	{
		// sleep
		if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
		{
			if (isEmpty())
			{
				LOG_WARN<<("get conection timeout");
				return nullptr;
			}
		}
	}
	
	/*
	shared_ptr智能指针析构时，会把connection资源直接delete掉，相当于
	调用connection的析构函数，connection就被close掉了。
	这里需要自定义shared_ptr的释放资源的方式，把connection直接归还到queue当中
	*/
	while (!isEmpty() && poolFront()->mysqlPing()) {
		poolFront()->mysqlClose();
		poolPop();
		_currentCount--;
	}
	shared_ptr<MysqlConnection> sp(poolFront(),
		[&](MysqlConnection *pcon)
	{
		//第二个参数是自定义删除器,当回收时使用。
		// 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
		unique_lock<mutex> lock(_queueLock);
		pcon->refreshAliveTime(); // 刷新一下开始空闲的起始时间
		_connQueue.push(pcon);
	});
	_connQueue.pop();
	cv.notify_all(); // 消费完连接以后，通知生产者线程检查一下，如果队列为空了，赶紧生产连接

	return sp;
}


void MysqlPool::scannerConnectionTask()
{
	/*printf("current pid = %d, tid = %d package\n",
		getpid(), CurrentThread::tid());*/
	while(1)
	{
		// 通过sleep模拟定时效果
		usleep(1000000);
		unique_lock<mutex> lock(_queueLock);
		while (!isEmpty() && poolFront()->mysqlPing()) {
			poolFront()->mysqlClose();
			poolPop();
			_currentCount--;
		}
		while (_currentCount > _initCount)
		{
			MysqlConnection *p = poolFront();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				poolPop();
				_currentCount--;
				delete p; // 调用~Connection()释放连接
			}
			else
			{
				break; // 队头的连接没有超过_maxIdleTime，其它连接肯定没有
			}
		}
	}
}

MysqlPool::~MysqlPool()
{
	while (poolSize() != 0)
	{
		poolFront()->mysqlClose();
		poolPop();
		_currentCount--;
	}
}
