#include "mysqlJsonTrans.h"
#include "codec.h"
#include "server.h"
MysqlTrans::MysqlTrans()
{
	mysql_ = mysql_init(NULL);
	if (!mysql_real_connect(mysql_, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWD, DB_NAME, 0, NULL, 0))
	{
		printf("\nFailed to connect:%s\n", mysql_error(mysql_));
	}
	else
	{
		printf("\nConnect sucessfully!\n");
	}
}
MysqlTrans::~MysqlTrans()
{
	mysql_close(mysql_);
}
void MysqlTrans::setUtf8(MYSQL* mysql, char *query)
{
	strcpy(query, "set names \'utf8\'");
	mysql_query(mysql, query);
}
//void MysqlTrans::mysqlInsert(const string&userName,const string &content)
//{
//	int t;
//	char queryUtf8[48];
//	char head[] = "INSERT INTO";
//	char query[1024];
//	char field[48] = "UserName,content";
//	char left[] = "(";
//	char right[] = ") ";
//	char values[] = "VALUES";
//	char message[1000] = { 0 };
//	setUtf8(mysql_, queryUtf8);
//	sprintf(message, "\'%s\',\'%s\'",userName.c_str(),content.c_str());
//
//	/* 拼接sql命令 */
//	sprintf(query, "%s %s%s%s%s%s%s%s%s", head, TABLE_NAME, left, field, right, values, left, message, right);
//	
//	t = mysql_real_query(mysql_, query, strlen(query));
//	if (t)
//	{
//		printf("Failed to Insert: %s\n", mysql_error(mysql_));
//	}
//	else
//	{
//		printf("\nInsert sucessfully!\n");
//	}
//	cout << query << endl;
//}

