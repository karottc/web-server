/*
 * =====================================================================================
 *
 *       Filename:  dealRequest_cy.h
 *
 *    Description:  处理用户请求
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 13时04分55秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang （陈杨） 
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef DEALREQUEST_CY_H
#define DEALREQUEST_CY_H

/* 请求方法 */
enum Req_Method{GET,HEAD,UNKOWN};
/* 请求类型 */
enum Req_Type{SIMPLE,FULL};
/* 请求消息的结构 */
struct ReqInfo
{
	enum Req_Method method;
	enum Req_Type type;
	char *resource; /* 请求资源的名字，包括URL*/
	int status;   /* 请求状态 */
};

/* 初始化资源结构 */
void InitReqInfo(struct ReqInfo *reqInfo);
/* 释放资源结构 */
void FreeReqInfo(struct ReqInfo *reInfo);
/* 得到HTTP头部 */
int Get_Request(int connId,struct ReqInfo *reqInfo);
/* 分析HTTP头部 */
int Parse_HTTP_Header(char *buffer,struct ReqInfo *reqInfo);

/* 对resource的操作 */
/* 将resource的文件写到connId中 */
int Return_Resource(int connId,int resource);
/* 尝试打开资源，根据返回值判断是否成功 */
int Check_Resource(struct ReqInfo *reqInfo);
void Return_Err_Msg(int connId,struct ReqInfo *reqInfo);

/* 输出HTTP响应报文的头部3行 */
void Output_HTTP_Headers(int connId,struct ReqInfo *reqInfo);

/* 处理用户请求 */
void deal_Request(int connId);

#endif
