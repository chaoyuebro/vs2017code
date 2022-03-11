#include <muduo/base/LogFile.h>
#include <muduo/base/Logging.h>
#include <muduo/base/TimeZone.h>
#include <unistd.h>

std::unique_ptr<muduo::LogFile> g_logFile;
int g_total;
FILE* g_file;

void dummyOutput(const char* msg, int len)
{
	g_total += len;
	if (g_file) {
		fwrite(msg, 1, len, g_file);
	}
	else if (g_logFile){
		g_logFile->append(msg, len);
	}
}
void test(const char *path)
{
	muduo::TimeZone Beijing("/usr/share/zoneinfo/Asia/Chongqing");  // Å¦Ô¼Ê±Çø
	muduo::Logger::setTimeZone(Beijing);
	char name[256] = { '\0' };
	strncpy(name, path, sizeof(name) - 1);
	//g_logFile.reset(new muduo::LogFile(::basename(name), 200 * 1000));
	g_file = fopen("./log/log.log", "w");
	muduo::Logger::setOutput(dummyOutput);
	muduo::Logger::setFlush([] { g_logFile->flush(); });
	muduo::string line = "1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

	for (int i = 0; i < 5000; ++i)
	{
		LOG_INFO << line << i;
		usleep(1000);
	}
}
int main(int argc,char *argv[])
{
	test(argv[0]);
	return 0;
}