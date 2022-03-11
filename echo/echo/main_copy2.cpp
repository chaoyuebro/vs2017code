//#include "muduo/base/Logging.h"
//#include "muduo/net/EventLoop.h"
//#include "muduo/net/TcpServer.h"
//#include <stdio.h>
//#include <unistd.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//void onHighWaterMark(const TcpConnectionPtr& conn, size_t len)
//{
//	LOG_INFO << "HighWaterMark " << len;
//}
//
//const int kBufSize = 64 * 1024;//每次发送64k数据
//const char* g_file = NULL;//文件名
//typedef std::shared_ptr<FILE> FilePtr;
//
//void onConnection(const TcpConnectionPtr& conn)
//{
//	LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() << " -> "//对端的IP地址：端口号
//		<< conn->localAddress().toIpPort() << " is "//本地的IP地址：端口号
//		<< (conn->connected() ? "UP" : "DOWN");//连接上了打印UP,连接断开打印DOWN
//	if (conn->connected())//如果连接上
//	{
//		LOG_INFO << "FileServer - Sending file " << g_file
//			<< " to " << conn->peerAddress().toIpPort();
//		conn->setHighWaterMarkCallback(onHighWaterMark, kBufSize + 1);//如果输出缓冲 的长度超过用户指定的大小，就会触发回调
//
//		FILE* fp = ::fopen(g_file, "rb");//以读方式打开一个二进制文件
//		if (fp)
//		{
//			FilePtr ctx(fp, ::fclose);//构造std::shared_ptr<FILE>对象管理智能指针，此处多了一个参数
//			表示ctx引用计数减为0是，要销毁fp这个智能指针是通过fclos()来销毁的
//			conn->setContext(ctx);//保存发送的用户上下文
//			char buf[kBufSize];
//			size_t nread = ::fread(buf, 1, sizeof(buf), fp); //读取一块的内容，存到buf中
//			conn->send(buf, static_cast<int>(nread));
//		}
//		else
//		{
//			conn->shutdown();
//			LOG_INFO << "FileServer - no such file";
//		}
//	}
//}
//
//void onWriteComplete(const TcpConnectionPtr& conn)
//{
//	const FilePtr& fp = boost::any_cast<const FilePtr&>(conn->getContext());//得到文件描述符的智能指针。
//	char buf[kBufSize];
//	size_t nread = ::fread(buf, 1, sizeof buf, get_pointer(fp));//如果还有大于0块的内容，就继续发送。否则断开连接，把done写入日志。
//	if (nread > 0)
//	{
//		conn->send(buf, static_cast<int>(nread));
//	}
//	else
//	{
//		conn->shutdown();
//		LOG_INFO << "FileServer - done";
//	}
//}
//
//int main(int argc, char* argv[])
//{
//	LOG_INFO << "pid = " << getpid();//打印出客户端ID
//	if (argc > 1)//参数大于1，说明参数里面有文件名字。
//	{
//		g_file = argv[1];
//		EventLoop loop;
//		InetAddress listenAddr(2021);
//		TcpServer server(&loop, listenAddr, "FileServer");
//		server.setConnectionCallback(onConnection);
//		server.setWriteCompleteCallback(onWriteComplete);
//		server.start();
//		loop.loop();
//	}
//	else
//	{
//		fprintf(stderr, "Usage: %s file_for_downloading\n", argv[0]);
//	}
//}
//
