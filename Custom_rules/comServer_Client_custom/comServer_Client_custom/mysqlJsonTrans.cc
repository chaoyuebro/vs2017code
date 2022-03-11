#include"./mysqlJsonTrans.h"
#include"./codec.h"
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
void MysqlTrans::mysqlInsert(int id, string content)
{
	int t;
	char *head = "INSERT INTO";
	char query[120];
	char field[48] = "id,content";
	char *left = "(";
	char *right = ") ";
	char *values = "VALUES";
	char message[100] = { 0 };

	sprintf(message, "%d, \'%s\'", id, content.c_str());

	/* 拼接sql命令 */
	sprintf(query, "%s %s%s%s%s%s%s%s%s", head, TABLE_NAME, left, field, right, values, left, message, right);

	t = mysql_real_query(mysql_, query, strlen(query));
	if (t)
	{
		printf("Failed to Insert: %s\n", mysql_error(mysql_));
	}
	else
	{
		printf("\n Insert sucessfully!\n");
	}
}
void MysqlTrans::mysqlDelete(int id)
{
	int t;
	char *head = "DELETE FROM ";
	char query[120];

	sprintf(query, "%s %s where \'%s\' =%d", head, TABLE_NAME,"id",id);
	printf("%s\n", query);

	t = mysql_real_query(mysql_, query, strlen(query));
	if (t)
	{
		printf("\nFailed to delete: %s\n", mysql_error(mysql_));
	}
	else
	{
		printf("\nDelete data sucessfully!\n");
	}
}
void MysqlTrans::mysqlUpdate(int id, string content)
{
	int t;
	char *head = "UPDATE ";
	char query[100];

	sprintf(query, "%s%s SET %s=%d", head, TABLE_NAME, "id", id);
	printf("%s\n", query);

	t = mysql_real_query(mysql_, query, strlen(query));
	if (t)
	{
		printf("Failed to update: %s\n", mysql_error(mysql_));
		return;
	}
	printf("\nUpdate data sucessfully!\n");
}
void MysqlTrans::mysqlQuery(int id,string &str)
{
	int t;
	char *head = "SELECT * FROM ";
	char query[50] = { 0 };
	MYSQL_RES *res;
	MYSQL_ROW row;

	sprintf(query, "%s%s", head, TABLE_NAME);

	t = mysql_real_query(mysql_, query, strlen(query));

	if (t)
	{
		printf("Failed to query: %s\n", mysql_error(mysql_));
		return;
	}
	else
	{
		printf("\nQuery successfully!\n");
	}

	res = mysql_store_result(mysql_);
	while (row = mysql_fetch_row(res))
	{
		for (t = 0; t < mysql_num_fields(res); t++)
		{
			printf("%s\t", row[t]);
			if (t == 1)
			{
				cout << row[t] << endl;
				str = row[t];
			}
		}
		printf("\n");
	}
	mysql_free_result(res);
}

void MysqlTrans::inputJson(string &str)
{
	Json::Reader reader;
	Json::Value root;
	ifstream ifs("input.json", ios::binary);
	if (!ifs.is_open())
	{
		cout << "Error opening file\n";
		return;
	}

	if (reader.parse(ifs, root))
	{
		int id = root["key1"].asInt();
		root.toStyledString();
		str = root.toStyledString();
		if (!mysql_)
		{
			printf("\nMysql init failed.\n");
		}
		// 插入数据
	}
}
