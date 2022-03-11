#include "muduo/net/TcpServer.h"
#include "muduo/base/Atomic.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Thread.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/TimerId.h"
#include "muduo/base/Timestamp.h"
#include <muduo/base/BlockingQueue.h>
#include <muduo/base/CountDownLatch.h>


#include <memory>
#include <string>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <iostream>



//Thread����

//void threadFunc()
//{
//	printf("tid=%d\n", muduo::CurrentThread::tid());
//}
//
//void threadFunc2(int x)
//{
//	printf("tid=%d, x=%d\n", muduo::CurrentThread::tid(), x);
//}
//
//class Foo
//{
//public:
//	explicit Foo(double x)
//		: x_(x)
//	{
//	}
//
//	void memberFunc()
//	{
//		printf("tid=%d, Foo::x_=%f\n", muduo::CurrentThread::tid(), x_);
//	}
//
//	void memberFunc2(const std::string& text)
//	{
//		printf("tid=%d, Foo::x_=%f, text=%s\n", muduo::CurrentThread::tid(), x_, text.c_str());
//	}
//
//private:
//	double x_;
//};
//
//int main()
//{
//	printf("pid=%d, tid=%d\n", ::getpid(), muduo::CurrentThread::tid());
//
//	threadFunc�޲Σ���������
//	muduo::Thread t1(threadFunc);
//	t1.start();
//	printf("t1.tid=%d\n", t1.tid());
//	t1.join();
//
//	threadFuncһ��������������
//	muduo::Thread t2(std::bind(threadFunc2, 42),
//		"thread for free function with argument");
//	t2.start();
//	printf("t2.tid=%d\n", t2.tid());
//	t2.join();
//
//	Foo����ĳ�Ա����memberFunc
//	Foo foo(87.53);
//	muduo::Thread t3(std::bind(&Foo::memberFunc, &foo),
//		"thread for member function without argument");
//	t3.start();
//	t3.join();
//	Foo����ĳ�Ա����memberFunc2
//	muduo::Thread t4(std::bind(&Foo::memberFunc2, std::ref(foo), std::string("Shuo Chen")));
//	t4.start();
//	t4.join();
//
//	printf("number of created threads %d\n", muduo::Thread::numCreated());
//}


//mutex��mutexGuard����
//#include <muduo/base/Mutex.h>
//#include <muduo/base/Thread.h>
//#include <muduo/base/Timestamp.h>
//#include <stdio.h>
//#include <vector>
//
//
//using namespace muduo;
//using namespace std;
//
//MutexLock g_mutex;
//vector<int> g_vec;
//const int kCount = 10 * 1000 * 1000;
//
//void threadFunc()
//{
//	for (int i = 0; i < kCount; ++i)
//	{
//		MutexLockGuard lock(g_mutex); //���캯���У�����
//		g_vec.push_back(i);
//	} //�����˳�������MutexLockGuard������������������˲���Ҫ�ֶ��Ľ�����
//}
//int main()
//{
//	const int kMaxThreads = 8;
//	g_vec.reserve(kMaxThreads * kCount);
//
//	//�����̲߳�����
//	Timestamp start(Timestamp::now());//��ȡ��ǰʱ��
//	for (int i = 0; i < kCount; ++i)
//	{
//		g_vec.push_back(i);
//	}
//
//	printf("single thread without lock %f\n", timeDifference(Timestamp::now(), start));//��ǰʱ���startʱ��֮��Ĳ��졣
//
//	//�����̼߳���
//	start = Timestamp::now();
//	threadFunc();
//	printf("single thread with lock %f\n", timeDifference(Timestamp::now(), start));
//
//	//�ֱ𴴽�1��2��3��4��5��6��7���̣߳�ͳ��ʱ�俪��
//	for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads)
//	{
//		std::vector<std::unique_ptr<Thread>> threads;
//		g_vec.clear();//���Ԫ�أ����ǲ����տռ�
//		start = Timestamp::now();
//		for (int i = 0; i < nthreads; ++i)
//		{
//			threads.emplace_back(new Thread(&threadFunc));
//			threads.back()->start();
//		}
//		for (int i = 0; i < nthreads; ++i)
//		{
//			threads[i]->join();
//		}
//		printf("%d thread(s) with lock %f\n", nthreads, timeDifference(Timestamp::now(), start));
//	}
//}