void MysqlTrans::mysqlInsert(const string&userName, const string &content)
{
	int t;
	string query;
	char queryUtf8[48];
	setUtf8(mysql_, queryUtf8);
	stringstream ss;
	string left("(");
	string right(") ");
	ss << string("INSERT INTO ");
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
	t = mysql_real_query(mysql_, query.c_str(), query.size());
	if (t)
	{
		printf("Failed to Insert: %s\n", mysql_error(mysql_));
	}
	else
	{
		printf("\nInsert successfully!\n");
	}
}
//void MysqlTrans::mysqlReplace(const string&userName, const string &content)
//{
//	int t;
//	char queryUtf8[48];
//	char head[] = "REPLACE INTO";
//	char query[1024];
//	char field[48] = "UserName,content";
//	char left[] = "(";
//	char right[] = ") ";
//	char values[] = "VALUES";
//	char message[1000] = { 0 };
//	setUtf8(mysql_, queryUtf8);
//	sprintf(message, "\'%s\',\'%s\'", userName.c_str(), content.c_str());
//
//	/* 拼接sql命令 */
//	sprintf(query, "%s %s%s%s%s%s%s%s%s", head, TABLE_NAME, left, field, right, values, left, message, right);
//
//	t = mysql_real_query(mysql_, query, strlen(query));
//	if (t)
//	{
//		printf("Failed to Insert: %s\n", mysql_error(mysql_));
//	}
//	else
//	{
//		printf("\n Insert sucessfully!\n");
//	}
//}
void MysqlTrans::mysqlDelete(const string &userName)
{
	int t;
	char head[] = "DELETE FROM ";
	char query[120];
	char queryName[] = "UserName";
	sprintf(query, "%s %s where %s =\"%s\"", head, TABLE_NAME, queryName, userName.c_str());
	t = mysql_real_query(mysql_, query, strlen(query));
	if (t)
	{
		printf("\nFailed to delete: %s\n", mysql_error(mysql_));
	}
	else
	{
		printf("\nDelete data successfully!\n");
	}
	increaseOne();
}
void MysqlTrans::increaseOne()
{
	int t;
	stringstream ss;
	ss << string("ALTER TABLE ") << TABLE_NAME;
	ss << string(" AUTO_INCREMENT =1");
	string query = ss.str();
	t = mysql_real_query(mysql_, query.c_str(), query.size());
	if (t)
	{
		printf("\nAlter auto_increment failer! \n");
	}	
	else
	{
		printf("\nAlter auto_increment successfully! \n");
	}
}
//void MysqlTrans::mysqlDeleteAll()
//{
//	int t;
//	char head[] = "DELETE FROM ";
//	char query[120];
//
//	sprintf(query, "%s %s", head, TABLE_NAME);
//	printf("%s\n", query);
//
//	t = mysql_real_query(mysql_, query, strlen(query));
//	if (t)
//	{
//		printf("\nFailed to delete all: %s\n", mysql_error(mysql_));
//	}
//	else
//	{
//		printf("\nDelete all data sucessfully!\n");
//	}
//}
//void MysqlTrans::mysqlUpdate(int id, string content)
//{
//	int t;
//	char head[] = "UPDATE ";
//	char query[100];
//
//	sprintf(query, "%s%s SET %s=%d", head, TABLE_NAME, "id", id);
//	printf("%s\n", query);
//
//	t = mysql_real_query(mysql_, query, strlen(query));
//	if (t)
//	{
//		printf("Failed to update: %s\n", mysql_error(mysql_));
//		return;
//	}
//	printf("\nUpdate data sucessfully!\n");
//}
void MysqlTrans::mysqlQuery(const string &userName, string &getRes)
{
	int t;
	char queryUtf8[48];
	char head[] = "SELECT content FROM ";
	char query[64] = { 0 };
	MYSQL_RES *res;
	MYSQL_ROW row;
	setUtf8(mysql_, queryUtf8);
	sprintf(query, "%s %s where UserName = \"%s\"", head, TABLE_NAME, userName.c_str());

	t = mysql_real_query(mysql_, query, strlen(query));
	if (t)
	{
		printf("Failed to query: %s\n", mysql_error(mysql_));
		return;
	}
	else
	{
		res = mysql_store_result(mysql_);
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

//void MysqlTrans::mysqlQueryAll()
//{
//	string str;
//	int t;
//	char head[] = "SELECT * FROM ";
//	char query[50] = { 0 };
//	MYSQL_RES *res;
//	MYSQL_ROW row;
//
//	sprintf(query, "%s%s", head, TABLE_NAME);
//
//	t = mysql_real_query(mysql_, query, strlen(query));
//
//	if (t)
//	{
//		printf("Failed to query: %s\n", mysql_error(mysql_));
//		return;
//	}
//	else
//	{
//		printf("\nQuery successfully!\n");
//	}
//
//	res = mysql_store_result(mysql_);
//	while ((row = mysql_fetch_row(res)))
//	{
//		for (t = 0; t < (int)mysql_num_fields(res); t++)
//		{
//			if (t == 1)
//			{
//				str = row[t];
//				strVec_.push_back(str);
//			}
//		}
//	}
//}
//void MysqlTrans::inputMysql(const string &msg)//将字符串转成一条条的json插入数据库
//{
//	Json::Reader reader;
//	Json::Value root;
//	if (!reader.parse(msg, root))
//	{
//		return;
//	}
//	MYSQL *mysql = mysql_init(NULL);
//	if (!mysql)
//	{
//		printf("\nMysql init failed.\n");
//	}
//
//	string str = root.toStyledString();
//	cout << str << endl;
//	//mysqlInsert(str);
//	
//}
//
//void MysqlTrans::outPutMysql(string &msg)//将查询Mysql的结果一条条的json合成一个数组json并形成字符串。
//{
//	string str;
//	if (!mysql_)
//	{
//		printf("\nMysql init failed.\n");
//	}
//	mysqlQueryAll();
//	size_t n = strVec_.size();
//	Json::Value root;
//	Json::Value arr;
//	Json::Reader reader;
//	ofstream ofs("output.json", fstream::out);
//	for (size_t i = 0; i < n; i++)
//	{
//		str = strVec_[i];
//		if (reader.parse(str, root))
//		{
//			arr.append(Json::Value(root));
//		}
//	}
//	strVec_.clear();
//	msg = arr.toStyledString();
//}

