#include "mysqlPool.h"


MysqlPool *MysqlPool::mysqlpool_object = NULL;
std::mutex MysqlPool::objectlock;
std::mutex MysqlPool::poollock;
//connectPool启动时先启动initCount个连接
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
	_mysqlTrans.mysqlInsert(mysql, userName, query);//再插入
	close(mysql);//归还连接
}
void MysqlPool::doGet(const string&userName, string& result)
{
	MYSQL *mysql = getOneConnect();
	_mysqlTrans.mysqlQuery(mysql, userName, result);
	close(mysql);
}
/*
 *配置数据库参数
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
 *有参的单例函数，用于第一次获取连接池对象，初始化数据库信息。
 */
MysqlPool* MysqlPool::getMysqlPoolObject(const char* mysqlhost,
	const char*   mysqluser,
	const char*   mysqlpwd,
	const char*   databasename,
	unsigned int  initcount,
	unsigned int  maxcount) {
	if (mysqlpool_object == NULL) {//双检锁机制
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
 *创建一个连接对象
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
 *判断当前MySQL连接池的是否空
 */
bool MysqlPool::isEmpty() {
	return mysqlpool.empty();
}
/*
 *获取当前连接池队列的队头
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
 *弹出当前连接池队列的队头
 */
void MysqlPool::poolPop() {
	mysqlpool.pop();
}
/*
 *获取连接对象，如果连接池中有连接，就取用;没有，就重新创建一个连接对象。
 *同时注意到MySQL的连接的时效性，即在连接队列中,连接对象在超过一定的时间后没有进行操作，
 *MySQL会自动关闭连接，当然还有其他原因，比如：网络不稳定，带来的连接中断。
 *所以在获取连接对象前，需要先判断连接池中连接对象是否有效。
 *考虑到数据库同时建立的连接数量有限制，在创建新连接需提前判断当前开启的连接数不超过设定值。
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
 *将有效的链接对象放回链接池队列中，以待下次的取用。
 */
void MysqlPool::close(MYSQL* conn) {
	if (conn != NULL) {
		poollock.lock();
		mysqlpool.push(conn);
		poollock.unlock();
	}
}
/*
 * sql语句执行函数，并返回结果，没有结果的SQL语句返回空结果，
 * 每次执行SQL语句都会先去连接队列中去一个连接对象，
 * 执行完SQL语句，就把连接对象放回连接池队列中。
 * 返回对象用map主要考虑，用户可以通过数据库字段，直接获得查询的字。
 * 例如：m["字段"][index]。
 */
 /*
  * 析构函数，将连接池队列中的连接全部关闭
  */
MysqlPool::~MysqlPool() {
	while (poolSize() != 0) {
		mysql_close(poolFront());
		poolPop();
		_currentCount--;
	}
}