//�޽���в�������
//class Test
//{
//public:
//	Test(int numThreads) : latch_(numThreads)
//	{
//		for (int i = 0; i < numThreads; ++i)
//		{
//			char name[32];
//			snprintf(name, sizeof(name), "work thread %d", i);
//			//�����̣߳���threadFunc�̺߳���
//			threads_.emplace_back(new muduo::Thread(std::bind(&Test::threadFunc, this), muduo::string(name)));
//		}
//		for (auto& thr : threads_)
//		{
//			thr->start(); //���̺߳���threadFunc������
//		}
//	}
//
//	void run(int times) //ֻ�����߳�������Ʒ�����߳���������
//	{
//		printf("waiting for count down latch\n");
//
//		//1.���߳̽�һֱ������count_>0�����ϣ�ֱ��count_=0
//		//2.ÿ�����߳������󣬶������countDown()������count_--
//
//		latch_.wait();
//		//3.��count��Ϊ0ʱ��wait�����ѣ�����ִ������Ĵ���
//
//		printf("all threads started\n");
//		for (int i = 0; i < times; ++i)
//		{
//			char buf[32];
//			snprintf(buf, sizeof(buf), "hello %d", i);
//			queue_.put(buf); //���߳���queue_�з�Ԫ��
//			printf("tid=%d, put data = %s, size = %zd\n", muduo::CurrentThread::tid(), buf, queue_.size());
//		}
//	}
//
//	void joinAll()
//	{
//		for (size_t i = 0; i < threads_.size(); ++i)
//		{
//			queue_.put("stop");
//		}
//
//		for (auto& thr : threads_)
//		{
//			thr->join();
//		}
//	}
//
//private:
//	void threadFunc()
//	{
//		printf("tid=%d, %s started\n",
//			muduo::CurrentThread::tid(),
//			muduo::CurrentThread::name());
//
//		latch_.countDown(); //ÿ���̶߳���latch_.count_--
//		bool running = true;
//		while (running)
//		{
//			//queue_.take()��ʹ�������������ƶ��У�������Ϊ��ʱ��һֱ����
//			std::string d(queue_.take());
//			printf("tid=%d, get data = %s, size = %zd\n", muduo::CurrentThread::tid(), d.c_str(), queue_.size());
//			running = (d != "stop");
//		}
//
//		printf("tid=%d, %s stopped\n",
//			muduo::CurrentThread::tid(),
//			muduo::CurrentThread::name());
//	}
//
//	muduo::BlockingQueue<std::string> queue_;
//	muduo::CountDownLatch latch_;//��װ������������
//	std::vector<std::unique_ptr<muduo::Thread>> threads_; // �߳�����
//};
//
////void testMove()
////{
////	muduo::BlockingQueue<std::unique_ptr<int>> queue;
////	queue.put(std::unique_ptr<int>(new int(42)));
////	std::unique_ptr<int> x = queue.take();
////	printf("took %d\n", *x);
////	*x = 123;
////	queue.put(std::move(x));
////	std::unique_ptr<int> y = queue.take();
////	printf("took %d\n", *y);
////}
//
//int main()
//{
//	Test t(5);//(��һ��for)���̴߳���5���̣߳����ҷ������̶߳�����,(�ڶ���for)����ÿ���̣߳����߳�(wait)�ȴ����̶߳�������ÿ�����߳�����
//	//���ᵼ��count--;��count��Ϊ0ʱ���������̣߳����̷߳�������5�����߳�ȡ�����񣬵����̷߳����������֮�󣬵���joinAll������
//	//5��"stop",�����߳��յ�stop��running��Ϊfalse,Ȼ�����߳�ֹͣ��
//	t.run(100);
//	t.joinAll();
//
//	//testMove();
//
//	printf("number of created threads %d\n", muduo::Thread::numCreated());
//}



