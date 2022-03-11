#include "mysqlConnection.h"

MysqlConnection::MysqlConnection()
{
	// 初始化数据库连接
	_conn = mysql_init(nullptr);
}
MysqlConnection::~MysqlConnection()
{
	// 释放数据库连接资源
	if (_conn != nullptr)
		mysql_close(_conn);
}
bool MysqlConnection::connect(string ip,
	string username, string password, string dbname)
{
	// 连接数据库
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), 0, NULL, 0);
	return p != nullptr;
}
void MysqlConnection::setUtf8(char *query)
{
	strcpy(query, "set names \'utf8\'");
	mysql_query(_conn, query);
}

void MysqlConnection::mysqlInsert(const string&userName, const string &content)
{
	int t;
	string query;
	stringstream ss;
	string left("(");
	string right(") ");
	ss << string("REPLACE INTO ");
	ss << TABLE_NAME;
	ss << left;
	ss << string("UserName,content");
	ss << right;
	ss << string("VALUES");
	ss << left;
	ss << string("\"") << userName;
	ss << string("\",\'");
	ss << content << string("\'");
	ss << right;
	char queryUtf8[48];
	setUtf8(queryUtf8);
	query = ss.str();
	/* 拼接sql命令 */
	t = mysql_real_query(_conn, query.c_str(), query.size());
	if (t)
	{
		printf("Failed to Insert: %s\n", mysql_error(_conn));
	}
	else
	{
		//printf("\nInsert successfully!\n");
	}
}

void MysqlConnection::mysqlDelete(const string &userName)
{
	int t;
	char head[] = "DELETE FROM ";
	char query[120];
	char queryName[] = "UserName";
	sprintf(query, "%s %s where %s =\"%s\"", head, TABLE_NAME, queryName, userName.c_str());
	t = mysql_real_query(_conn, query, strlen(query));
	if (t)
	{
		printf("\nFailed to delete: %s\n", mysql_error(_conn));
	}
	else
	{
		//printf("\nDelete data successfully!\n");
	}
	//increaseOne(mysql);
}
// void MysqlConnection::increaseOne(MYSQL* mysql)
// {
// 	int t;
// 	stringstream ss;
// 	ss << string("ALTER TABLE ") << TABLE_NAME;
// 	ss << string(" AUTO_INCREMENT =1");
// 	string query = ss.str();
// 	t = mysql_real_query(mysql, query.c_str(), query.size());
// 	if (t)
// 	{
// 		printf("\nAlter auto_increment failer! \n");
// 	}	
// 	else
// 	{
// 		printf("\nAlter auto_increment successfully! \n");
// 	}
// }

void MysqlConnection::mysqlQuery(const string &userName, string &getRes)
{
	int t;
	char head[] = "SELECT content FROM ";
	char query[64] = { 0 };
	MYSQL_RES *res;
	MYSQL_ROW row;
	sprintf(query, "%s %s where UserName = \"%s\"", head, TABLE_NAME, userName.c_str());

	t = mysql_real_query(_conn, query, strlen(query));
	if (t)
	{
		printf("Failed to query: %s\n", mysql_error(_conn));
		return;
	}
	else
	{
		res = mysql_store_result(_conn);
		row = mysql_fetch_row(res);
		if (row == NULL)
		{
			printf("User did't exist\n");
			getRes = "FAIL";//如果查询失败返回fail
			return;
		}
		else
		{
			//printf("\nQuery successfully!\n");
		}
	}

	getRes = row[0];
	mysql_free_result(res);

}
bool MysqlConnection::mysqlPing()
{
	return mysql_ping(_conn);
}
void MysqlConnection::mysqlClose()
{
	mysql_close(_conn);
}

