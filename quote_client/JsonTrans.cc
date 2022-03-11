#include"./JsonTrans.h"
#include"./codec.h"
JsonTrans::JsonTrans()
	:localStr_(string())
	,mysqlStr_(string())
{

}
void JsonTrans::setLocalStr(string localStr)
{
	localStr_ = localStr;
}
void JsonTrans::setMysqlStr(string mysqlStr)
{
	mysqlStr_ = mysqlStr;
}
void JsonTrans::inputJson(string& str)
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
	localStr_ = root_.toStyledString();
	str = localStr_;
	cout << str << endl;
	// ²åÈëÊý¾Ý
}

void JsonTrans::coverJson(string &str)
{
	Json::Value root;
	Json::Reader reader;
	ofstream ofs("output.json", fstream::out);
	if (!reader.parse(str, root))
	{
		return;
	}
	Json::StyledWriter styWriter;
	std::string strWriter = styWriter.write(root);
	ofs << strWriter;
}

void JsonTrans::mergeJson(string &str)
{
	Json::Value root;
	Json::Value arr;
	Json::Value root1;
	Json::Value root2;
	Json::Reader reader;
	if (reader.parse(localStr_, root1)&&reader.parse(mysqlStr_,root2))
	{
		int size1 = root1.size();
		int size2 = root2.size();
		for (int i = 0; i < size1; i++)
		{
			int id = root1[i].get("key1", "ASCII").asInt();
			string str = root1[i].toStyledString();
			mergeMap_.insert({ id, str });
		}
		for (int i = 0; i < size2; i++)
		{
			int id = root2[i].get("key1", "ASCII").asInt();
			string str = root2[i].toStyledString();
			mergeMap_.insert({ id, str });
		}
		for (auto &item : mergeMap_)
		{
			str = item.second;
			if (reader.parse(str, root))
			{
				arr.append(Json::Value(root));
			}
		}
		str = arr.toStyledString();
	}
	else if (reader.parse(mysqlStr_, root1))
	{
		str = mysqlStr_;
	}
	coverJson(str);
}
