/*
 * =====================================================================================
 *
 *       Filename:  utility_cy.h
 *
 *    Description:  该文件包含各种有用的头文件
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 07时55分43秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang (陈杨)
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef UTILITY_CY_H
#define UTILITY_CY_H

#include <stdio.h>   /* base I/O */
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <ctype.h>
#include <errno.h>    /* for errno eintr */
#include <sys/socket.h> /* socketed definition */
#include <sys/types.h> /* socket types */
#include <sys/wait.h> /* for waitpid */
#include <arpa/inet.h>
#include <unistd.h> /* for fork() */
#include <sys/time.h>   /* for select() */


static char server_root[1000] = "/home/administrator/httpd";
#define LISTENQ		512    /* 套接口的最大排队个数 */
#define MAX_REQ_LINE    1024   /* 最大请求行数 */

#endif
