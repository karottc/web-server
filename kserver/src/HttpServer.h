#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#include <muduo/net/TcpServer.h>
#include <boost/noncopyable.hpp>

using namespace muduo;
using namespace muduo::net;

class HttpRequest;
class HttpResponse;

//Web服务器的核心class
class HttpServer : boost::noncopyable
{
 public:
  typedef boost::function<void (const HttpRequest&,HttpResponse*)> HttpCallback;

  HttpServer(EventLoop* loop, const InetAddress& listenAddr, const string& name);

  ~HttpServer();

  void setHttpCallback(const HttpCallback& cb)
  {
    httpCallback_ = cb;
  }

  void setThreadNum(int numThreads)
  {
    server_.setThreadNum(numThreads);
  }

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp receiveTime);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer server_;
  HttpCallback httpCallback_;
};

#endif  // HTTPSERVER_H
