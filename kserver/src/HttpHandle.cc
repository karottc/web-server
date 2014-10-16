#include "HttpHandle.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

extern std::ofstream log;
extern string phpCgi;
extern string documentRoot;
extern string cgiRoot;

//请求的URL为“/”，调用web server默认的页面，
//默认页面是index.html,如果没有则调用index.php，
//如果上面两个页面均找不到，则返回NOT found
void handleIndex(const HttpRequest& req, HttpResponse* resp)
{
    //得到返回给客户端的页面,index.php
    string path = documentRoot + req.path() + "index.html";
    struct stat sf;     //用来表示文件的状态
    if (lstat(path.c_str(),&sf) < 0)    //查找文件是否存在，包括符号链接
    {
        //如果没有index.html，则找index.php
        path = documentRoot + req.path() + "index.php";
        if (lstat(path.c_str(),&sf) < 0)
        {
            //如果文件不存在返回信息给客户端
            packetMessage(resp,HttpResponse::k404NotFound,"Not Found","");
            resp->setCloseConnection(true);
            log << "404 0" << std::endl;
            return;
        }
        else
        {
            handlePHP(req,resp);  //不存在index.html, 有index.php
            return;
        }
    }
    else
    {
        int fd = open(path.c_str(),O_RDONLY);  //用只读模式打开index.html文件
        char* buf = new char[sf.st_size];    //st_size是文件的大小
        read(fd,buf,sf.st_size);
        packetMessage(resp,HttpResponse::k200Ok,"OK",buf);
        log << "200 " << strlen(buf) << std::endl;
        close(fd);
    }
}

//请求的是PHP页面，调用php-cgi来将PHP标签转换成HTML的标签
void handlePHP(const HttpRequest& req, HttpResponse* resp)
{
    string path;
    if (req.path() == "/")   //index.html文件不存在
    {
        path = "/index.php";
    }
    else
    {
        path = req.path();
    }
      int p[2];
      pipe(p);  //创建管道和执行PHP-CGI的子进程通信
      string buf;    //表示HTTP响应的message-body
      pid_t pid = fork();
      if (pid == 0)   //子进程中执行PHP-CGI
      {
          close(p[0]);  //关闭管道的读端
          dup2(p[1],STDOUT_FILENO);      //将STDOUT_FILENO重定向到p[1],管道写入端
          string tt = documentRoot + path;
          char *exeArgs[3];      //构造execv的参数
          exeArgs[0] = strdup((phpCgi + "/php-cgi").c_str());    //需要执行的文件
          exeArgs[1] = strdup(tt.c_str());    //所带的参数
          exeArgs[2] = NULL;    //参数以NULL结尾
          execv((phpCgi+"/php-cgi").c_str(),exeArgs);
          delete exeArgs[0];
          delete exeArgs[1];
          exit(0);
      }
      else if(pid > 0)      //父进程中读入子进程中写入的数据
      {
          close(p[1]);      //关闭管道的写端
          char c;
          buf = "";
          bool started = false;     //表明HTML标签的开始位置
          while (read(p[0],&c,1) > 0)
          {
              if (!started)
              {
                  if (c == '<')
                  {
                      started = true;
                      buf += c;
                  }
              }
              else
              {
                  buf += c;
              }
          }
      }
      packetMessage(resp,HttpResponse::k200Ok,"OK",buf);
      log << "200 " << buf.size() << std::endl;
}

//HTTP请求，只返回头部信息，不包含message-body
void handleHEAD(const HttpRequest& req, HttpResponse* resp)
{
    string path = documentRoot + req.path();
    struct stat sf;
    if (lstat(path.c_str(),&sf) < 0)
    {
        packetMessage(resp,HttpResponse::k404NotFound,"Not Found","");
        log << "404 0" << std::endl;
        resp->setCloseConnection(true);
    }
    else
    {
        packetMessage(resp,HttpResponse::k200Ok,"OK","");
        log << "200 0" << std::endl;
        resp->setCloseConnection(true);
    }
}

void handleGET(const HttpRequest& req, HttpResponse* resp)
{
    string path = req.path();
    if (path.find("?") < path.size())
    {
        //增加对get方法带参数的处理
        handleCGI(req,resp);
    }
    else
    {
        path = documentRoot + req.path();
        struct stat sf;
        if (lstat(path.c_str(),&sf) < 0)
        {
            packetMessage(resp,HttpResponse::k404NotFound,"Not Found","");
            resp->setCloseConnection(true);
            log << "404 0" << std::endl;
            return;
        }
        else
        {
            int fd = open(path.c_str(),O_RDONLY);
            char* buf = new char[sf.st_size];
            read(fd,buf,sf.st_size);
            packetMessage(resp,HttpResponse::k200Ok,"OK",buf);
            log << "200 " << strlen(buf) << std::endl;
            close(fd);
        }
    }
}

