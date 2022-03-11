#include <iostream>    //��дio c++��׼��
#include <fstream>     //��д�ļ� c++��׼��
#include <string>      //�ַ����� c++��׼��
#include <sstream>     //�ַ����� c++��׼��
#include <json/json.h> //jsoncpp��ͷ�ļ�
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