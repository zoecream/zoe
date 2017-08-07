/*========================================*\
    文件 : log.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <log.h>

int main(void)
{
	int result;

	char logpath[64];
	sprintf(logpath,"%s/000/log/%s-zoe.log",getenv("BUSINESS"),flogDate(0));
	result=flogMove(2,logpath);
	if(result==-1)
		return -1;

	flogAnyhow("#[%s][%s]接收报文",flogDate(1),flogTime(1));
	flogAnyhow("#[%s][%s]发送报文",flogDate(1),flogTime(1));

	sleep(1);

	char logtmppath[64];
	sprintf(logtmppath,"%s/000/log/%s-001-10000.tmp",getenv("BUSINESS"),flogDate(0));
	result=flogInit(logtmppath);
	if(result==-1)
	{
		fprintf(stderr,"flogInit failed!\n");
		return -1;
	}

	flogAnyhow("#[%s][%s]接收报文",flogDate(1),flogTime(1));
	flogAnyhow("#[%s][%s]发送报文",flogDate(1),flogTime(1));

	flogAnyhow("%s",flogDate(0));
	flogAnyhow("%s",flogDate(1));
	flogAnyhow("%s",flogDate(2));
	flogAnyhow("%s",flogDate(3));

	flogAnyhow("%s",flogTime(0));
	flogAnyhow("%s",flogTime(1));
	flogAnyhow("%s",flogTime(2));
	flogAnyhow("%s",flogTime(3));

	char logfmlpath[64];
	sprintf(logfmlpath,"%s/000/log/%s-001-A01.log",getenv("BUSINESS"),flogDate(0));
	result=flogFree(logfmlpath,logtmppath);
	if(result==-1)
	{
		fprintf(stderr,"flogFree failed!\n");
		return -1;
	}

	return 0;
}