//�̳߳�ʾ��
//#include "muduo/base/ThreadPool.h"
//#include "muduo/base/CountDownLatch.h"
//#include "muduo/base/CurrentThread.h"
//#include "muduo/base/Logging.h"
//
//#include <stdio.h>
//#include <unistd.h>  // usleep
//
//using task=std::function<void()>;
//void print()
//{
//	printf("tid=%d\n", muduo::CurrentThread::tid());
//	sleep(1);
//}
//
//void printString(const std::string& str)
//{
//	LOG_INFO << str;
//	usleep(100 * 1000);
//}
//class printStr
//{
//public:
//	void print()
//	{
//		std::cout << "hello world" << std::endl;
//	}
//};
//void test(int maxSize)
//{
//	LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
//	muduo::ThreadPool pool("MainThreadPool");
//	pool.setMaxQueueSize(maxSize); //����������еĴ�С
//	pool.start(5); //����5���̣߳��������߳�
//	LOG_WARN << "Adding";
//	
//	//���̳߳�������Զ�����޲ε�print���񣬲������̳߳��еĿ����߳�
//	for (int i = 0; i < 10; i++)
//	{
//		task cb;
//		printStr mem;
//		cb = std::bind(&printStr::print, mem);
//		pool.run(cb);
//	}
//	//for (int i = 0; i < 100; ++i)
//	//{
//	//	char buf[32];
//	//	snprintf(buf, sizeof(buf), "task %d", i);
//	//	//���̳߳�������Զ�����в�����printString���񣬲������̳߳��еĿ����߳�
//	//	pool.run(std::bind(printString, std::string(buf)));
//	//}
//	LOG_WARN << "Done";
//
//	//���̳߳������CountDownLatch���еĳ�Ա����countDown()���������̳߳��еĿ����߳�
//	/*muduo::CountDownLatch latch(1);
//	pool.run(std::bind(&muduo::CountDownLatch::countDown, &latch));
//	latch.wait();*/
//
//	pool.stop();
//}
//
//int main()
//{
//	test(0);
//	/*test(1);
//	test(5);
//	test(10);
//	test(50);*/
//}

//Singletonʾ��
//#include <muduo/base/Singleton.h>
//#include <muduo/base/CurrentThread.h>
//#include <muduo/base/Thread.h>
//
//#include <stdio.h>
//
//class Test : muduo::noncopyable
//{
//public:
//	Test()
//	{
//		printf("tid=%d, constructing %p\n", muduo::CurrentThread::tid(), this);
//	}
//
//	~Test()
//	{
//		printf("tid=%d, destructing %p %s\n", muduo::CurrentThread::tid(), this, name_.c_str());
//	}
//
//	const muduo::string& name() const { return name_; }
//	void setName(const muduo::string& n) { name_ = n; }
//
//private:
//	muduo::string name_;
//};
//
//void threadFunc()
//{
//	//��ӡname	
//	printf("tid=%d, %p name=%s\n",
//		muduo::CurrentThread::tid(),
//		&muduo::Singleton<Test>::instance(),
//		muduo::Singleton<Test>::instance().name().c_str());
//
//	//����name = "only one, changed";
//	muduo::Singleton<Test>::instance().setName("only one, changed");
//}
//
//int main()
//{
//	//ʹ��Singleton�࣬���Զ����Test��װ���̰߳�ȫ�ĵ�����	
//	muduo::Singleton<Test>::instance().setName("only one");
//
//	muduo::Thread t1(threadFunc); //���߳�
//	t1.start();
//	t1.join();
//
//	//��ӡname����Ϊ����ģʽ���������ͬһ��ʵ������˴�ӡ���nameΪ�޸ĺ��ֵonly one, changed
//	printf("tid=%d, %p name=%s\n", //���߳�
//		muduo::CurrentThread::tid(),
//		&muduo::Singleton<Test>::instance(),
//		muduo::Singleton<Test>::instance().name().c_str());
//}

