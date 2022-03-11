#include "./codec.h"
#include "./JsonTrans.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "gzip.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace muduo;
using namespace muduo::net;

class ChatClient : noncopyable
{
public:
	ChatClient(EventLoop* loop, const InetAddress& serverAddr)
		: client_(loop, serverAddr, "ChatClient"),
		codec_(std::bind(&ChatClient::onStringMessage, this, _1, _2, _3))
	{
		client_.setConnectionCallback(
			std::bind(&ChatClient::onConnection, this, _1));
		client_.setMessageCallback(
			std::bind(&LengthHeaderCodec::onMessage, &codec_, _1, _2, _3));
		client_.enableRetry();
	}
	void dealMessage(const string& msg)
	{
		/*printf("current pid = %d, tid = %d deal message\n",
			getpid(), CurrentThread::tid());*/
		if (msg.size() >= headerLen)
		{
			muduo::string cmdBuf(msg.substr(0, 8));
			muduo::string cmd;
			
			if (cmdBuf.find('\0') > 0)
			{
				cmd = cmdBuf.substr(0, cmdBuf.find('\0'));
			}
			else
			{
				cmd = cmdBuf;
			}
			cout << cmd << endl;
			muduo::string format;
			
			muduo::string formatBuf(msg.substr(8, 4));
			if (formatBuf.find('\0') > 0)
			{
				format = formatBuf.substr(0, formatBuf.find('\0'));
			}
			else
			{
				format = formatBuf;
			}
			cout << format << endl;
			muduo::string userNameBuf(msg.substr(12, 32));
			muduo::string userName;
			
			if (userNameBuf.find('\0') > 0)
			{
				userName = userNameBuf.substr(0, userNameBuf.find('\0'));
			}
			else
			{
				userName = userNameBuf;
			}
			cout << userName << endl;
			muduo::string body(msg.substr(headerLen));
			if (cmd == "RSDPJ")
			{
				if (format == "TXT")
				{
					cout << body << endl;
				}
				else if (format == "GZIP") {
					const char *source = body.c_str();
					int srcLen = (int)body.size();
					int destLen = srcLen * 2;
					int ret = -5;
					char* uncompressed = NULL;
					while (ret == -5)
					{
						uncompressed = (char *)malloc(destLen);
						memset(uncompressed, 0, destLen);
						ret = gzDecompress(source, srcLen, uncompressed, destLen);
						if (ret == -5)
						{
							destLen = destLen * 2;
							free(uncompressed);//如果空间不够,就将destLen扩大两倍然后释放原空间。
						}
					}
					muduo::string unComBody(uncompressed);
					cout << unComBody << endl;
				}
			}
			
			//进来的只有put和get,get的话就直接返回查询结果以便于在客户端合并或者直接覆盖
		}
	}
	void connect()
	{
		client_.connect();
	}

	void disconnect()
	{
		client_.disconnect();
	}
	void setOperation(string operation)
	{
		operation_ = operation;
	}
	void cover(string message)
	{
		jsonTrans_.coverJson(message);//如果是cover就直接将服务器上的数据覆盖到本地。
	}
	void merge(string message)//如果是merge就将服务器数据和本地数据合并。
	{
		jsonTrans_.setMysqlStr(message);
		string localStr;
		string mergeStr;
		jsonTrans_.inputJson(localStr);
		jsonTrans_.setLocalStr(localStr);
		jsonTrans_.mergeJson(mergeStr);
		write("PUT", mergeStr);
		jsonTrans_.coverJson(mergeStr);
	}
	void write(const StringPiece& cmd, const StringPiece& body)
	{
		MutexLockGuard lock(mutex_);
		if (connection_)
		{
			codec_.send(get_pointer(connection_), cmd,"chaoyuebro",body);//get_pointer返回智能指针的原指针。
		}
	}//发送数据
	void sendHeartBeat()
	{
		usleep(30000 * 1000);
		write("HBET", "TXT");
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG_INFO << conn->localAddress().toIpPort() << " -> "
			<< conn->peerAddress().toIpPort() << " is "
			<< (conn->connected() ? "UP" : "DOWN");

		MutexLockGuard lock(mutex_);
		if (conn->connected())
		{
			connection_ = conn;
		}
		else
		{
			connection_.reset();
		}
	}

	void onStringMessage(const TcpConnectionPtr&,
		const string& message,
		Timestamp)
	{
		dealMessage(message);
		/*printf("current pid = %d, tid = %d package\n",
			getpid(), CurrentThread::tid());*/
	}
private:
	TcpClient client_;
	LengthHeaderCodec codec_;
	MutexLock mutex_;
	TcpConnectionPtr connection_ GUARDED_BY(mutex_);
	JsonTrans jsonTrans_;//用来读取json文件或者输出到json文件
	string operation_;//主要用来判断是cover还是merge
};

