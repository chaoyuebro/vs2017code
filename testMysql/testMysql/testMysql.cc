
/*
 * =====================================================================================
 *
 *       Filename:  mysql_in_c.c
 *       Description:
 *       Version:   1.0
 *       Created:   2018年07月23日 13时58分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *       Author:    xugaoxiang , djstava@gmail.com
 *
 * =====================================================================================
 */

#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <mysql/mysql.h>  

#define MYSQL_HOST    "localhost"
#define MYSQL_USER    "root"
#define MYSQL_PASSWD  "chinese86"
#define DB_NAME "test_db"
#define TABLE_NAME "students"

 /*
  * 测试的数据表结构很简单, number, grade, sex, score 四个字段, 代表学号、年级、性别、分数
 */

 /* 连接mysql */
void mysqldb_connect(MYSQL *mysql)
{
	if (!mysql_real_connect(mysql, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWD, DB_NAME, 0, NULL, 0)) {
		printf("\nFailed to connect:%s\n", mysql_error(mysql));
	}
	else {
		printf("\nConnect sucessfully!\n");
	}
}

/* 插入数据 */
void mysqldb_insert(MYSQL *mysql, int number, int grade, char *sex, int score)
{
	int t;
	char *head = "INSERT INTO";
	char query[120];
	char field[48] = "number, grade, sex, score";
	char *left = "(";
	char *right = ") ";
	char *values = "VALUES";
	char message[100] = { 0 };

	sprintf(message, "%d, %d, \"%s\", %d", number, grade, sex, score);

	/* 拼接sql命令 */
	sprintf(query, "%s %s%s%s%s%s%s%s%s", head, TABLE_NAME, left, field, right, values, left, message, right);
	printf("%s\n", query);

	t = mysql_real_query(mysql, query, strlen(query));
	if (t) {
		printf("Failed to query: %s\n", mysql_error(mysql));
	}
	else {
		printf("\nInsert sucessfully!\n");
	}

}

/* 删除数据 */
void mysqldb_delete(MYSQL *mysql, char *field_name, int number)
{
	int t;
	char *head = "DELETE FROM ";
	char query[120];

	sprintf(query, "%s%s where %s =%d", head, TABLE_NAME, field_name, number);
	printf("%s\n", query);

	t = mysql_real_query(mysql, query, strlen(query));
	if (t) {
		printf("\nFailed to query: %s\n", mysql_error(mysql));
	}
	else {
		printf("\nDelete data sucessfully!\n");
	}

}

/* 更新数据 */
void mysqldb_update(MYSQL *mysql, char *field_name, int score)
{
	int t;
	char *head = "UPDATE ";
	char query[100];

	sprintf(query, "%s%s SET %s=%d", head, TABLE_NAME, field_name, score);
	printf("%s\n", query);

	t = mysql_real_query(mysql, query, strlen(query));
	if (t) {
		printf("Failed to update: %s\n", mysql_error(mysql));
		return;
	}
	printf("\nUpdate data sucessfully!\n");
}

/* 查询数据 */
void mysqldb_query(MYSQL *mysql)
{
	int t;
	char *head = "SELECT * FROM ";
	char query[50] = { 0 };
	MYSQL_RES *res;
	MYSQL_ROW row;

	sprintf(query, "%s%s", head, TABLE_NAME);

	t = mysql_real_query(mysql, query, strlen(query));

	if (t) {
		printf("Failed to query: %s\n", mysql_error(mysql));
		return;
	}
	else {
		printf("\nQuery successfully!\n");
	}

	res = mysql_store_result(mysql);
	while (row = mysql_fetch_row(res)) {
		for (t = 0; t < mysql_num_fields(res); t++) {
			printf("%s\t", row[t]);
		}
		printf("\n");
	}
	mysql_free_result(res);
}

/* 断开mysql连接 */
void close_connection(MYSQL *mysql)
{
	mysql_close(mysql);
}

int main(int argc, char *argv[])
{
	// 准备一组数据
	int number = 1;
	int grade = 1;
	char sex[] = "male";
	int score = 100;

	// 初始化mysql
	MYSQL *mysql = mysql_init(NULL);
	if (!mysql) {
		printf("\nMysql init failed.\n");
	}

	// 连接MYSQL
	mysqldb_connect(mysql);

	// 插入数据
	mysqldb_insert(mysql, number, grade, sex, score);

	//// 更新数据
	//mysqldb_update(mysql, "score", 99);

	//// 查询数据
	//mysqldb_query(mysql);

	//// 删除数据
	//mysqldb_delete(mysql, "number", number);

	// 断开连接
	close_connection(mysql);

	return 0;
}
