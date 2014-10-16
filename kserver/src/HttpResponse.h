#ifndef _HTTPRESPONSE_H
#define _HTTPRESPONSE_H

#include <muduo/base/copyable.h>
#include <muduo/base/Types.h>
#include <muduo/net/Buffer.h>

#include <map>

using namespace muduo;
using namespace muduo::net;

class HttpResponse : public muduo::copyable
{
 public:
  enum HttpStatusCode
  {
    kUnknown,
    k200Ok = 200,
    k201Created = 201,
    k301MovedPermanently = 301,
    k400BadRequest = 400,
    k404NotFound = 404,
    k500InternalError = 500,
  };

  explicit HttpResponse(bool close)
    : statusCode_(kUnknown),closeConnection_(close)
  {
  }

  void setStatusCode(HttpStatusCode code)
  { statusCode_ = code; }

  void setStatusMessage(const string& message)
  { statusMessage_ = message; }

  void setCloseConnection(bool on)
  { closeConnection_ = on; }

  bool closeConnection() const
  { return closeConnection_; }

  void setContentType(const string& contentType)
  { addHeader("Content-Type", contentType); }

  void addHeader(const string& key, const string& value)
  { headers_[key] = value; }

  void setBody(const string& body)
  { body_ = body; }

  void appendToBuffer(muduo::net::Buffer* output) const;

 private:
  std::map<string, string> headers_;
  HttpStatusCode statusCode_;
  string httpVersion_;
  string statusMessage_;
  bool closeConnection_;
  string body_;
};

#endif  // HTTPRESPONSE_H
