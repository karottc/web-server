/*
 * =====================================================================================
 *
 *       Filename:  assist_cy.c
 *
 *    Description:  对assist_cy.h函数的实现
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 12时15分16秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang（陈杨）
 *        Company:  
 *
 * =====================================================================================
 */
#include "utility_cy.h"
#include "assist_cy.h"

void
Error_Quit(char *msg)
{
	fprintf(stderr,"Web Server Error: %s\n",msg);
	exit(EXIT_FAILURE);
}

void
Read_Line(int connId,char *buffer,int max_len)
{
	int n,rc;
	char c;

	for (n=1;n<max_len;n++)  
	{
		if ((rc = read(connId,&c,1))==1)   /* 读入一个字符 */
		{
			*buffer++ = c;
			if (c == '\n')
				break;
		}
		else if (rc == 0)
		{
			if (n==1)
				return 0;
			else
				break;
		}
		else
		{
			if (errno == EINTR)
				continue;
			else
				Error_Quit("Read_Line Error!!\n");
		}
	}
	*buffer = 0;
	return n;
}

void
Write_Line(int connId,char *buffer,int n)
{
	int nleft;
	int nwritten;
	
	nleft = n;

	while (nleft > 0)
	{
		if ((nwritten = write(connId,buffer,nleft))<=0)
		{
			if (errno == EINTR)
				nwritten = 0;
			else
				Error_Quit("Write_Line Error\n");
		}
		nleft -= nwritten;
		buffer += nwritten;
	}
	return n;
}



void
Trim(char *buffer)
{
	int n = strlen(buffer) - 1;
	while (!isalnum(buffer[n])&&n>=0)
		buffer[n--] = '\0';

	return 0;
}

int
Str_Upper(char *buffer)
{
	while (*buffer)
	{
		*buffer = toupper(*buffer);
		++buffer;
	}

}
