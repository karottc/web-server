/*
 * =====================================================================================
 *
 *       Filename:  webserver_cy.c
 *
 *    Description:  这个是web server 的主文件
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 07时51分25秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang（陈杨）
 *        Company:  
 *
 * =====================================================================================
 */
#include "utility_cy.h"
#include "dealRequest_cy.h"

int main(void)
{
	int webserver_port = 8080;
	int listenId,connId;
	pid_t pid;
	
	int forkNumber = 0; /* 计算fork的个数 */
	int forkNumber2 = 0; /* 有pid号的fork*/
	
	struct sockaddr_in serveraddr;    /* web server 的地址结构 */
	fprintf(stderr,"Please enter a web server port number(5000~49152): ");
	scanf("%d",&webserver_port);

	/* 将serveraddr 全部初始化为0 */
	memset(&serveraddr,0,sizeof(serveraddr));

	/* 创建一个套接字，为了listen */
	fprintf(stderr,"Create a listen socket\n");
	if ((listenId=socket(AF_INET,SOCK_STREAM,0))<0)
		Error_Quit("Can't Create Listen Socket!!");
	fprintf(stderr,"--------listennId = %d\n",listenId);

	/* 初始化serveraddr */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(webserver_port);
	
	/* 绑定socket地址 */
	fprintf(stderr,"bind socket \n");
	if (bind(listenId,(struct sockaddr *)&serveraddr,sizeof(serveraddr))<0)
		Error_Quit("Can't bind socket\n");

	/* 调用listen套接字 */
	fprintf(stderr,"Call to listen socket\n");
	if (listen(listenId,LISTENQ)<0)
		Error_Quit("Can't call to listen socket\n");

	/* 等待用户请求并响应 */
	while (1)
	{
		/* 等待用户请求 */
		fprintf(stderr,"call to accept \n");
		if ((connId = accept(listenId,NULL,NULL))<0)
			Error_Quit("call to listen failure");

		/* 为服务链接创建一个子进程 */
		fprintf(stderr,"fork a child process %d\n",++forkNumber);
		if ((pid = fork()) == 0)
		{
			fprintf(stderr,"fork a clild process----%d , pid = %d\n",++forkNumber2,pid);
			if (close(listenId)< 0)
				Error_Quit("Close listen socket Error in child");
			/* 处理用户请求 */
			fprintf(stderr,"deal with client request \n");
			deal_Request(connId);
		/* 	fprintf(stderr,"deal_Request end......\n");   */

			fprintf(stderr,"close connection socket and exit......\n");
			if (close(connId)<0)
				Error_Quit("close connetion Error");
		/* 	fprintf(stderr,"before exit........\n");   */
			exit(EXIT_SUCCESS);  
		/*	fprintf(stderr,"after exit........\n");     */
		}
		
		fprintf(stderr,"close connection socket and clean up child process....\n");
		if (close(connId)<0)
			Error_Quit("Close socket Error in parent");
		/* 等待子进程结束 */
		fprintf(stderr,"waitpid.........\n");
		waitpid(-1,NULL,WNOHANG);
	}

	return EXIT_FAILURE;
}
