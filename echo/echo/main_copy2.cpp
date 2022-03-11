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
//const int kBufSize = 64 * 1024;//ÿ�η���64k����
//const char* g_file = NULL;//�ļ���
//typedef std::shared_ptr<FILE> FilePtr;
//
//void onConnection(const TcpConnectionPtr& conn)
//{
//	LOG_INFO << "FileServer - " << conn->peerAddress().toIpPort() << " -> "//�Զ˵�IP��ַ���˿ں�
//		<< conn->localAddress().toIpPort() << " is "//���ص�IP��ַ���˿ں�
//		<< (conn->connected() ? "UP" : "DOWN");//�������˴�ӡUP,���ӶϿ���ӡDOWN
//	if (conn->connected())//���������
//	{
//		LOG_INFO << "FileServer - Sending file " << g_file
//			<< " to " << conn->peerAddress().toIpPort();
//		conn->setHighWaterMarkCallback(onHighWaterMark, kBufSize + 1);//���������� �ĳ��ȳ����û�ָ���Ĵ�С���ͻᴥ���ص�
//
//		FILE* fp = ::fopen(g_file, "rb");//�Զ���ʽ��һ���������ļ�
//		if (fp)
//		{
//			FilePtr ctx(fp, ::fclose);//����std::shared_ptr<FILE>�����������ָ�룬�˴�����һ������
//			��ʾctx���ü�����Ϊ0�ǣ�Ҫ����fp�������ָ����ͨ��fclos()�����ٵ�
//			conn->setContext(ctx);//���淢�͵��û�������
//			char buf[kBufSize];
//			size_t nread = ::fread(buf, 1, sizeof(buf), fp); //��ȡһ������ݣ��浽buf��
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
//	const FilePtr& fp = boost::any_cast<const FilePtr&>(conn->getContext());//�õ��ļ�������������ָ�롣
//	char buf[kBufSize];
//	size_t nread = ::fread(buf, 1, sizeof buf, get_pointer(fp));//������д���0������ݣ��ͼ������͡�����Ͽ����ӣ���doneд����־��
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
//	LOG_INFO << "pid = " << getpid();//��ӡ���ͻ���ID
//	if (argc > 1)//��������1��˵�������������ļ����֡�
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
