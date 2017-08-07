/*========================================*\
    文件 : log.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>

#include <log.h>

//一次性执行.
pthread_once_t vlogOnce=PTHREAD_ONCE_INIT;
//本地化存储.
pthread_key_t vlogfp;
//本地化存储创建标志.
char vlogIsCreate;

/*========================================*\
    功能 : 打开日志
    参数 : (输入)日志文件路径
    返回 : 空
\*========================================*/
int flogInit(char *logpath)
{
	int result;

	void create(void)
	{
		pthread_key_create(&vlogfp,NULL);
		vlogIsCreate=1;
	}
	result=pthread_once(&vlogOnce,create);
	if(result!=0)
	{
		mlogError("pthread_once",result,strerror(result),"[]");
		return -1;
	}

	FILE *logfp;
	logfp=fopen(logpath,"a+");
	if(logfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",logpath);
		return -1;
	}
	result=pthread_setspecific(vlogfp,logfp);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 关闭日志
    参数 : (输入)正式日志文件路径
           (输入)临时日志文件路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int flogFree(char *fmlpath,char *tmppath)
{
	int result;

	FILE *tmpfp;
	tmpfp=(FILE*)pthread_getspecific(vlogfp);
	if(tmpfp==NULL)
		return 0;
	result=pthread_setspecific(vlogfp,NULL);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		fclose(tmpfp);
		return -1;
	}

	fflush(tmpfp);

	struct stat status;
	result=stat(tmppath,&status);
	if(result==-1)
	{
		mlogError("stat",errno,strerror(errno),"[%s]",tmppath);
		fclose(tmpfp);
		return -1;
	}

	FILE *fmlfp;
	fmlfp=fopen(fmlpath,"a");
	if(fmlfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",fmlpath);
		fclose(tmpfp);
		return -1;
	}

	char *text;
	text=(char*)malloc(status.st_size);
	if(text==NULL)
	{
		mlogError("malloc",0,"0","[%d]",status.st_size);
		fclose(fmlfp);
		fclose(tmpfp);
		return -1;
	}
	bzero(text,status.st_size);

	result=fseek(tmpfp,0,SEEK_SET);
	if(result==-1)
	{
		mlogError("fseek",errno,strerror(errno),"[]");
		free(text);
		fclose(fmlfp);
		fclose(tmpfp);
		return -1;
	}

	int remain;
	int record;

	remain=status.st_size;
	record=0;
	while(remain>0)
	{
		result=fread(text+record,1,remain,tmpfp);
		if(ferror(tmpfp)!=0)
		{
			mlogError("fread",errno,strerror(errno),"[]");
			free(text);
			fclose(fmlfp);
			fclose(tmpfp);
			return -1;
		}
		remain-=result;
		record+=result;
	}

	remain=status.st_size;
	record=0;
	while(remain>0)
	{
		result=fwrite(text+record,1,remain,fmlfp);
		if(ferror(tmpfp)!=0)
		{
			mlogError("fwrite",errno,strerror(errno),"[]");
			free(text);
			fclose(fmlfp);
			fclose(tmpfp);
			return -1;
		}
		remain-=result;
		record+=result;
	}

	free(text);
	fclose(fmlfp);
	fclose(tmpfp);
	unlink(tmppath);

	return 0;
}

