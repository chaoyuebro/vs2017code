#include "./echo.h"

EchoServer::EchoServer(muduo::net::EventLoop *loop, const muduo::net::InetAddress &listenAddr)
	:_loop(loop)
	, _server(loop, listenAddr, "EchoServer")
{
	_server.setConnectionCallback(std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
	_server.setMessageCallback(std::bind(&EchoServer::onMessage, this, std::placeholders::_1, std::placeholders::_2,
		std::placeholders::_3));
}
void EchoServer::onConnection(const muduo::net::TcpConnectionPtr &conn)
{
	LOG_INFO << "EchoServer-" << conn->peerAddress().toIpPort() << "->" << conn->localAddress().toIpPort() << "is" << (conn->connected() ? "UP" : "DOWN");
}
void EchoServer::onMessage(const muduo::net::TcpConnectionPtr &conn, muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	muduo::string msg(buf->retrieveAllAsString());
	LOG_INFO << conn->name() << "echo" << msg.size() << "bytes," << "data received at" << time.toString();
	conn->send(msg);
}