void handleCGI(const HttpRequest& req, HttpResponse* resp)
{
    string method = req.methodString();
    if (method == "GET")
    {
        string path = req.path();
        string cgi(path,0,path.find("?"));  //解析出需要执行的文件
        string var(path,path.find("?")+1,path.size());   //得到写入环境变量的参数
        string buf;
        int p[2];
        pipe(p);
        pid_t pid = fork();
        if (pid == 0)  //子进程中执行cgi脚本
        {
            close(p[0]);
            setenv("QUERY_STRING",var.c_str(),1);   //将值设入环境变量
            dup2(p[1],STDOUT_FILENO);
            string command = cgiRoot + cgi;
            char *exeArgs[2];
            exeArgs[0] = strdup(command.c_str());
            exeArgs[1] = NULL;
            execv(command.c_str(),exeArgs);
            delete exeArgs[0];
            exit(0);
        }
        else if (pid > 0)
        {
            close(p[1]);
            char c;
            buf = "";
            //sleep(5);
            while (read(p[0],&c,1) > 0)
                buf += c;
        }
        packetMessage(resp,HttpResponse::k200Ok,"OK",buf);
        log << "200 " << buf.size() << std::endl;
    }
    else if (method == "POST")   //处理POST方法的CGI
    {
        //对POST方法的处理
        string body = req.body();
        char length[32];
        sprintf(length,"%d",body.size());
        string buf;
        int readp[2];    //子进程从父进程读数据
        int writep[2];   //子进程向父进程写入数据
        pipe(readp);
        pipe(writep);
        pid_t pid = fork();
        if (pid == 0)
        {
            close(readp[1]);   //关闭读数据管道的写端
            close(writep[0]);  //关闭写数据的读端
            setenv("CONTENT-LENGTH",length,1);
            dup2(readp[0],STDIN_FILENO);   //将管道读端重定向为标准输入
            dup2(writep[1],STDOUT_FILENO);
            string cgi = cgiRoot + req.path();
            char* exeArgs[2];
            exeArgs[0] = strdup(cgi.c_str());
            exeArgs[1] = NULL;
            execv(cgi.c_str(),exeArgs);
            delete exeArgs[0];
            exit(0);
        }
        else if (pid > 0)
        {
            close(readp[0]);
            close(writep[1]);
            write(readp[1],body.c_str(),body.size());
            char c;
            buf = "";
            while (read(writep[0],&c,1) > 0)
                buf += c;
        }
        packetMessage(resp,HttpResponse::k200Ok,"OK",buf);
        log << "200 " << buf.size() << std::endl;
    }
}

//对PUT方法的响应
//检查URL中的文件，不存在就创建一个新文件
void handlePUT(const HttpRequest& req, HttpResponse* resp)
{
    string path = req.path();
    struct stat sf;
    string::size_type pos = 1;
    //判断中间目录是否存在，不存在就创建中间目录
    while ((pos = path.find("/",pos)) < path.size())
    {
        string tt(path,0,pos++);
        string dir = documentRoot + tt;
        if (stat(tt.c_str(),&sf) < 0)
            mkdir(dir.c_str(),0755);
    }
    path = documentRoot + req.path();
    int fd = 0;
    if (stat(path.c_str(),&sf) < 0)
    {
        fd = open(path.c_str(),O_WRONLY|O_CREAT|O_NONBLOCK|O_TRUNC,0644);
    }
    else     //文件存在，返回200响应代码
    {
        packetMessage(resp,HttpResponse::k200Ok,"OK","资源已存在！！");
        resp->setCloseConnection(true);
        log << "200 0" << std::endl; 
        return;
    }
    if (fd > 0)    //创建文件成功，返回201响应代码
    {
        close(fd);
        packetMessage(resp,HttpResponse::k201Created,"Created","资源已创建！！");
        resp->setCloseConnection(true);
        log << "201 0" << std::endl;
    }
    else    //创建资源失败，返回错误消息
    {
        packetMessage(resp,HttpResponse::k500InternalError,"Internal Server Error",
                "服务器内部错误！！");
        resp->setCloseConnection(true);
        log << "500 0" << std::endl;
    }
}

//对DELETE方法的响应
void handleDELETE(const HttpRequest& req, HttpResponse* resp)
{
    string path = documentRoot + req.path();
    struct stat sf;
    if (lstat(path.c_str(),&sf) < 0)
    {
        packetMessage(resp,HttpResponse::k200Ok,"OK","资源不存在！！");
        log << "200 0" << std::endl;
    }
    else
    {
        if (remove(path.c_str()) == 0)
        {
            packetMessage(resp,HttpResponse::k200Ok,"OK","资源删除成功！！");
            log << "200 0" << std::endl;
        }
        else
        {
            packetMessage(resp,HttpResponse::k500InternalError,"Internal Server Error ",
                    "服务器内部错误！！");
            log << "500 0" << std::endl;
        }
    }
}

//封装返回给客户端的消息
void packetMessage(HttpResponse* resp,HttpResponse::HttpStatusCode code,string statusMessage,string body)
{ 
    resp->setStatusCode(code);
    resp->setStatusMessage(statusMessage);
    resp->setContentType("text/html;charset=UTF-8");
    resp->addHeader("Server","Kserver");
    resp->addHeader("Author","Chen Yang (SCU:0943111194)");
    if (body == "")
        return;
    resp->setBody(body);
}
