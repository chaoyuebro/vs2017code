#include <iostream>    //读写io c++标准库
#include <fstream>     //读写文件 c++标准库
#include <string>      //字符串类 c++标准库
#include <sstream>     //字符串流 c++标准库
#include <json/json.h> //jsoncpp的头文件
#include <string.h>
using namespace std;
void output(string);
void input()
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
		root.toStyledString();
		string str = root.toStyledString();
		cout << str << endl;
		output(str);
	}
}

void output(string str)
{
	Json::Value root;
	Json::Reader reader;
	if (reader.parse(str, root))
	{
		Json::StyledWriter styWriter;
		std::string str2 = styWriter.write(root);
		cout << str2 << endl;
		ofstream ofs("output.json", fstream::out);
		ofs << str2;
	}
}

int main()
{
	input();
	return 0;
}