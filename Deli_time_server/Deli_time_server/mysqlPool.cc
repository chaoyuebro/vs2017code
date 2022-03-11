#include "mysqlPool.h"

shared_ptr<MysqlPool> MysqlPool::mysqlpool_object = NULL;
std::mutex MysqlPool::_queueLock;
std::mutex MysqlPool::_poolLock;
// connectPool����ʱ������initCount������
MysqlPool::MysqlPool()
{
	// ������������
	if (!loadConfigFile())
	{
		return;
	}

	// ������ʼ����������
	for (int i = 0; i < _initCount; ++i)
	{
		MysqlConnection *p = new MysqlConnection();
		p->connect(_mysqlhost, _mysqluser, _mysqlpwd, _databasename);
		p->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
		_connQueue.push(p);
		_currentCount++;
	}

	// ����һ���µ��̣߳���Ϊ���ӵ������� linux thread => pthread_create
	thread produce(std::bind(&MysqlPool::produceConnectionTask, this));
	produce.detach();

	// ����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
	thread scanner(std::bind(&MysqlPool::scannerConnectionTask, this));
	scanner.detach();
}

/*
 *�������ݿ����
 */
bool MysqlPool::loadConfigFile()
{

	string filename("mysql.conf");
	ifstream ifs(filename);
	if (!ifs)
	{
		cout << "open file error!" << endl;
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
			cout << "wrong conf format" << endl;
			return false;
		}
	}
	return true;
}
/*
 *�вεĵ������������ڵ�һ�λ�ȡ���ӳض��󣬳�ʼ�����ݿ���Ϣ��
 */
MysqlPool* MysqlPool::getMysqlPoolObject()
{
	if (mysqlpool_object == NULL)
	{ //˫��������
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
			cv.wait(lock); // ���в��գ��˴������߳̽���ȴ�״̬
		}

		// ��������û�е������ޣ����������µ�����
		if ((int)_currentCount < _maxCount)
		{
			MysqlConnection *p = new MysqlConnection();
			p->connect(_mysqlhost, _mysqluser, _mysqlpwd, _databasename);
			p->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
			_connQueue.push(p);
			_currentCount++;
		}

		// ֪ͨ�������̣߳���������������
		cv.notify_all();
	}
}

/*
 *�жϵ�ǰMySQL���ӳص��Ƿ��
 */
bool MysqlPool::isEmpty()
{
	return _connQueue.empty();
}
/*
 *��ȡ��ǰ���ӳض��еĶ�ͷ
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
 *������ǰ���ӳض��еĶ�ͷ
 */
void MysqlPool::poolPop()
{
	_connQueue.pop();
}
/*
 *��ȡ���Ӷ���������ӳ��������ӣ���ȡ��;û�У������´���һ�����Ӷ���
 *ͬʱע�⵽MySQL�����ӵ�ʱЧ�ԣ��������Ӷ�����,���Ӷ����ڳ���һ����ʱ���û�н��в�����
 *MySQL���Զ��ر����ӣ���Ȼ��������ԭ�򣬱��磺���粻�ȶ��������������жϡ�
 *�����ڻ�ȡ���Ӷ���ǰ����Ҫ���ж����ӳ������Ӷ����Ƿ���Ч��
 *���ǵ����ݿ�ͬʱ�������������������ƣ��ڴ�������������ǰ�жϵ�ǰ�������������������趨ֵ��
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
				LOG("get conection timeout");
				return nullptr;
			}
		}
	}

	/*
	shared_ptr����ָ������ʱ�����connection��Դֱ��delete�����൱��
	����connection������������connection�ͱ�close���ˡ�
	������Ҫ�Զ���shared_ptr���ͷ���Դ�ķ�ʽ����connectionֱ�ӹ黹��queue����
	*/
	shared_ptr<MysqlConnection> sp(poolFront(),
		[&](MysqlConnection *pcon)
	{
		//�ڶ����������Զ���ɾ����,������ʱʹ�á�
		// �������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
		unique_lock<mutex> lock(_queueLock);
		pcon->refreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
		_connQueue.push(pcon);
	});

	_connQueue.pop();
	cv.notify_all(); // �����������Ժ�֪ͨ�������̼߳��һ�£��������Ϊ���ˣ��Ͻ���������

	return sp;
}


void MysqlPool::scannerConnectionTask()
{
	while(1)
	{
		// ͨ��sleepģ�ⶨʱЧ��
		this_thread::sleep_for(chrono::seconds(_maxIdleTime));
		unique_lock<mutex> lock(_queueLock);
		while (!isEmpty() && poolFront()->mysqlPing()) {
			
			poolFront()->mysqlClose();
			poolPop();
			_queueLock.unlock();
			_currentCount--;
		}
		// ɨ���������У��ͷŶ��������
		while (_currentCount > _initCount)
		{
			MysqlConnection *p = _connQueue.front();
			if (p->getAliveeTime() >= (_maxIdleTime * 1000))
			{
				_connQueue.pop();
				_currentCount--;
				delete p; // ����~Connection()�ͷ�����
			}
			else
			{
				break; // ��ͷ������û�г���_maxIdleTime���������ӿ϶�û��
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
