#include "HttpServer.h"
#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <muduo/base/Logging.h>
#include <boost/bind.hpp>

#include <iostream>
#include <fstream>
//extern int logFd;
extern std::ofstream log;

//解析请求行
bool processRequestLine(const char* begin, const char* end, HttpContext* context)
{
    string line(begin,end);
    log << "\"" << line << "\" ";
  bool succeed = false;
  const char* start = begin;
  const char* space = std::find(start, end, ' ');
  HttpRequest& request = context->request();
  if (space != end && request.setMethod(start, space))   //将请求方法存储到context中
  {
    start = space+1;
    space = std::find(start, end, ' ');
    if (space != end)
    {
      request.setPath(start, space);   //HTTP请求行中的URL存储在context中
      start = space+1;
      succeed = end-start == 8 && std::equal(start, end-1, "HTTP/1."); //判断HTTP协议版本
      if (succeed)
      {
        if (*(end-1) == '1')
        {
          request.setVersion(HttpRequest::kHttp11);
        }
        else if (*(end-1) == '0')
        {
          request.setVersion(HttpRequest::kHttp10);
        }
        else
        {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

//解析HTTP请求
//从buf中取出从从连接接收到的所有数据
//将数据解析格式放到context中
bool parseRequest(Buffer* buf, HttpContext* context, Timestamp receiveTime)
{
  log << "[" << receiveTime.toFormattedString() << "] ";
  bool ok = true;
  bool hasMore = true;   //判断buf中是否还有数据
  while (hasMore)
  {
    if (context->expectRequestLine())      //解析请求行
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
        ok = processRequestLine(buf->peek(), crlf, context);
        if (ok)
        {
          context->request().setReceiveTime(receiveTime);
          buf->retrieveUntil(crlf + 2);
          context->receiveRequestLine();
        }
        else
        {
          hasMore = false;
        }
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context->expectHeaders())    //解析HTTP请求的头部域，有多行
    {
      const char* crlf = buf->findCRLF();
      if (crlf)
      {
          //头部行中，域名和域值，以“：”分割
        const char* colon = std::find(buf->peek(), crlf, ':');
        if (colon != crlf)
        {
          context->request().addHeader(buf->peek(), colon, crlf);
        }
        else
        {
          //头部行的结束是空行
          context->receiveHeaders();
        }
        buf->retrieveUntil(crlf + 2);
      }
      else
      {
        hasMore = false;
      }
    }
    else if (context->expectBody()) //解析请求的message-body
    {
        string body(buf->peek(),buf->peek()+buf->readableBytes());
        buf->retrieveUntil(buf->peek()+buf->readableBytes());
        context->request().setBody(body);
        hasMore = false;
        context->receiveBody();
    }
  }
  string timeLog = "[" + receiveTime.toFormattedString() + "] ";
  //write(logFd,timeLog.c_str(),timeLog.size());
  return ok;
}

//没有设定解析函数时的默认请求函数
void defaultHttpCallback(const HttpRequest&, HttpResponse* resp)
{
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
                       const string& name)
  : server_(loop, listenAddr, name),
    httpCallback_(defaultHttpCallback)
{
  server_.setConnectionCallback(
      boost::bind(&HttpServer::onConnection, this, _1));
  server_.setMessageCallback(
      boost::bind(&HttpServer::onMessage, this, _1, _2, _3));
}

HttpServer::~HttpServer()
{
}

void HttpServer::start()
{
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn)
{
  if (conn->connected())
  {
    conn->setContext(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,Buffer* buf,
                           Timestamp receiveTime)
{
  HttpContext* context = boost::any_cast<HttpContext>(conn->getMutableContext());

  log << conn->peerAddress().toIpPort() << " ";
  if (!parseRequest(buf, context, receiveTime))
  {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll())
  {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req)
{
  const string& connection = req.getHeader("Connection");
  bool close = connection == "close" ||
    (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection())
  {
    conn->shutdown();
  }
}

