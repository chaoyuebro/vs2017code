#include"./JsonTrans.h"
#include"./codec.h"

void JsonTrans::inputJson(string &str)
{
	ifstream ifs("input.json", ios::binary);
	if (!ifs.is_open())
	{
		cout << "Error opening file\n";
		return;
	}

	if (!reader_.parse(ifs, root_))
	{
		cout << "error input" << endl;
		return;
	}
	str = root_.toStyledString();
	// ²åÈëÊý¾Ý
}

//void JsonTrans::outputJson()
//{
//
//	string str;
//	MYSQL *mysql = mysql_init(NULL);
//	if (!mysql)
//	{
//		printf("\nMysql init failed.\n");
//	}
//	mysqldb_connect(mysql);
//	mysqldb_query_all(mysql);
//	int n = strVec.size();
//	Json::Value root;
//	Json::Value arr;
//	Json::Reader reader;
//	ofstream ofs("output.json", fstream::out);
//	for (int i = 0; i < n; i++)
//	{
//		str = strVec[i];
//		if (reader.parse(str, root))
//		{
//			arr.append(Json::Value(root));
//		}
//	}
//	Json::StyledWriter styWriter;
//	std::string strWriter = styWriter.write(arr);
//	ofs << strWriter;
//}
