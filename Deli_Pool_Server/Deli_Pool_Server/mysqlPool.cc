#include "mysqlPool.h"


MysqlPool *MysqlPool::mysqlpool_object = NULL;
std::mutex MysqlPool::objectlock;
std::mutex MysqlPool::poollock;
//connectPool����ʱ������initCount������
MysqlPool::MysqlPool(
	const char* mysqlhost,
	const char*   mysqluser,
	const char*   mysqlpwd,
	const char*   databasename,
	unsigned int  initcount,
	unsigned int  maxcount = 50
)
	: _mysqlhost(mysqlhost),
	_mysqluser(mysqluser),
	_mysqlpwd(mysqlpwd),
	_databasename(databasename),
	_initCount(initcount),
	_maxCount(maxcount),
	_currentCount(0)
{
	for (int i = 0; i < (int)_initCount; i++)
	{
		MYSQL* conn = createOneConnect();
		_currentCount++;
		mysqlpool.push(conn);
	}

}
void MysqlPool::doPut(const string& userName, const string& query)
{
	MYSQL *mysql = getOneConnect();
	_mysqlTrans.mysqlInsert(mysql, userName, query);//�ٲ���
	close(mysql);//�黹����
}
void MysqlPool::doGet(const string&userName, string& result)
{
	MYSQL *mysql = getOneConnect();
	_mysqlTrans.mysqlQuery(mysql, userName, result);
	close(mysql);
}
/*
 *�������ݿ����
 */
void MysqlPool::setParameter(const char* mysqlhost,
	const char*   mysqluser,
	const char*   mysqlpwd,
	const char*   databasename,
	unsigned int  maxConnect) {
	_mysqlhost = mysqlhost;
	_mysqluser = mysqluser;
	_mysqlpwd = mysqlpwd;
	_databasename = databasename;
	_maxCount = maxConnect;
}

/*
 *�вεĵ������������ڵ�һ�λ�ȡ���ӳض��󣬳�ʼ�����ݿ���Ϣ��
 */
MysqlPool* MysqlPool::getMysqlPoolObject(const char* mysqlhost,
	const char*   mysqluser,
	const char*   mysqlpwd,
	const char*   databasename,
	unsigned int  initcount,
	unsigned int  maxcount) {
	if (mysqlpool_object == NULL) {//˫��������
		objectlock.lock();
		if (mysqlpool_object == NULL) {
			mysqlpool_object = new MysqlPool(mysqlhost,
				mysqluser, mysqlpwd, databasename, initcount,
				maxcount);
		}
		objectlock.unlock();
	}
	return mysqlpool_object;
}

/*
 *����һ�����Ӷ���
 */
MYSQL* MysqlPool::createOneConnect() {
	MYSQL* conn = NULL;
	conn = mysql_init(conn);
	if (conn != NULL) {
		if (mysql_real_connect(conn,
			_mysqlhost,
			_mysqluser,
			_mysqlpwd,
			_databasename, 0, NULL, 0
		)) {
			_currentCount++;
			return conn;
		}
		else {
			std::cout << mysql_error(conn) << std::endl;
			return NULL;
		}
	}
	else {
		std::cerr << "init failed" << std::endl;
		return NULL;
	}
}

/*
 *�жϵ�ǰMySQL���ӳص��Ƿ��
 */
bool MysqlPool::isEmpty() {
	return mysqlpool.empty();
}
/*
 *��ȡ��ǰ���ӳض��еĶ�ͷ
 */
MYSQL* MysqlPool::poolFront() {
	return mysqlpool.front();
}
/*
 *
 */
unsigned int MysqlPool::poolSize() {
	return (unsigned)mysqlpool.size();
}
/*
 *������ǰ���ӳض��еĶ�ͷ
 */
void MysqlPool::poolPop() {
	mysqlpool.pop();
}
/*
 *��ȡ���Ӷ���������ӳ��������ӣ���ȡ��;û�У������´���һ�����Ӷ���
 *ͬʱע�⵽MySQL�����ӵ�ʱЧ�ԣ��������Ӷ�����,���Ӷ����ڳ���һ����ʱ���û�н��в�����
 *MySQL���Զ��ر����ӣ���Ȼ��������ԭ�򣬱��磺���粻�ȶ��������������жϡ�
 *�����ڻ�ȡ���Ӷ���ǰ����Ҫ���ж����ӳ������Ӷ����Ƿ���Ч��
 *���ǵ����ݿ�ͬʱ�������������������ƣ��ڴ�������������ǰ�жϵ�ǰ�������������������趨ֵ��
 */
MYSQL* MysqlPool::getOneConnect() {
	cout << "getOneConnect" << endl;
	poollock.lock();
	MYSQL *conn = NULL;
	if (!isEmpty()) {
		while (!isEmpty() && mysql_ping(poolFront())) {
			mysql_close(poolFront());
			poolPop();
			_currentCount--;
		}
		if (!isEmpty()) {
			conn = poolFront();
			poolPop();
		}
		else {
			if (_currentCount < (int)_maxCount)
				conn = createOneConnect();
			else
				std::cerr << "the number of mysql connections is too much!" << std::endl;
		}
	}
	else {
		if (_currentCount < (int)_maxCount)
			conn = createOneConnect();
		else
			std::cerr << "the number of mysql connections is too much!" << std::endl;
	}
	poollock.unlock();
	return conn;
}
/*
 *����Ч�����Ӷ���Ż����ӳض����У��Դ��´ε�ȡ�á�
 */
void MysqlPool::close(MYSQL* conn) {
	if (conn != NULL) {
		poollock.lock();
		mysqlpool.push(conn);
		poollock.unlock();
	}
}
/*
 * sql���ִ�к����������ؽ����û�н����SQL��䷵�ؿս����
 * ÿ��ִ��SQL��䶼����ȥ���Ӷ�����ȥһ�����Ӷ���
 * ִ����SQL��䣬�Ͱ����Ӷ���Ż����ӳض����С�
 * ���ض�����map��Ҫ���ǣ��û�����ͨ�����ݿ��ֶΣ�ֱ�ӻ�ò�ѯ���֡�
 * ���磺m["�ֶ�"][index]��
 */
 /*
  * ���������������ӳض����е�����ȫ���ر�
  */
MysqlPool::~MysqlPool() {
	while (poolSize() != 0) {
		mysql_close(poolFront());
		poolPop();
		_currentCount--;
	}
}
