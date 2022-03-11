#include "mysqlJsonTrans.h"

MysqlTrans::MysqlTrans()
{
}
MysqlTrans::~MysqlTrans()
{
}
void MysqlTrans::setUtf8(MYSQL* mysql, char *query)
{
	strcpy(query, "set names \'utf8\'");
	mysql_query(mysql, query);
}

void MysqlTrans::mysqlInsert(MYSQL *mysql, const string&userName, const string &content)
{
	int t;
	string query;
	char queryUtf8[48];
	setUtf8(mysql, queryUtf8);
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

	query = ss.str();
	/* 拼接sql命令 */
	t = mysql_real_query(mysql, query.c_str(), query.size());
	if (t)
	{
		printf("Failed to Insert: %s\n", mysql_error(mysql));
	}
	else
	{
		printf("\nInsert successfully!\n");
	}
}

void MysqlTrans::mysqlDelete(MYSQL *mysql, const string &userName)
{
	int t;
	char head[] = "DELETE FROM ";
	char query[120];
	char queryName[] = "UserName";
	sprintf(query, "%s %s where %s =\"%s\"", head, TABLE_NAME, queryName, userName.c_str());
	t = mysql_real_query(mysql, query, strlen(query));
	if (t)
	{
		printf("\nFailed to delete: %s\n", mysql_error(mysql));
	}
	else
	{
		printf("\nDelete data successfully!\n");
	}
	//increaseOne(mysql);
}
//void MysqlTrans::increaseOne(MYSQL* mysql)
//{
//	int t;
//	stringstream ss;
//	ss << string("ALTER TABLE ") << TABLE_NAME;
//	ss << string(" AUTO_INCREMENT =1");
//	string query = ss.str();
//	t = mysql_real_query(mysql, query.c_str(), query.size());
//	if (t)
//	{
//		printf("\nAlter auto_increment failer! \n");
//	}
//	else
//	{
//		printf("\nAlter auto_increment successfully! \n");
//	}
//}

void MysqlTrans::mysqlQuery(MYSQL *mysql, const string &userName, string &getRes)
{
	int t;
	char queryUtf8[48];
	char head[] = "SELECT content FROM ";
	char query[64] = { 0 };
	MYSQL_RES *res;
	MYSQL_ROW row;
	setUtf8(mysql, queryUtf8);
	sprintf(query, "%s %s where UserName = \"%s\"", head, TABLE_NAME, userName.c_str());

	t = mysql_real_query(mysql, query, strlen(query));
	if (t)
	{
		printf("Failed to query: %s\n", mysql_error(mysql));
		return;
	}
	else
	{
		res = mysql_store_result(mysql);
		row = mysql_fetch_row(res);
		if (row == NULL)
		{
			printf("User did't exist\n");
			getRes = "FAIL";//如果查询失败返回fail
			return;
		}
		else
		{
			printf("\nQuery successfully!\n");
		}
	}
	getRes = row[0];
	mysql_free_result(res);

}