/*========================================*\
    功能 : 转移日志
    参数 : (输入)日志文件描述
           (输入)日志文件路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int flogMove(int logfd,char *logpath)
{
	int result;

	FILE *logfp;
	logfp=fopen(logpath,"a");
	if(logfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",logpath);
		return -1;
	}
	result=dup2(fileno(logfp),logfd);
	if(result==-1)
	{
		mlogError("dup2",errno,strerror(errno),"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 打印日志(任何版本都打印)
    参数 : (输入)格式化字符串
    返回 : 空
\*========================================*/
void flogAnyhow(char *format,...)
{
	FILE *fp;
	fp=(FILE*)pthread_getspecific(vlogfp);
	if(fp!=NULL&&vlogIsCreate==1)
	{
		va_list argument;
		va_start(argument,format);
		vfprintf(fp,format,argument);
		va_end(argument);
		fprintf(fp,"\n");

		#ifndef NDEBUG
		fflush(fp);
		#endif
	}
	else
	{
		va_list argument;

		va_start(argument,format);
		vfprintf(stdout,format,argument);
		va_end(argument);
		fprintf(stdout,"\n");
		fflush(stdout);

		va_start(argument,format);
		vfprintf(stderr,format,argument);
		va_end(argument);
		fprintf(stderr,"\n");
		fflush(stderr);
	}
	return;
}

/*========================================*\
    功能 : 打印日志(调试版本才打印)
    参数 : (输入)格式化字符串
    返回 : 空
\*========================================*/
void flogDepend(char *format,...)
{
	#ifndef NDEBUG
	FILE *fp;
	fp=(FILE*)pthread_getspecific(vlogfp);
	if(fp!=NULL&&vlogIsCreate==1)
	{
		va_list argument;
		va_start(argument,format);
		vfprintf(fp,format,argument);
		va_end(argument);
		fprintf(fp,"\n");

		fflush(fp);
	}
	else
	{
		va_list argument;

		va_start(argument,format);
		vfprintf(stdout,format,argument);
		va_end(argument);
		fprintf(stdout,"\n");
		fflush(stdout);

		va_start(argument,format);
		vfprintf(stderr,format,argument);
		va_end(argument);
		fprintf(stderr,"\n");
		fflush(stderr);
	}
	#endif
	return;
}

/*========================================*\
    功能 : 获取日期
    参数 : (输入)日期类型 (无分隔符/4位年份)0
                          (有分隔符/4位年份)1
                          (无分隔符/2位年份)2
                          (有分隔符/2位年份)3
    返回 : 日期
\*========================================*/
char *flogDate(int type)
{
	time_t stamp;
	stamp=time(NULL);
	if(type==0)
	{
		static char data[16];
		strftime(data,sizeof(data),"%Y%m%d",localtime(&stamp));
		return data;
	}
	else
	if(type==1)
	{
		static char data[16];
		strftime(data,sizeof(data),"%Y-%m-%d",localtime(&stamp));
		return data;
	}
	else
	if(type==2)
	{
		static char data[16];
		strftime(data,sizeof(data),"%y%m%d",localtime(&stamp));
		return data;
	}
	else
	if(type==3)
	{
		static char data[16];
		strftime(data,sizeof(data),"%y-%m-%d",localtime(&stamp));
		return data;
	}
	return NULL;
}

/*========================================*\
    功能 : 获取时间
    参数 : (输入)时间类型 (无分隔符/秒)0
                          (有分隔符/秒)1
                          (无分隔符/微秒)2
                          (有分隔符/微秒)3
    返回 : 时间
\*========================================*/
char *flogTime(int type)
{
	time_t stamp;
	stamp=time(NULL);
	if(type==0)
	{
		static char data[16];
		strftime(data,sizeof(data),"%H%M%S",localtime(&stamp));
		return data;
	}
	else
	if(type==1)
	{
		static char data[16];
		strftime(data,sizeof(data),"%H:%M:%S",localtime(&stamp));
		return data;
	}
	else
	if(type==2)
	{
		static char data[16];
		int result=strftime(data,sizeof(data),"%H%M%S",localtime(&stamp));
		struct timeval tv;
		gettimeofday(&tv,NULL);
		sprintf(data+result,"%06d",tv.tv_usec);
		return data;
	}
	else
	if(type==3)
	{
		static char data[16];
		int result=strftime(data,sizeof(data),"%H:%M:%S",localtime(&stamp));
		struct timeval tv;
		gettimeofday(&tv,NULL);
		sprintf(data+result,":%06d",tv.tv_usec);
		return data;
	}
	return NULL;
}
