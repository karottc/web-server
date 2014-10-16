#ifndef HTTP_HANDLE_H_
#define HTTP_HANDLE_H_

#include "HttpRequest.h"
#include "HttpResponse.h"

//封装返回给客户端的消息
void packetMessage(HttpResponse* resp,HttpResponse::HttpStatusCode code,string statusMessage,string body);
//写入日志
//void writeLog(int fd,)

//处理Web server的默认页面请求
void handleIndex(const HttpRequest& req, HttpResponse* resp);
//对请求的是PHP页面的处理
void handlePHP(const HttpRequest& req, HttpResponse* resp);

//对HEAD方法的响应
void handleHEAD(const HttpRequest& req, HttpResponse* resp);
//对GET方法的响应
void handleGET(const HttpRequest& req, HttpResponse* resp);
//对CGI的调用，即GET方法带参数或者POST方法的响应
void handleCGI(const HttpRequest& req, HttpResponse* resp);
//对PUT方法的响应
void handlePUT(const HttpRequest& req, HttpResponse* resp);
//对DELETE方法的响应
void handleDELETE(const HttpRequest& req, HttpResponse* resp);


#endif
