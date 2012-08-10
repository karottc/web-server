/*
 * =====================================================================================
 *
 *       Filename:  assist_cy.h
 *
 *    Description:  提供一些辅助函数
 *
 *        Version:  1.0
 *        Created:  2011年05月17日 12时12分09秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  chen yang（陈杨）
 *        Company:  
 *
 * =====================================================================================
 */
#ifndef ASSIST_CY_H
#define ASSIST_CY_H


/* 输出错误信息并退出程序 */
void Error_Quit(char *msg);
/* 从socket读入一行 */
void Read_Line(int connId,char *buffer,int max_len);
/* 写入一行到socket */
void Write_Line(int connId,char *buffer,int n);
/* 处理buffer的数据 移除空白*/
void Trim(char *buffer);
/* 将字母转换成大写 */
int Str_Upper(char *buffer);

#endif