//channelʾ��
//#include <sys/timerfd.h>
//#include <unistd.h>
//#include "./Channel.h"
//using namespace muduo;
//using namespace muduo::net;
//using namespace std;
//
//EventLoop* g_loop;
//int timerfd;
//
//void timeout(Timestamp receiveTime)
//{
//	printf("Timeout!\n");
//	uint64_t howmany;
//	read(timerfd, &howmany, sizeof(howmany));
//	cout << "get howmany=" << howmany<<endl;
//}
//void func()
//{
//	cout << "write event" << endl;
//	g_loop->quit();
//}
//int main()
//{
//	EventLoop loop;
//	g_loop = &loop;
//	timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
//	struct itimerspec howlong;
//	bzero(&howlong, sizeof(howlong));
//	howlong.it_value.tv_sec = 1;
//	howlong.it_value.tv_nsec = 0;
//	howlong.it_interval.tv_sec = 1;
//	howlong.it_interval.tv_nsec = 0;
//	timerfd_settime(timerfd, 0, &howlong, NULL);
//	Channel channel(&loop, timerfd);
//	channel.setReadCallback(std::bind(timeout, _1));
//	channel.enableReading();
//	Channel channel2(&loop, STDIN_FILENO);
//	channel2.setReadCallback(std::bind(&func));
//	channel2.enableReading();
//	
//	cout << "before loop()" << endl;
//	loop.loop();
//	loop.quit();
//	cout << "after loop()" << endl;
//	CloseCallback(timerfd);
//	CloseCallback(0);
//}

//#include "muduo/net/EventLoopThreadPool.h"
//#include "muduo/net/EventLoop.h"
//#include "muduo/base/Thread.h"
//#include "muduo/base/Logging.h"
//#include "muduo/net/TcpServer.h"
//
//#include <stdio.h>
//#include <unistd.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//void print(EventLoop* p = NULL)
//{
//	printf("main(): pid = %d, tid = %d, loop = %p\n",
//		getpid(), CurrentThread::tid(), p);
//}
//
//void init(EventLoop* p)
//{
//	printf("init(): pid = %d, tid = %d, loop = %p\n",
//		getpid(), CurrentThread::tid(), p);
//}
//
//int main()
//{
//	EventLoop loop;//main reactor ����������
//	EventLoopThreadPool model(&loop, "three");
//	model.setThreadNum(3);//subReactor ���𴴽�num���̣߳�ÿ���̶߳�����һ��subEventLoop������IO.
//	model.start(init);
//	EventLoop* nextLoop = model.getNextLoop();
//	nextLoop->runInLoop(std::bind(print, nextLoop));
//}

//EventLoopThreadʹ��
//#include "muduo/net/EventLoop.h"
//#include "muduo/net/EventLoopThread.h"
//
//#include <stdio.h>
//
//using namespace muduo;
//using namespace muduo::net;
//
//void runInThread()
//{
//	printf("runInThread(): pid = %d, tid = %d\n",
//		getpid(), CurrentThread::tid());
//}
//
//int main()
//{
//	printf("main(): pid = %d, tid = %d\n",
//		getpid(), CurrentThread::tid());
//	EventLoop loop2;
//	EventLoopThread loopThread;
//	EventLoop* loop = loopThread.startLoop();
//	loop2.runInLoop(runInThread);
//	
//	// �첽����runInThread������runInThread��ӵ�loop��������IO�̣߳��ø�IO�߳�ִ��
//	loop->runInLoop(runInThread);
//	sleep(1);
//	// runAfter�ڲ�Ҳ������runInLoop����������Ҳ���첽����
//	loop->runAfter(2, runInThread);
//	sleep(3);
//	loop->quit();
//	loop2.quit();
//	printf("exit main().\n");
//}

