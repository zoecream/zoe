/*========================================*\
    文件 : pkg.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>

#include <log.h>
#include <ini.h>
#include <mmp.h>
#include <dbs.h>

/*========================================*\
    功能 : 报文入口函数
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)渠道类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgInit(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *lnkcode;
	if(strcmp(argument,"client")==0)
		result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
	else
	if(strcmp(argument,"server")==0)
		result=fmmpRefGet("pSrvLnkCode",0,&lnkcode,0);
	if(result==-1)
		return -1;

	if(strcmp(lnkcode,"001")==0)
	{
		result=fmmpValSet("pkey",0,"ABCDEFGH",0);
		if(result==-1)
			return -1;
		result=fmmpValSet("pvec",0,"abcdefgh",0);
		if(result==-1)
			return -1;

		result=fmmpValSet("pCliTrnCode",0,*fmldata+4,3);
		if(result==-1)
			return -1;
		*size-=7;
		memcpy(*tmpdata,*fmldata+7,*size);

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}
	else
	if(strcmp(lnkcode,"002")==0)
	{
	}

	return 0;
}

/*========================================*\
    功能 : 报文出口函数
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)渠道类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgFree(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *lnkcode;
	if(strcmp(argument,"client")==0)
		result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
	else
	if(strcmp(argument,"server")==0)
		result=fmmpRefGet("pSrvLnkCode",0,&lnkcode,0);
	if(result==-1)
		return -1;

	if(strcmp(lnkcode,"001")==0)
	{
		sprintf(*tmpdata,"%04d",*size);
		result=fmmpValGet("pCliTrnCode",0,*tmpdata+4,0);
		if(result==-1)
			return -1;
		memcpy(*tmpdata+7,*fmldata,*size);
		*size+=7;

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}
	else
	if(strcmp(lnkcode,"002")==0)
	{
	}

	return 0;
}

/*========================================*\
    功能 : 报文模块实验
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)拼接(cat)/裁剪(cut)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgSomething(char **fmldata,char **tmpdata,int *size,char *argument)
{
	if(strcmp(argument,"cat")==0)
	{
		(*fmldata)[*size+0]='a';
		(*fmldata)[*size+1]='\0';
		*size+=1;
	}
	else
	if(strcmp(argument,"cut")==0)
	{
		(*fmldata)[*size-1]='\0';
		*size-=-1;
	}

	return 0;
}
