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
bool MysqlConnection::connect(string ip,int port,
	string username, string password, string dbname)
{
	// 连接数据库
	MYSQL *p = mysql_real_connect(_conn, ip.c_str(), username.c_str(),
		password.c_str(), dbname.c_str(), port, NULL, 0);
	return p != NULL;
}


void MysqlConnection::mysqlInsert(const string&userName, const string &content)
{
	int t; 
	char *query=new char[content.length()+200];
	char *end;
	memset(query, 0, content.length()+200);
	sprintf(query, "REPLACE INTO %s(UserName,content) VALUES (\"%s\",\'",TABLE_NAME,userName.c_str());
	end = query + strlen(query);
	end += mysql_real_escape_string(_conn, end, content.c_str(), content.length());
	*end++ = '\'';
	*end++ = ')';
	/* 拼接sql命令 */
	t = mysql_real_query(_conn, query,(unsigned int)(end-query));
	
	if (t)
	{
		LOG_WARN << "Failed to Insert" << mysql_error(_conn);
	}
	else
	{
		//printf("\nInsert successfully!\n");
	}
	delete[] query;
}
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
		LOG_WARN << "Failed to query" << mysql_error(_conn);
		return;
	}
	else
	{
		res = mysql_store_result(_conn);
		row = mysql_fetch_row(res);
		if (row == NULL)
		{
			LOG_INFO<<"User did't exist\n";
			getRes = "FAIL";//如果查询失败返回fail
			return;
		}
		else
		{
			//printf("\nQuery successfully!\n");
		}
	}
	int num_fields = mysql_num_fields(res);
	unsigned long *lengths = mysql_fetch_lengths(res);
	uint32_t *destIDs;
	destIDs = (uint32_t *)malloc(lengths[0]);
	if (destIDs == NULL)
	{
		printf("malloc error!\n");
		exit(1);
	}
	memcpy(destIDs, row[0], lengths[0]);
	getRes = string((char *)destIDs,lengths[0]);
	mysql_free_result(res);
	free(destIDs);
}
bool MysqlConnection::mysqlPing()
{
	return mysql_ping(_conn);
}
void MysqlConnection::mysqlClose()
{
	mysql_close(_conn);
}