//#include "muduo/net/TcpServer.h"
//#include "muduo/net/EventLoop.h"
//#include "muduo/net/InetAddress.h"
//
//#include <functional>
//#include <iostream>
//#include <stdio.h>
//
//using namespace muduo;
//using namespace muduo::net;
//using namespace std;
//class TestServer
//{
//public:
//	TestServer(EventLoop* loop,
//		const InetAddress& listenAddr)
//		: loop_(loop),
//		server_(loop, listenAddr, "TestServer")
//	{
//		server_.setConnectionCallback(
//			std::bind(&TestServer::onConnection, this, _1));
//		server_.setMessageCallback(
//			std::bind(&TestServer::onMessage, this, _1, _2, _3));
//	}
//
//	void start()
//	{
//		server_.start();
//	}
//
//private:
//	void onConnection(const TcpConnectionPtr& conn)
//	{
//		if (conn->connected())
//		{
//			printf("onConnection(): new connection [%s] from %s\n",
//				conn->name().c_str(),
//				conn->peerAddress().toIpPort().c_str());
//		}
//		else
//		{
//			printf("onConnection(): connection [%s] is down\n",
//				conn->name().c_str());
//		}
//	}
//
//	void onMessage(const TcpConnectionPtr& conn,
//		Buffer* buf,
//		Timestamp receiveTime)
//	{
//		string msg(buf->retrieveAllAsString());
//		printf("onMessage(): received %zd bytes from connection [%s] at %s\n",
//			msg.size(),
//			conn->name().c_str(),
//			receiveTime.toFormattedString().c_str());
//		printf("current pid = %d, tid = %d\n",
//			getpid(), CurrentThread::tid());
//		conn->send(msg);
//	}
//
//	EventLoop* loop_;
//	TcpServer server_;
//};


//int main()
//{
//	printf("main(): pid = %d\n", getpid());
//	printf("main(): pid = %d, tid = %d\n",
//				getpid(), CurrentThread::tid());
//	InetAddress listenAddr(8888);
//	EventLoop loop;
//
//	TestServer server(&loop, listenAddr);
//	server.start();
//
//	loop.loop();
//}


//EventLoopThreadPool ʹ��
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include <stdio.h>
#include <iostream>
#include <functional>

using namespace std;
using namespace muduo;
using namespace muduo::net;



class TestServer
{
public:
	TestServer(EventLoop* loop,
		const InetAddress& listenAddr, int numThreads)
		: loop_(loop),
		server_(loop, listenAddr, "TestServer"),
		numThreads_(numThreads)
	{
		server_.setConnectionCallback(
			std::bind(&TestServer::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(
			std::bind(&TestServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		server_.setThreadNum(numThreads);
	}

	void start()
	{
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		if (conn->connected())
		{
			printf("onConnection(): new connection [%s] from %s\n",
				conn->name().c_str(),
				conn->peerAddress().toIpPort().c_str());
		}
		else
		{
			printf("onConnection(): connection [%s] is down\n",
				conn->name().c_str());
		}
	}

	void onMessage(const TcpConnectionPtr& conn,
		Buffer* buf,
		Timestamp receiveTime)
	{
		printf("onMessage(): received %zd bytes from connection [%s]\n",
			 conn->name().c_str());
		printf("current pid = %d, tid = %d\n",
					getpid(), CurrentThread::tid());
	}

	EventLoop* loop_;
	TcpServer server_;
	int numThreads_;
};


int main()
{
	printf("main(): pid = %d\n", getpid());

	InetAddress listenAddr(8888);
	EventLoop loop;

	TestServer server(&loop, listenAddr, 4);
	server.start();

	loop.loop();
}