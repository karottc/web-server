/*
 * =====================================================================================
 *
 *       Filename:  dealRequest_cy.c
 *
 *    Description:  实现dealRequest_cy.h
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 13时06分49秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang （陈杨）
 *        Company:  
 *
 * =====================================================================================
 */
#include "dealRequest_cy.h"
#include "utility_cy.h"
#include "assist_cy.h"

void
InitReqInfo(struct ReqInfo *reqInfo)
{
	reqInfo->method = UNKOWN;
	reqInfo->type = SIMPLE;
	reqInfo->resource = NULL;
	reqInfo->status = 200;
}

void
FreeReqInfo(struct ReqInfo *reqInfo)
{
	if (reqInfo->resource)
		free(reqInfo->resource);
}

int
Get_Request(int connId,struct ReqInfo *reqInfo)
{
	char buffer[MAX_REQ_LINE] = {0};
	int rval;  /* 超时标志 */
	fd_set fds;
	struct timeval tv;    /* 时间结构 */

	/* 设置等待时间 5秒为超时 */
	tv.tv_sec = 5;
	tv.tv_usec = 0;

	/* 得到请求头 */
	do
	{
		/* 重设文件描述集,初始化 */
		FD_ZERO(&fds);
		FD_SET(connId,&fds);

		/* 等待超时.调用select的到请求 */
		rval = select(connId+1,&fds,NULL,NULL,&tv);
		if (rval < 0)
			Error_Quit("Can't call to select to get request! \n");
		else if (rval == 0)
			return -1;  /* 超时 */
		else
		{
			/* 获取输入 */
			fprintf(stderr,"Read_Line ----\n");
			Read_Line(connId,buffer,MAX_REQ_LINE-1);
			fprintf(stderr,"buffer = %s",buffer);
			Trim(buffer);
			if (buffer[0]=='\0')
				break;
			fprintf(stderr,"Parse_HTTP_Header......\n");
			if (Parse_HTTP_Header(buffer,reqInfo))
			{
				fprintf(stderr,"parse http faid ......\n");
				break;
			}
/* 			fprintf(stderr,"after Parsr-----buffer = %s\n",buffer);   */
		}

		fprintf(stderr,"ReqInfo 结构：\n");
		fprintf(stderr,"method = %d\n",reqInfo->method);
		fprintf(stderr,"type = %d\n",reqInfo->type);
		fprintf(stderr,"resource = %s\n",reqInfo->resource);
		fprintf(stderr,"status = %d\n\n",reqInfo->status);

	}while (reqInfo->type != SIMPLE);
	return 0;
}

int
Parse_HTTP_Header(char *buffer,struct ReqInfo *reqInfo)
{
	static int first_header = 1;
	char *endptr;
	int len;

	/* 第一次请求,如果不是第一次，first_header = 0 */
	if (first_header == 1)
	{
		/* 这个之实现了GET和HEAD方法 */
		if (!strncmp(buffer,"GET ",4))
		{
			reqInfo->method = GET;
			buffer += 4;
		}
		else if (!strncmp(buffer,"HEAD ",5))
		{
			reqInfo->method = HEAD;
			buffer += 5;
		}
		else
		{
			reqInfo->method = UNKOWN;
			reqInfo->status = 501;
			return -1;
		}
		
		while (*buffer && isspace(*buffer))
			buffer++;

		/* 计算resources的length */
		endptr = strchr(buffer,' ');
		if (endptr == NULL)
			len = strlen(buffer);
		else
			len = endptr - buffer;
		if (len == 0)
		{
			reqInfo->status = 400;
			return -1;
		}

		/* 分配储存空间 */
		reqInfo->resource = calloc(len+1,sizeof(char));
		strncpy(reqInfo->resource,buffer,len);

		/* 得到HTTP的版本 */
		if (strstr(buffer,"HTTP/"))
			reqInfo->type = FULL;
		else
			reqInfo->type =	SIMPLE;

		first_header = 0;

		return 0;
	}

	return 0;
}


int
Return_Resource(int connId,int resource)
{
	char c;
	int i;
	while ((i = read(resource,&c,1)))
	{
		if (i < 0)
			Error_Quit("Read file Error");
		if ((write(connId,&c,1))<1)
			Error_Quit("Send file Error!!");
	}
	return 0;
}

int
Check_Resource(struct ReqInfo *reqInfo)
{
	strcat(server_root,reqInfo->resource);
/*	strcat(server_root,"/index.html");  */
    // 这里或许用access这个系统调用会更好。
	return open(server_root,O_RDONLY);
}

void
Return_Err_Msg(int connId,struct ReqInfo *reqInfo)
{
	char buffer[100];
	sprintf(buffer,"<HTML>\n<HEAD>\n<TITLE>Server Error %d </TITLE>\n</HEAD>\n\n",reqInfo->status);
	Write_Line(connId,buffer,strlen(buffer));
	sprintf(buffer,"<BODY>\n<H1>Server Error %d </H1>\n",reqInfo->status);
	Write_Line(connId,buffer,strlen(buffer));
	sprintf(buffer,"<P>This reqquest could not be completed!</P>\n</BODY>\n</HTML>\n");
	Write_Line(connId,buffer,strlen(buffer));

}


void
Output_HTTP_Headers(int connId,struct ReqInfo *reqInfo)
{
	char buffer[100];
	/* 响应报文的首部行 */
	sprintf(buffer,"http/1.0 %d OK \r\n",reqInfo->status);
	Write_Line(connId,buffer,strlen(buffer));
	Write_Line(connId, "Server: CYWebServ v1.0\r\n", 24);
    Write_Line(connId, "Content-Type: text/html\r\n", 25);

    Write_Line(connId, "\r\n", 2);

}

void
deal_Request(int connId)
{
	struct ReqInfo reqInfo;
	int resource = 0;

	/* 初始化请求信息 */
	InitReqInfo(&reqInfo);
	/* 得到HTTP请求 */
	fprintf(stderr,"Get HTTP Request!!\n");
	if (Get_Request(connId,&reqInfo)<0)
		return;   /* 得到失败 */
	fprintf(stderr,"ReqInfo 结构：\n");
	fprintf(stderr,"method = %d\n",reqInfo.method);
	fprintf(stderr,"type = %d\n",reqInfo.type);
	fprintf(stderr,"resource = %s\n",reqInfo.resource);
	fprintf(stderr,"status = %d\n",reqInfo.status);
	/* 检查resources是否存在，是否使用持久链接，更新status */
	if (reqInfo.status == 200)
	{
		fprintf(stderr,"check resource ......\n");
		if ((resource = Check_Resource(&reqInfo))<0)
		{
			if (errno == EACCES)
				reqInfo.status = 401;
			else
				reqInfo.status = 404;
		}
	}

	/* 输出HTTP响应报文，当类型为FULL时 */
	if (reqInfo.type = FULL)
		Output_HTTP_Headers(connId,&reqInfo);
	/* 响应HTTP请求 */
	if (reqInfo.status == 200)   /*200,请求成功 */
	{
		if (Return_Resource(connId,resource))
			Error_Quit("return resource Error");
	}
	else
	{
		Return_Err_Msg(connId,&reqInfo);
	}
	
	if (resource > 0)
		if (close(resource) < 0)
			Error_Quit("Close resource Error");
	fprintf(stderr,"Free ReqInfo....\n");
	FreeReqInfo(&reqInfo);
}
