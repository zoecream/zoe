/*========================================*\
    文件 : log.h
    作者 : 陈乐群
\*========================================*/

#ifndef __LOG__
#define __LOG__

#include <pthread.h>
#include <sys/syscall.h>

/*========================================*\
    功能 : 打开日志
    参数 : (输入)日志文件路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int flogInit(char *logpath);
/*========================================*\
    功能 : 关闭日志
    参数 : (输入)正式日志文件路径
           (输入)临时日志文件路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int flogFree(char *fmlpath,char *tmppath);

/*========================================*\
    功能 : 打印日志(任何版本都打印)
    参数 : (输入)格式化字符串
    返回 : 空
\*========================================*/
void flogAnyhow(char *format,...);
/*========================================*\
    功能 : 打印日志(调试版本才打印)
    参数 : (输入)格式化字符串
    返回 : 空
\*========================================*/
void flogDepend(char *format,...);

/*========================================*\
    功能 : 获取日期
    参数 : (输入)日期类型 (无分隔符/4位年份)0
                          (有分隔符/4位年份)1
                          (无分隔符/2位年份)2
                          (有分隔符/2位年份)3
    返回 : 日期
\*========================================*/
char *flogDate(int type);
/*========================================*\
    功能 : 获取时间
    参数 : (输入)时间类型 (无分隔符/秒)0
                          (有分隔符/秒)1
                          (无分隔符/微秒)2
                          (有分隔符/微秒)3
    返回 : 时间
\*========================================*/
char *flogTime(int type);

#define mlogDebug(format,argument...)\
	flogDepend("=DEBUG= [%s][%s]\n=DEBUG= [%s][%d]\n=DEBUG= [%05d][%08X]\n=DEBUG= "format"\n",\
		flogDate(1),flogTime(1),\
		__FILE__,__LINE__,\
		syscall(SYS_gettid),pthread_self(),\
		##argument)

#define mlogError(function,code,info,format,argument...)\
	flogAnyhow("=ERROR= [%s][%s]\n=ERROR= [%s][%d]\n=ERROR= [%05d][%08X]\n=ERROR= [%s][%s][%d][%s]\n=ERROR= "format"\n",\
		flogDate(1),flogTime(1),\
		__FILE__,__LINE__,\
		syscall(SYS_gettid),pthread_self(),\
		__FUNCTION__,function,code,info,\
		##argument)

#endif
