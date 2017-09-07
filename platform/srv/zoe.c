/*========================================*\
	文件 : zoe.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <setjmp.h>

#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <log.h>
#include <mmp.h>
#include <pkg.h>
#include <dbs.h>

//业务代码.
char vzoeBsnCode[3+1];
//渠道信息列表.
struct tzoeLnkInfo
{
	//渠道代码.
	char lnkcode[3+1];
	//渠道类型.
	char lnktype;
	//渠道模式.
	char lnkmode;
	//渠道主机.
	char lnkhost[15+1];
	//渠道端口.
	int lnkport;
	//渠道规则.
	int lnkrule[3];
	//监听标识.
	int lisid;
}vzoeLnkList[16];
//渠道信息数量.
int vzoeLnkCount;
//交易信息列表.
struct tzoeTrnInfo
{
	//渠道代码.
	char lnkcode[3+1];
	//交易代码.
	char trncode[15+1];
	//交易时间.
	int trntime;
}vzoeTrnList[64];
//交易信息数量.
int vzoeTrnCount;

//工作线程列表.
struct tzoeThrInfo
{
	//linux线程id.
	int linuxid;
	//posix线程id.
	pthread_t posixid;
	//线程类型.
	char nature;
	//线程状态.
	char status;
	//线程开关.
	char button;
}*vzoeThrList;
//工作线程起始位置.
int vzoeThrHead;
//工作线程结束位置.
int vzoeThrTail;
//工作线程在线数量.
int vzoeThrLive;
//工作线程忙碌数量.
int vzoeThrBusy;
//工作线程最小数量.
int vzoeThrMinCnt;
//工作线程最大数量.
int vzoeThrMaxCnt;
//任务列表.
struct tzoeTskInfo
{
	//渠道代码.
	char lnkcode[3+1];
	//连接标识.
	int conid;
	//连接时间.
	time_t time;
}*vzoeTskList;
//任务列表起始位置.
int vzoeTskHead;
//任务列表结束位置.
int vzoeTskTail;
//任务列表互斥锁.
pthread_mutex_t vzoeMutex;
//任务列表非空条件变量.
pthread_cond_t vzoeCondNe;
//任务列表非满条件变量.
pthread_cond_t vzoeCondNf;

//等待时间.
int vzoeWaitTime;
//连接列表尺寸.
int vzoeLisSize;
//任务列表尺寸.
int vzoeTskSize;

//线程编号.
pthread_key_t vzoeIndex;
//内存池尺寸.
int vzoeMmpSize;
//缓冲区尺寸.
int vzoePkgSize;
//交易函数库句柄.
void *vzoeTrnHandle;
//报文函数库句柄.
void *vzoePkgHandle;
//正式报文数据.
char **vzoeFmlData;
//临时报文数据.
char **vzoeTmpData;
//报文长度.
int *vzoeSize;
//交易定时装置.
timer_t *vzoeTime;
//交易跳转位置.
jmp_buf *vzoeJump;

//批量批次数量.
int vzoeBtcCnt;
//批量并发数量.
int vzoeCncCnt;

//批量位置状态.
char *vzoeSlot;
//批量位置互斥锁.
pthread_mutex_t vzoeSlotMutex;
//批量位置条件变量.
pthread_cond_t vzoeSlotCond;

//批量空闲内存池列表.
void ***vzoeFreeList;
//批量忙碌内存池列表.
void ***vzoeBusyList;
//批量空闲内存池起始位置.
int *vzoeFreeHead;
//批量空闲内存池结束位置.
int *vzoeFreeTail;
//批量忙碌内存池起始位置.
int *vzoeBusyHead;
//批量忙碌内存池结束位置.
int *vzoeBusyTail;
//批量空闲内存池互斥锁.
pthread_mutex_t *vzoeFreeMutex;
//批量忙碌内存池互斥锁.
pthread_mutex_t *vzoeBusyMutex;
//批量空闲内存池条件变量.
pthread_cond_t *vzoeFreeCond;
//批量忙碌内存池条件变量.
pthread_cond_t *vzoeBusyCond;

/*========================================*\
    功能 : 启动服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeBoot(void);
/*========================================*\
    功能 : 停止服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeShut(void);
/*========================================*\
    功能 : 显示服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeList(void);

/*========================================*\
    功能 : 启动服务处理函数
    参数 : (输入)启动状态
           (输入)linux线程id
           (输入)posix线程id
    返回 : 空
\*========================================*/
void fzoeBootHand(int done,int linuxid,pthread_t posixid);
/*========================================*\
    功能 : 停止服务处理函数
    参数 : (输入)信号编号
           (输入)没有使用
           (输入)没有使用
    返回 : 空
\*========================================*/
void fzoeShutHand(int id,siginfo_t *siginfo,void *nothing);
/*========================================*\
    功能 : 显示服务处理函数
    参数 : (输入)信号编号
           (输入)没有使用
           (输入)没有使用
    返回 : 空
\*========================================*/
void fzoeListHand(int id,siginfo_t *siginfo,void *nothing);

/*========================================*\
    功能 : 启动服务管理线程
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeManageBoot(void);
/*========================================*\
    功能 : 启动服务工作线程
    参数 : (输入)线程信息
    返回 : 没有使用
\*========================================*/
void *fzoeEmployBoot(void *argument);

/*========================================*\
    功能 : 调用服务交易
    参数 : (输入)服务渠道代码
           (输入)服务交易代码
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fzoeSrvTrn(char *lnkcode,char *trncode);
/*========================================*\
    功能 : 调用批量交易
    参数 : (输入)批量交易代码
           (输入)批量交易函数
           (输入)批量交易参数
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fzoeBtcTrn(char *trncode,void *trnhand,void *trnpara);

int main(int argc,char *argv[])
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	struct option options[]=
	{
		{"boot",required_argument,NULL,'b'},
		{"shut",required_argument,NULL,'s'},
		{"list",required_argument,NULL,'l'},
		{"help",required_argument,NULL,'h'},
		{0,0,0,0}
	};

	int option;
	option=getopt_long(argc,argv,":b:s:l:h",options,NULL);

	switch(option)
	{
		case 'b':
		fzoeBoot();
		break;

		case 's':
		fzoeShut();
		break;

		case 'l':
		fzoeList();
		break;

		case 'h':
		case ':':
		case '?':
		case -1:
		printf("\n");
		printf("-b|--boot : 启动服务,参数:业务.\n");
		printf("-s|--shut : 停止服务,参数:业务.\n");
		printf("-l|--list : 显示服务,参数:业务.\n");
		printf("\n");
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 启动服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeBoot(void)
{
	int result;

	if(signal(SIGCHLD,SIG_IGN)==SIG_ERR)
	{
		mlogError("signal",errno,strerror(errno),"");
		return;
	}

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(optarg,"all")!=0&&strcmp(optarg,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vzoeBsnCode,bsndirent[i]->d_name);
		printf("启动业务[%s]\n",vzoeBsnCode);

		char logpath[64];
		sprintf(logpath,"%s/%s/log/%s-zoe.log",getenv("BUSINESS"),vzoeBsnCode,flogDate(0));
		result=flogMove(2,logpath);
		if(result==-1)
			continue;

		char lockpath[64];
		sprintf(lockpath,"%s/%s/.zoelock",getenv("BUSINESS"),vzoeBsnCode);
		char fifopath[64];
		sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),vzoeBsnCode);

		int lockid;
		lockid=open(lockpath,O_RDONLY,0600);
		if(lockid==-1&&errno!=ENOENT)
		{    
			mlogError("open",errno,strerror(errno),"[%s]",lockpath);
			continue;
		}    
		else
		if(lockid!=-1)
		{
			char linuxid[5+1];
			bzero(linuxid,sizeof(linuxid));
			result=read(lockid,linuxid,sizeof(linuxid)-1);
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				continue;
			}
			close(lockid);

			result=sigqueue(atoi(linuxid),0,(union sigval)0);
			if(result==-1&&errno!=ESRCH)
			{
				mlogError("sigqueue",errno,strerror(errno),"[%d]",atoi(linuxid));
				continue;
			}
			else
			if(result==-1&&errno==ESRCH)
			{
				unlink(fifopath);
				unlink(lockpath);
			}
			else
			if(result==0)
			{
				printf("业务已启动\n");
				continue;
			}
		}

		lockid=open(lockpath,O_CREAT|O_RDWR,0600);
		if(lockid==-1)
		{
			mlogError("open",errno,strerror(errno),"[%s]",lockid);
			continue;
		}
		struct flock lock;
		bzero(&lock,sizeof(lock));
		lock.l_whence=SEEK_SET;
		lock.l_type=F_WRLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{
			mlogError("fcntl",errno,strerror(errno),"");
			close(lockid);
			continue;
		}
		if(result==-1&&errno==EAGAIN)
		{
			printf("操作繁忙中\n");
			close(lockid);
			continue;
		}

		result=mkfifo(fifopath,0600);
		if(result==-1&&errno!=EEXIST)
		{
			mlogError("mkfifo",errno,strerror(errno),"[%s]",fifopath);
			goto unlock;
		}
		if(result==-1&&errno==EEXIST)
		{
			unlink(fifopath);
			result=mkfifo(fifopath,0600);
			if(result==-1)
			{
				mlogError("mkfifo",errno,strerror(errno),"[%s]",fifopath);
				goto unlock;
			}
		}

		int pid;
		pid=fork();
		if(pid==-1)
		{
			mlogError("fork",errno,strerror(errno),"");
			goto unlock;
		}
		else
		if(pid>0)
		{
			int fifoid;
			fifoid=open(fifopath,O_RDONLY);
			if(fifoid==-1)
			{
				mlogError("open",errno,strerror(errno),"[%s]",fifopath);
				goto unlock;
			}
			int done;
			result=read(fifoid,&done,sizeof(done));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				close(fifoid);
				goto unlock;
			}
			int linuxid;
			result=read(fifoid,&linuxid,sizeof(linuxid));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				close(fifoid);
				goto unlock;
			}
			pthread_t posixid;
			result=read(fifoid,&posixid,sizeof(posixid));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				close(fifoid);
				goto unlock;
			}
			close(fifoid);

			if(done==0)
			{
				char id[13+1];
				sprintf(id,"%05d%08X",linuxid,posixid);
				result=write(lockid,id,sizeof(id)-1);
				if(result==-1)
				{
					mlogError("write",errno,strerror(errno),"");
					goto unlock;
				}
			}
		}
		else
		if(pid==0)
		{
			free(bsndirent);
			fzoeManageBoot();
		}

		unlock:
		lock.l_type=F_UNLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1)
		{
			mlogError("fcntl",errno,strerror(errno),"");
		}
		close(lockid);
	}

	return;
}

/*========================================*\
    功能 : 停止服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeShut(void)
{
	int result;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(optarg,"all")!=0&&strcmp(optarg,bsndirent[i]->d_name)!=0)
			continue;

		char bsncode[3+1];
		strcpy(bsncode,bsndirent[i]->d_name);
		printf("停止业务[%s]\n",bsncode);

		char logpath[64];
		sprintf(logpath,"%s/%s/log/%s-zoe.log",getenv("BUSINESS"),bsncode,flogDate(0));
		result=flogMove(2,logpath);
		if(result==-1)
			continue;

		char lockpath[64];
		sprintf(lockpath,"%s/%s/.zoelock",getenv("BUSINESS"),bsncode);
		char fifopath[64];
		sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),bsncode);

		int lockid;
		lockid=open(lockpath,O_RDWR,0600);
		if(lockid==-1&&errno!=ENOENT)
		{
			mlogError("open",errno,strerror(errno),"[%s]",lockpath);
			continue;
		}
		else
		if(lockid==-1&&errno==ENOENT)
		{
			printf("业务未启动\n");
			continue;
		}

		struct flock lock;
		bzero(&lock,sizeof(lock));
		lock.l_whence=SEEK_SET;
		lock.l_type=F_WRLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{    
			mlogError("fcntl",errno,strerror(errno),"");
			close(lockid);
			continue;
		}    
		if(result==-1&&errno==EAGAIN)
		{
			printf("操作繁忙中\n");
			close(lockid);
			continue;
		}

		char linuxid[5+1];
		bzero(linuxid,sizeof(linuxid));
		result=read(lockid,linuxid,sizeof(linuxid)-1);
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto unlock;
		}
		char posixid[8+1];
		bzero(posixid,sizeof(posixid));
		result=read(lockid,posixid,sizeof(posixid)-1);
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto unlock;
		}

		result=sigqueue(atoi(linuxid),SIGUSR1,(union sigval)0);
		if(result==-1&&errno!=ESRCH)
		{
			mlogError("sigqueue",errno,strerror(errno),"[%d]",atoi(linuxid));
			goto unlock;
		}
		if(result==-1&&errno==ESRCH)
		{
			printf("业务未启动\n");
			goto unlock;
		}

		int fifoid;
		fifoid=open(fifopath,O_RDONLY);
		if(fifoid==-1)
		{
			mlogError("open",errno,strerror(errno),"[%s]",fifopath);
			goto unlock;
		}

		int done;
		result=read(fifoid,&done,sizeof(done));
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			close(fifoid);
			goto unlock;
		}

		unlock:
		lock.l_type=F_UNLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1)
			mlogError("fcntl",errno,strerror(errno),"");
		close(lockid);

		unlink(fifopath);
		unlink(lockpath);
	}

	return;
}

/*========================================*\
    功能 : 显示服务
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeList(void)
{
	int result;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(optarg,"all")!=0&&strcmp(optarg,bsndirent[i]->d_name)!=0)
			continue;

		char bsncode[3+1];
		strcpy(bsncode,bsndirent[i]->d_name);
		printf("显示业务[%s]\n",bsncode);

		char logpath[64];
		sprintf(logpath,"%s/%s/log/%s-zoe.log",getenv("BUSINESS"),bsncode,flogDate(0));
		result=flogMove(2,logpath);
		if(result==-1)
			continue;

		char lockpath[64];
		sprintf(lockpath,"%s/%s/.zoelock",getenv("BUSINESS"),bsncode);
		char fifopath[64];
		sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),bsncode);

		int lockid;
		lockid=open(lockpath,O_RDWR,0600);
		if(lockid==-1&&errno!=ENOENT)
		{
			mlogError("open",errno,strerror(errno),"[%s]",lockpath);
			continue;
		}
		else
		if(lockid==-1&&errno==ENOENT)
		{
			printf("业务未启动\n");
			continue;
		}

		struct flock lock;
		bzero(&lock,sizeof(lock));
		lock.l_whence=SEEK_SET;
		lock.l_type=F_WRLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1&&errno!=EAGAIN)
		{    
			mlogError("fcntl",errno,strerror(errno),"");
			close(lockid);
			continue;
		}    
		if(result==-1&&errno==EAGAIN)
		{
			printf("操作繁忙中\n");
			close(lockid);
			continue;
		}

		char linuxid[5+1];
		bzero(linuxid,sizeof(linuxid));
		result=read(lockid,linuxid,sizeof(linuxid)-1);
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto unlock;
		}
		char posixid[8+1];
		bzero(posixid,sizeof(posixid));
		result=read(lockid,posixid,sizeof(posixid)-1);
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto unlock;
		}

		result=sigqueue(atoi(linuxid),SIGUSR2,(union sigval)0);
		if(result==-1&&errno!=ESRCH)
		{
			mlogError("sigqueue",errno,strerror(errno),"[%d]",atoi(linuxid));
			goto unlock;
		}
		if(result==-1&&errno==ESRCH)
		{
			printf("业务未启动\n");
			goto unlock;
		}

		char data[32768];
		int size=0;

		int index=0;
		int count=0;

		int fifoid;
		fifoid=open(fifopath,O_RDONLY);
		if(fifoid==-1)
		{
			mlogError("open",errno,strerror(errno),"[%s]",fifopath);
			goto unlock;
		}

		while(1)
		{
			int linuxid;
			result=read(fifoid,&linuxid,sizeof(linuxid));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				break;
			}
			if(linuxid==-1)
				break;

			pthread_t posixid;
			result=read(fifoid,&posixid,sizeof(posixid));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				break;
			}
			unsigned char nature;
			result=read(fifoid,&nature,sizeof(nature));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				break;
			}
			unsigned char status;
			result=read(fifoid,&status,sizeof(status));
			if(result==-1)
			{
				mlogError("read",errno,strerror(errno),"");
				break;
			}

			if(nature!=0X7F)
				index=0;
			index++;
			count++;
			if(status==0X00&&nature==0X7F)
				size+=sprintf(data+size,"\033[0;32m工作线程[%05d][%08X]-序号[%05d]-类型[普通]-状态[空闲]\033[0;37m\n",linuxid,posixid,index);
			else
			if(status==0X01&&nature==0X7F)
				size+=sprintf(data+size,"\033[0;31m工作线程[%05d][%08X]-序号[%05d]-类型[普通]-状态[忙碌]\033[0;37m\n",linuxid,posixid,index);
			else
			if(status==0X00&&nature!=0X7F)
				size+=sprintf(data+size,"\033[0;32m工作线程[%05d][%08X]-序号[%05d]-类型[批量]-状态[空闲]\033[0;37m\n",linuxid,posixid,index);
			else
			if(status==0X01&&nature!=0X7F)
				size+=sprintf(data+size,"\033[0;31m工作线程[%05d][%08X]-序号[%05d]-类型[批量]-状态[忙碌]\033[0;37m\n",linuxid,posixid,index);
		}
		close(fifoid);

		printf("\033[0;31m管理线程[%s][%s]-总数[%05d]\033[0;37m\n",linuxid,posixid,count);
		printf("%s",data);

		unlock:
		lock.l_type=F_UNLCK;
		result=fcntl(lockid,F_SETLK,&lock);
		if(result==-1)
			mlogError("fcntl",errno,strerror(errno),"");
		close(lockid);
	}

	return;
}

/*========================================*\
    功能 : 启动服务处理函数
    参数 : (输入)启动状态
           (输入)linux线程id
           (输入)posix线程id
    返回 : 空
\*========================================*/
void fzoeBootHand(int done,int linuxid,pthread_t posixid)
{
	int result;

	char fifopath[64];
	sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),vzoeBsnCode);
	int fifoid;
	fifoid=open(fifopath,O_WRONLY);
	if(fifoid==-1)
	{
		mlogError("open",errno,strerror(errno),"[%s]",fifopath);
		return;
	}
	result=write(fifoid,&done,sizeof(done));
	if(result==-1)
	{
		mlogError("write",errno,strerror(errno),"");
		close(fifoid);
		return;
	}
	result=write(fifoid,&linuxid,sizeof(linuxid));
	if(result==-1)
	{
		mlogError("write",errno,strerror(errno),"");
		close(fifoid);
		return;
	}
	result=write(fifoid,&posixid,sizeof(posixid));
	if(result==-1)
	{
		mlogError("write",errno,strerror(errno),"");
		close(fifoid);
		return;
	}
	close(fifoid);

	return;
}

/*========================================*\
    功能 : 停止服务处理函数
    参数 : (输入)信号编号
           (输入)没有使用
           (输入)没有使用
    返回 : 空
\*========================================*/
void fzoeShutHand(int id,siginfo_t *siginfo,void *nothing)
{
	int result;

	result=pthread_mutex_lock(&vzoeMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"");
		exit(-1);
	}
	while(vzoeTskHead!=vzoeTskTail)
	{
		result=pthread_cond_wait(&vzoeCondNf,&vzoeMutex);
		if(result!=0)
		{
			mlogError("pthread_cond_wait",result,strerror(result),"");
			pthread_mutex_unlock(&vzoeMutex);
			exit(-1);
		}
	}
	int i;
	for(i=0;i<vzoeThrLive;i++)
		vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].button=0X01;
	result=pthread_cond_broadcast(&vzoeCondNe);
	if(result!=0)
	{
		mlogError("pthread_cond_broadcast",result,strerror(result),"");
		pthread_mutex_unlock(&vzoeMutex);
		exit(-1);
	}
	result=pthread_mutex_unlock(&vzoeMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_unlock",result,strerror(result),"");
		exit(-1);
	}
	for(i=0;i<vzoeThrLive;i++)
	{
		result=pthread_join(vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].posixid,NULL);
		if(result!=0)
		{
			mlogError("pthread_join",result,strerror(result),"[%08X]",vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].posixid);
			exit(-1);
		}
	}

	char fifopath[64];
	sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),vzoeBsnCode);
	int fifoid;
	fifoid=open(fifopath,O_WRONLY);
	if(fifoid==-1)
	{
		mlogError("open",errno,strerror(errno),"[%s]",fifopath);
		exit(-1);
	}
	int done;
	result=write(fifoid,&done,sizeof(done));
	if(result==-1)
	{
		mlogError("write",errno,strerror(errno),"");
		close(fifoid);
		exit(-1);
	}
	close(fifoid);

	exit(0);
}

/*========================================*\
    功能 : 显示服务处理函数
    参数 : (输入)信号编号
           (输入)没有使用
           (输入)没有使用
    返回 : 空
\*========================================*/
void fzoeListHand(int id,siginfo_t *siginfo,void *nothing)
{
	int result;

	int i;
	int j;

	char fifopath[64];
	sprintf(fifopath,"%s/%s/.zoefifo",getenv("BUSINESS"),vzoeBsnCode);
	int fifoid;
	fifoid=open(fifopath,O_WRONLY);
	if(fifoid==-1)
	{
		mlogError("open",errno,strerror(errno),"[%s]",fifopath);
		return;
	}

	for(i=0;i<vzoeThrLive;i++)
	{
		result=write(fifoid,&vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].linuxid,sizeof(vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].linuxid));
		if(result==-1)
		{
			mlogError("write",errno,strerror(errno),"");
			close(fifoid);
			return;
		}
		result=write(fifoid,&vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].posixid,sizeof(vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].posixid));
		if(result==-1)
		{
			mlogError("write",errno,strerror(errno),"");
			close(fifoid);
			return;
		}
		result=write(fifoid,&vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].nature,sizeof(char));
		if(result==-1)
		{
			mlogError("write",errno,strerror(errno),"");
			close(fifoid);
			return;
		}
		result=write(fifoid,&vzoeThrList[(vzoeThrHead+i)%vzoeThrMaxCnt].status,sizeof(char));
		if(result==-1)
		{
			mlogError("write",errno,strerror(errno),"");
			close(fifoid);
			return;
		}
	}

	if(vzoeBtcCnt!=0)
	{
		for(i=0;i<vzoeBtcCnt;i++)
		{
			if(vzoeSlot[i]==0X01)
			{
				for(j=0;j<vzoeCncCnt;j++)
				{
					result=write(fifoid,&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].linuxid,sizeof(vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].linuxid));
					if(result==-1)
					{
						mlogError("write",errno,strerror(errno),"");
						close(fifoid);
						return;
					}
					result=write(fifoid,&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].posixid,sizeof(vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].posixid));
					if(result==-1)
					{
						mlogError("write",errno,strerror(errno),"");
						close(fifoid);
						return;
					}
					result=write(fifoid,&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].nature,sizeof(char));
					if(result==-1)
					{
						mlogError("write",errno,strerror(errno),"");
						close(fifoid);
						return;
					}
					result=write(fifoid,&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*i+j].status,sizeof(char));
					if(result==-1)
					{
						mlogError("write",errno,strerror(errno),"");
						close(fifoid);
						return;
					}
				}
			}
		}
	}

	result=-1;
	result=write(fifoid,&result,sizeof(result));
	if(result==-1)
	{
		mlogError("write",errno,strerror(errno),"");
		close(fifoid);
		return;
	}

	close(fifoid);
	return;
}

/*========================================*\
    功能 : 启动服务管理线程
    参数 : 空
    返回 : 空
\*========================================*/
void fzoeManageBoot(void)
{
	int result;
	int i;
	int j;

	struct sigaction action;
	bzero(&action,sizeof(action));
	action.sa_flags=0;
	action.sa_flags|=SA_RESTART;
	action.sa_flags|=SA_SIGINFO;
	action.sa_sigaction=fzoeShutHand;
	result=sigaction(SIGUSR1,&action,NULL);
	if(result==-1)
	{
		mlogError("sigaction",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	action.sa_sigaction=fzoeListHand;
	result=sigaction(SIGUSR2,&action,NULL);
	if(result==-1)
	{
		mlogError("sigaction",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	sigset_t oldset;
	result=sigprocmask(SIG_SETMASK,NULL,&oldset);
	if(result==-1)
	{
		mlogError("sigprocmask",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	sigset_t newset;
	result=sigemptyset(&newset);
	if(result==-1)
	{
		mlogError("sigemptyset",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	sigaddset(&newset,SIGUSR1);
	if(result==-1)
	{
		mlogError("sigaddset",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	result=sigaddset(&newset,SIGUSR2);
	if(result==-1)
	{
		mlogError("sigaddset",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	result=sigprocmask(SIG_SETMASK,&newset,NULL);
	if(result==-1)
	{
		mlogError("sigprocmask",errno,strerror(errno),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	char bsninipath[64];
	sprintf(bsninipath,"%s/%s/ini/bsn.ini",getenv("BUSINESS"),vzoeBsnCode);
	FILE *bsnfp;
	bsnfp=fopen(bsninipath,"r");
	if(bsnfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",bsninipath);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	while(1)
	{
		char bsnline[128];
		fgets(bsnline,sizeof(bsnline),bsnfp);
		if(ferror(bsnfp))
		{
			mlogError("fgets",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		if(feof(bsnfp))
		{
			fclose(bsnfp);
			break;
		}
		if(bsnline[0]=='#')
			continue;
		if(bsnline[0]=='\n')
			continue;

		if(strncmp(bsnline,"ThrMinCnt",9)==0)
			vzoeThrMinCnt=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"ThrMaxCnt",9)==0)
			vzoeThrMaxCnt=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"WaitTime",7)==0)
			vzoeWaitTime=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"LisSize",7)==0)
			vzoeLisSize=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"TskSize",7)==0)
			vzoeTskSize=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"MmpSize",7)==0)
			vzoeMmpSize=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"PkgSize",7)==0)
			vzoePkgSize=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"BtcCnt",6)==0)
			vzoeBtcCnt=atoi(strchr(bsnline,'=')+1);
		else
		if(strncmp(bsnline,"CncCnt",6)==0)
			vzoeCncCnt=atoi(strchr(bsnline,'=')+1);
	}

	char lnkinipath[64];
	sprintf(lnkinipath,"%s/%s/ini/lnk.ini",getenv("BUSINESS"),vzoeBsnCode);
	FILE *lnkfp;
	lnkfp=fopen(lnkinipath,"r");
	if(lnkfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",lnkinipath);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	while(1)
	{
		char lnkline[128];
		fgets(lnkline,sizeof(lnkline),lnkfp);
		if(ferror(lnkfp))
		{
			mlogError("fgets",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		if(feof(lnkfp))
		{
			fclose(lnkfp);
			break;
		}
		if(lnkline[0]=='#')
			continue;
		if(lnkline[0]=='\n')
			continue;

		if(lnkline[0]=='[')
		{
			strncpy(vzoeLnkList[vzoeLnkCount].lnkcode,lnkline+1,3);
			vzoeLnkList[vzoeLnkCount].lnkcode[3]='\0';
			vzoeLnkCount++;
		}
		else
		if(strncmp(lnkline,"LnkType",7)==0)
			vzoeLnkList[vzoeLnkCount-1].lnktype=*(strchr(lnkline,'=')+1);
		else
		if(strncmp(lnkline,"LnkMode",7)==0)
			vzoeLnkList[vzoeLnkCount-1].lnkmode=*(strchr(lnkline,'=')+1);
		else
		if(strncmp(lnkline,"LnkHost",7)==0)
		{
			strcpy(vzoeLnkList[vzoeLnkCount-1].lnkhost,strchr(lnkline,'=')+1);
			vzoeLnkList[vzoeLnkCount-1].lnkhost[strlen(vzoeLnkList[vzoeLnkCount-1].lnkhost)-1]='\0';
		}
		else
		if(strncmp(lnkline,"LnkPort",7)==0)
			vzoeLnkList[vzoeLnkCount-1].lnkport=atoi(strchr(lnkline,'=')+1);
		else
		if(strncmp(lnkline,"LnkRule",7)==0)
		{
			char *position1;
			char *position2;
			position1=strchr(lnkline,'=')+1;
			position2=strchr(lnkline,',');
			if(position2==NULL)
				vzoeLnkList[vzoeLnkCount-1].lnkrule[0]=-atoi(position1);
			else
			{
				vzoeLnkList[vzoeLnkCount-1].lnkrule[0]=atoi(position1);
				position1=position2+1;
				position2=strchr(position1,',');
				vzoeLnkList[vzoeLnkCount-1].lnkrule[1]=atoi(position1);
				position1=position2+1;
				vzoeLnkList[vzoeLnkCount-1].lnkrule[2]=atoi(position1);
			}
		}
	}

	char trninipath[64];
	sprintf(trninipath,"%s/%s/ini/trn.ini",getenv("BUSINESS"),vzoeBsnCode);
	FILE *trnfp;
	trnfp=fopen(trninipath,"r");
	if(trnfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",trninipath);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	while(1)
	{
		char trnline[128];
		fgets(trnline,sizeof(trnline),trnfp);
		if(ferror(trnfp))
		{
			mlogError("fgets",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		if(feof(trnfp))
		{
			fclose(trnfp);
			break;
		}
		if(trnline[0]=='#')
			continue;
		if(trnline[0]=='\n')
			continue;

		if(trnline[0]=='[')
		{
			strncpy(vzoeTrnList[vzoeTrnCount].lnkcode,trnline+1,4);
			strncpy(vzoeTrnList[vzoeTrnCount].trncode,trnline+5,strchr(trnline,']')-trnline-5);
			vzoeTrnList[vzoeTrnCount].trncode[strchr(trnline,']')-trnline-5]='\0';
			vzoeTrnCount++;
		}
		else
		if(strncmp(trnline,"TrnTime",7)==0)
			vzoeTrnList[vzoeTrnCount-1].trntime=atoi(strchr(trnline,'=')+1);
	}

	qsort(vzoeLnkList,vzoeLnkCount,sizeof(struct tzoeLnkInfo),(int(*)(const void*,const void*))strcmp);
	qsort(vzoeTrnList,vzoeTrnCount,sizeof(struct tzoeTrnInfo),(int(*)(const void*,const void*))strcmp);

	result=fpkgRuleInit(vzoeBsnCode);
	if(result==-1)
	{
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	char trnpath[64];
	sprintf(trnpath,"%s/%s/trn/libtrn.so",getenv("BUSINESS"),vzoeBsnCode);
	vzoeTrnHandle=dlopen(trnpath,RTLD_NOW|RTLD_GLOBAL);
	if(vzoeTrnHandle==NULL)
	{
		mlogError("dlopen",0,dlerror(),"[%s]",trnpath);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	char pkgpath[64];
	sprintf(pkgpath,"%s/%s/pkg/libpkg.so",getenv("BUSINESS"),vzoeBsnCode);
	vzoePkgHandle=dlopen(pkgpath,RTLD_NOW|RTLD_GLOBAL);
	if(vzoePkgHandle==NULL)
	{
		mlogError("dlopen",0,dlerror(),"[%s]",pkgpath);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	vzoeThrList=(struct tzoeThrInfo*)malloc(sizeof(struct tzoeThrInfo)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeThrList==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(struct tzoeThrInfo)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	vzoeTskList=(struct tzoeTskInfo*)malloc(sizeof(struct tzoeTskInfo)*vzoeTskSize);
	if(vzoeTskList==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(struct tzoeTskInfo)*vzoeTskSize);
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	result=pthread_mutex_init(&vzoeMutex,NULL);
	if(result!=0)
	{
		mlogError("pthread_mutex_init",result,strerror(result),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	result=pthread_cond_init(&vzoeCondNe,NULL);
	if(result!=0)
	{
		mlogError("pthread_cond_init",result,strerror(result),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	result=pthread_cond_init(&vzoeCondNf,NULL);
	if(result!=0)
	{
		mlogError("pthread_cond_init",result,strerror(result),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	vzoeFmlData=(char**)malloc(sizeof(char*)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeFmlData==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(char*)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	vzoeTmpData=(char**)malloc(sizeof(char*)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeTmpData==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(char*)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	vzoeSize=(int*)malloc(sizeof(int)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeSize==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(int)*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	vzoeTime=(timer_t*)malloc(sizeof(timer_t)*2*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeTime==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(timer_t)*2*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}
	vzoeJump=(sigjmp_buf*)malloc(sizeof(sigjmp_buf)*2*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
	if(vzoeJump==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(sigjmp_buf)*2*(vzoeThrMaxCnt+vzoeCncCnt*vzoeBtcCnt));
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	if(vzoeBtcCnt>0&&vzoeCncCnt>0)
	{
		vzoeSlot=(char*)calloc(sizeof(char),vzoeBtcCnt);
		if(vzoeTskList==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(char)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		result=pthread_mutex_init(&vzoeSlotMutex,NULL);
		if(result!=0)
		{
			mlogError("pthread_mutex_init",result,strerror(result),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		result=pthread_cond_init(&vzoeSlotCond,NULL);
		if(result!=0)
		{
			mlogError("pthread_cond_init",result,strerror(result),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}

		vzoeFreeList=(void***)calloc(sizeof(void**),vzoeBtcCnt);
		if(vzoeTskList==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(void**)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeBusyList=(void***)calloc(sizeof(void**),vzoeBtcCnt);
		if(vzoeTskList==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(void**)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeFreeHead=(int*)malloc(sizeof(int)*vzoeBtcCnt);
		if(vzoeFreeHead==NULL)
		{
			mlogError("malloc",0,"0","[%d]",sizeof(int)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeFreeTail=(int*)malloc(sizeof(int)*vzoeBtcCnt);
		if(vzoeFreeTail==NULL)
		{
			mlogError("malloc",0,"0","[%d]",sizeof(int)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeBusyHead=(int*)malloc(sizeof(int)*vzoeBtcCnt);
		if(vzoeBusyHead==NULL)
		{
			mlogError("malloc",0,"0","[%d]",sizeof(int)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeBusyTail=(int*)malloc(sizeof(int)*vzoeBtcCnt);
		if(vzoeBusyTail==NULL)
		{
			mlogError("malloc",0,"0","[%d]",sizeof(int)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeFreeMutex=(pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),vzoeBtcCnt);
		if(vzoeFreeMutex==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(pthread_mutex_t)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeBusyMutex=(pthread_mutex_t*)calloc(sizeof(pthread_mutex_t),vzoeBtcCnt);
		if(vzoeBusyMutex==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(pthread_mutex_t)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeFreeCond=(pthread_cond_t*)calloc(sizeof(pthread_cond_t),vzoeBtcCnt);
		if(vzoeFreeCond==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(pthread_cond_t)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		vzoeBusyCond=(pthread_cond_t*)calloc(sizeof(pthread_cond_t),vzoeBtcCnt);
		if(vzoeBusyCond==NULL)
		{
			mlogError("calloc",0,"0","[%d]",sizeof(pthread_cond_t)*vzoeBtcCnt);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
	}

	result=pthread_key_create(&vzoeIndex,NULL);
	if(result!=0)
	{
		mlogError("pthread_key_create",result,strerror(result),"");
		fzoeBootHand(-1,0,0);
		exit(-1);
	}

	for(i=0;i<vzoeThrMinCnt;i++)
	{
		result=pthread_create(&vzoeThrList[vzoeThrTail].posixid,NULL,fzoeEmployBoot,&vzoeThrList[vzoeThrTail]);
		if(result!=0)
		{
			mlogError("pthread_create",result,strerror(result),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}

		vzoeThrList[vzoeThrTail].nature=0X7F;
		vzoeThrList[vzoeThrTail].status=0X00;
		vzoeThrList[vzoeThrTail].button=0X00;

		vzoeThrTail=(vzoeThrTail+1)%vzoeThrMaxCnt;
	}
	vzoeThrLive+=vzoeThrMinCnt;
	vzoeThrBusy+=vzoeThrMinCnt;

	struct timespec timeout;
	bzero(&timeout,sizeof(timeout));
	timeout.tv_sec=vzoeWaitTime;
	int maxfd;
	maxfd=0;
	fd_set fmlfdset;
	FD_ZERO(&fmlfdset);

	for(i=0;i<vzoeLnkCount;i++)
	{
		if(vzoeLnkList[i].lnktype!='c')
			continue;
		else
		if(vzoeLnkList[i].lnktype!='s')
			;

		int lisid;
		lisid=socket(AF_INET,SOCK_STREAM,0);
		if(lisid==-1)
		{
			mlogError("socket",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		result=fcntl(lisid,F_SETFL,O_NONBLOCK);
		if(result==-1)
		{
			mlogError("fcntl",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}

		struct sockaddr_in lisaddress;
		bzero(&lisaddress,sizeof(lisaddress));
		lisaddress.sin_family=AF_INET;
		inet_pton(AF_INET,vzoeLnkList[i].lnkhost,&lisaddress.sin_addr);
		lisaddress.sin_port=htons(vzoeLnkList[i].lnkport);

		int option;
		option=1;
		result=setsockopt(lisid,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
		if(result==-1)
		{
			mlogError("setsockopt",errno,strerror(errno),"");
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		result=bind(lisid,(struct sockaddr*)&lisaddress,sizeof(struct sockaddr_in));
		if(result==-1)
		{
			mlogError("bind",errno,strerror(errno),"[%s][%d]",vzoeLnkList[i].lnkhost,vzoeLnkList[i].lnkport);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}
		result=listen(lisid,vzoeLisSize);
		if(result==-1)
		{
			mlogError("listen",errno,strerror(errno),"[%d]",vzoeLisSize);
			fzoeBootHand(-1,0,0);
			exit(-1);
		}

		vzoeLnkList[i].lisid=lisid;
		maxfd=lisid>maxfd?lisid:maxfd;
		FD_SET(lisid,&fmlfdset);
	}

	fzoeBootHand(0,syscall(SYS_gettid),pthread_self());
	result=flogMove(1,"/dev/null");
	if(result==-1)
		exit(-1);

	unsigned long long trncount=0;

	while(1)
	{
		fd_set tmpfdset;
		memcpy(&tmpfdset,&fmlfdset,sizeof(fd_set));
		int count;
		count=pselect(maxfd+1,&tmpfdset,NULL,NULL,&timeout,&oldset);
		if(count==-1&&errno!=EINTR)
		{
			mlogError("select",errno,strerror(errno),"[%d]",timeout.tv_sec);
			exit(-1);
		}
		if(count==-1&&errno==EINTR)
			continue;

		if(count==0)
		{
			if(vzoeThrBusy==0&&vzoeThrLive>vzoeThrMinCnt)
			{
				result=pthread_mutex_lock(&vzoeMutex);
				if(result!=0)
				{
					mlogError("pthread_mutex_lock",result,strerror(result),"");
					exit(-1);
				}
				for(j=0;j<vzoeThrMinCnt;j++)
					vzoeThrList[(vzoeThrHead+j)%vzoeThrMaxCnt].button=0X01;
				result=pthread_cond_broadcast(&vzoeCondNe);
				if(result!=0)
				{
					mlogError("pthread_cond_broadcast",result,strerror(result),"");
					exit(-1);
				}
				result=pthread_mutex_unlock(&vzoeMutex);
				if(result!=0)
				{
					mlogError("pthread_mutex_unlock",0,strerror(result),"");
					exit(-1);
				}

				for(j=0;j<vzoeThrMinCnt;j++)
				{
					result=pthread_join(vzoeThrList[vzoeThrHead].posixid,NULL);
					if(result!=0)
					{
						mlogError("pthread_join",result,strerror(result),"[%08X]",vzoeThrList[vzoeThrHead].posixid);
						exit(-1);
					}

					vzoeThrHead=(vzoeThrHead+1)%vzoeThrMaxCnt;
				}
				vzoeThrLive-=vzoeThrMinCnt;
			}
			continue;
		}
		else
		{
			if(vzoeThrBusy==vzoeThrLive&&vzoeThrLive<vzoeThrMaxCnt)
			{
				for(j=0;j<vzoeThrMinCnt;j++)
				{
					result=pthread_create(&vzoeThrList[vzoeThrTail].posixid,NULL,fzoeEmployBoot,&vzoeThrList[vzoeThrTail]);
					if(result!=0)
					{
						mlogError("pthread_create",result,strerror(result),"");
						exit(-1);
					}

					vzoeThrList[vzoeThrTail].nature=0X7F;
					vzoeThrList[vzoeThrTail].status=0X00;
					vzoeThrList[vzoeThrTail].button=0X00;

					vzoeThrTail=(vzoeThrTail+1)%vzoeThrMaxCnt;
				}
				vzoeThrLive+=vzoeThrMinCnt;
				vzoeThrBusy+=vzoeThrMinCnt;
			}
		}

		for(i=0;i<count;i++)
		{
			for(j=0;j<vzoeLnkCount;j++)
			{
				if(FD_ISSET(vzoeLnkList[j].lisid,&tmpfdset))
				{
					while(1)
					{
						struct sockaddr_in conaddress;
						bzero(&conaddress,sizeof(conaddress));
						int size;
						size=sizeof(conaddress);
						int conid;
						conid=accept(vzoeLnkList[j].lisid,(struct sockaddr*)&conaddress,&size);
						if(conid==-1&&errno!=EAGAIN&&errno!=ECONNABORTED)
						{
							mlogError("accept",errno,strerror(errno),"");
							exit(-1);
						}
						if(conid==-1&&(errno==EAGAIN||errno==ECONNABORTED))
							break;

						result=pthread_mutex_lock(&vzoeMutex);
						if(result!=0)
						{
							mlogError("pthread_mutex_lock",result,strerror(result),"");
							exit(-1);
						}
						if((vzoeTskTail+1)%vzoeTskSize==vzoeTskHead)
						{
							result=pthread_cond_wait(&vzoeCondNf,&vzoeMutex);
							if(result!=0)
							{
								mlogError("pthread_cond_wait",result,strerror(result),"");
								pthread_mutex_unlock(&vzoeMutex);
								exit(-1);
							}
						}
						strcpy(vzoeTskList[vzoeTskTail].lnkcode,vzoeLnkList[j].lnkcode);
						vzoeTskList[vzoeTskTail].conid=conid;
						time(&vzoeTskList[vzoeTskTail].time);
						vzoeTskTail=(vzoeTskTail+1)%vzoeTskSize;
						result=pthread_cond_signal(&vzoeCondNe);
						if(result!=0)
						{
							mlogError("pthread_cond_signal",result,strerror(result),"");
							pthread_mutex_unlock(&vzoeMutex);
							exit(-1);
						}
						result=pthread_mutex_unlock(&vzoeMutex);
						if(result!=0)
						{
							mlogError("pthread_mutex_unlock",result,strerror(result),"");
							exit(-1);
						}

						trncount++;
						if(trncount%10000==0)
							mlogDebug("%d-%s",trncount,flogTime(3));
					}
				}
			}
		}
	}
}

/*========================================*\
    功能 : 启动服务工作线程
    参数 : (输入)线程信息
    返回 : 没有使用
\*========================================*/
void *fzoeEmployBoot(void *argument)
{
	int result;

	int index;
	index=(struct tzoeThrInfo*)argument-vzoeThrList;
	vzoeThrList[index].linuxid=syscall(SYS_gettid);

	result=pthread_setspecific(vzoeIndex,&index);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"");
		exit(-1);
	}

	result=fmmpInit(vzoeMmpSize);
	if(result==-1)
		exit(-1);

	result=fdbsInit(vzoeBsnCode);
	if(result==-1)
		exit(-1);
	mlogDebug("");

	vzoeFmlData[index]=(char*)malloc(vzoePkgSize);
	if(vzoeFmlData[index]==NULL)
	{
		mlogError("malloc",0,"0","[%d]",vzoePkgSize);
		exit(-1);
	}
	vzoeTmpData[index]=(char*)malloc(vzoePkgSize);
	if(vzoeTmpData[index]==NULL)
	{
		mlogError("malloc",0,"0","[%d]",vzoePkgSize);
		exit(-1);
	}

	struct sigevent event;
	event.sigev_notify=SIGEV_SIGNAL|SIGEV_THREAD_ID;
	event.sigev_signo=SIGALRM;
	event._sigev_un._tid=vzoeThrList[index].linuxid;
	event.sigev_value.sival_int=0;
	result=timer_create(CLOCK_MONOTONIC,&event,&vzoeTime[index*2+0]);
	if(result==-1)
	{
		mlogError("timer_create",errno,strerror(errno),"");
		exit(-1);
	}
	event.sigev_value.sival_int=1;
	result=timer_create(CLOCK_MONOTONIC,&event,&vzoeTime[index*2+1]);
	if(result==-1)
	{
		mlogError("timer_create",errno,strerror(errno),"");
		exit(-1);
	}
	struct itimerspec it;
	bzero(&it,sizeof(it));

	void sighand(int id,siginfo_t *siginfo,void *nothing)
	{
		if(id==SIGALRM)
			if(siginfo->si_value.sival_int==0)
				flogAnyhow("### 客户端交易超时 ###");
			else
			if(siginfo->si_value.sival_int==1)
				flogAnyhow("### 服务端交易超时 ###");
		else
			flogAnyhow("### 发生异常信号[%d] ###",id);
		int index;
		memcpy(&index,pthread_getspecific(vzoeIndex),sizeof(index));
		siglongjmp(vzoeJump[index*2+0],1);
	}

	struct sigaction action;
	result=sigfillset(&action.sa_mask);
	if(result==-1)
	{
		mlogError("sigfillset",errno,strerror(errno),"");
		exit(-1);
	}
	action.sa_flags=0;
	action.sa_flags|=SA_RESTART;
	action.sa_flags|=SA_SIGINFO;
	action.sa_sigaction=sighand;
	result=sigaction(SIGSEGV,&action,NULL);
	if(result==-1)
	{
		mlogError("sigaction",errno,strerror(errno),"");
		exit(-1);
	}
	result=sigaction(SIGALRM,&action,NULL);
	if(result==-1)
	{
		mlogError("sigaction",errno,strerror(errno),"");
		exit(-1);
	}

	result=sigsetjmp(vzoeJump[index*2+0],1);
	if(result!=0)
	{
		if(vzoeThrList[index].nature==0X7F)
			goto tag10ftrnerror;
		else
			goto tag20ftrnerror;
	}

	if(vzoeThrList[index].nature==0X7F)
	{
		while(1)
		{
			int error;
			error=-100;

			result=fmmpRnit(vzoeMmpSize);
			if(result==-1)
				return NULL;

			result=pthread_mutex_lock(&vzoeMutex);
			if(result!=0)
			{
				mlogError("pthread_mutex_lock",result,strerror(result),"");
				return NULL;
			}
			vzoeThrBusy--;
			while(vzoeTskHead==vzoeTskTail&&vzoeThrList[index].button!=0X01)
			{
				result=pthread_cond_wait(&vzoeCondNe,&vzoeMutex);
				if(result!=0)
				{
					mlogError("pthread_cond_wait",result,strerror(result),"");
					pthread_mutex_unlock(&vzoeMutex);
					return NULL;
				}
			}
			if(vzoeThrList[index].button==0X01)
			{
				result=pthread_mutex_unlock(&vzoeMutex);
				if(result!=0)
				{
					mlogError("pthread_mutex_unlock",result,strerror(result),"");
					return NULL;
				}

				timer_delete(vzoeTime[index*2+1]);
				timer_delete(vzoeTime[index*2+0]);
				free(vzoeTmpData[index]);
				free(vzoeFmlData[index]);
				fdbsFree();
				fmmpFree();

				return NULL;
			}
			vzoeThrBusy++;
			struct tzoeTskInfo tsk;
			memcpy(&tsk,&vzoeTskList[vzoeTskHead],sizeof(struct tzoeTskInfo));
			vzoeTskHead=(vzoeTskHead+1)%vzoeTskSize;
			result=pthread_cond_signal(&vzoeCondNf);
			if(result!=0)
			{
				mlogError("pthread_cond_signal",result,strerror(result),"");
				pthread_mutex_unlock(&vzoeMutex);
				return NULL;
			}
			result=pthread_mutex_unlock(&vzoeMutex);
			if(result!=0)
			{
				mlogError("pthread_mutex_unlock",result,strerror(result),"");
				return NULL;
			}

			vzoeThrList[index].status=0X01;

			char *trncode;
			trncode="xxx";

			char logtmppath[64];
			sprintf(logtmppath,"%s/%s/log/%s-%s-%05d.tmp",getenv("BUSINESS"),vzoeBsnCode,flogDate(0),tsk.lnkcode,vzoeThrList[index].linuxid);
			result=flogInit(logtmppath);
			if(result==-1)
				goto tag18socketclose;
			flogAnyhow("");
			flogAnyhow("#[%s]交易开始",flogTime(3));
			flogAnyhow("#linuxid[%05d]",vzoeThrList[index].linuxid);
			flogAnyhow("#posixid[%08X]",vzoeThrList[index].posixid);

			result=fmmpValSet("pBsnCode",0,vzoeBsnCode,0);
			if(result==-1)
				goto tag17flogfree;
			result=fmmpValSet("pCliLnkCode",0,tsk.lnkcode,0);
			if(result==-1)
				goto tag17flogfree;

			void *dlhand;
			dlhand=dlsym(vzoeTrnHandle,"ftrnInit");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag17flogfree;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]交易前处理开始",flogTime(3));
				result=((int(*)(void))dlhand)();
				flogAnyhow("#[%s]交易前处理结束",flogTime(3));
				if(result==-1)
					goto tag17flogfree;
			}

			struct tzoeLnkInfo *lnk;
			lnk=(struct tzoeLnkInfo*)bsearch(tsk.lnkcode,vzoeLnkList,vzoeLnkCount,sizeof(struct tzoeLnkInfo),(int(*)(const void*,const void*))strcmp);
			if(lnk==NULL)
			{
				mlogError("bsearch",0,"0","[%s][%d]",tsk.lnkcode,vzoeLnkCount);
				goto tag16ftrnfree;
			}

			if(lnk->lnkrule[0]==0)
			{
				vzoeSize[index]=read(tsk.conid,vzoeFmlData[index],vzoePkgSize);
				if(vzoeSize[index]==-1)
				{
					mlogError("read",errno,strerror(errno),"");
					goto tag16ftrnfree;
				}
			}
			else
			if(lnk->lnkrule[0]<0)
			{
				vzoeSize[index]=-lnk->lnkrule[0];

				int remain;
				remain=vzoeSize[index];
				int record;
				record=0;
				while(remain>0)
				{
					int result;
					result=read(tsk.conid,vzoeFmlData[index]+record,remain);
					if(result==-1&&errno!=EAGAIN)
					{
						mlogError("read",errno,strerror(errno),"");
						goto tag16ftrnfree;
					}
					remain-=result;
					record+=result;
				}
			}
			else
			{
				result=read(tsk.conid,vzoeFmlData[index],lnk->lnkrule[0]);
				if(result==-1)
				{
					mlogError("read",errno,strerror(errno),"");
					goto tag16ftrnfree;
				}
				strncpy(vzoeTmpData[index],vzoeFmlData[index]+lnk->lnkrule[1],lnk->lnkrule[2]);
				vzoeTmpData[index][lnk->lnkrule[2]]='\0';
				vzoeSize[index]=lnk->lnkrule[0]+atoi(vzoeTmpData[index]);

				int remain;
				remain=vzoeSize[index]-lnk->lnkrule[0];
				int record;
				record=0;
				while(remain>0)
				{
					int result;
					result=read(tsk.conid,vzoeFmlData[index]+lnk->lnkrule[0]+record,remain);
					if(result==-1&&errno!=EAGAIN)
					{
						mlogError("read",errno,strerror(errno),"");
						goto tag16ftrnfree;
					}
					remain-=result;
					record+=result;
				}
			}

			dlhand=dlsym(vzoePkgHandle,"fpkgInit");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag16ftrnfree;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]报文解码前处理开始",flogTime(3));
				result=((int(*)(char**,char**,int*,char*))dlhand)(&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index],"client");
				flogAnyhow("#[%s]报文解码前处理结束",flogTime(3));
				if(result==-1)
					goto tag16ftrnfree;
			}

			result=fmmpRefGet("pCliTrnCode",0,&trncode,0);
			if(result==-1)
				goto tag16ftrnfree;

			flogAnyhow("#CliLnkCode[%s]",tsk.lnkcode);
			flogAnyhow("#CliTrnCode[%s]",trncode);

			char mark[32];
			sprintf(mark,"%s_%s",tsk.lnkcode,trncode);
			struct tzoeTrnInfo *trn;
			trn=(struct tzoeTrnInfo*)bsearch(mark,vzoeTrnList,vzoeTrnCount,sizeof(struct tzoeTrnInfo),(int(*)(const void*,const void*))strcmp);
			if(trn==NULL)
			{
				flogAnyhow("### 交易代码无法识别 ###");
				trncode="xxx";
				mlogError("bsearch",0,"0","[%s][%d]",mark,vzoeLnkCount);
				goto tag16ftrnfree;
			}

			double diff;
			diff=difftime(time(NULL),tsk.time);
			if(trn->trntime!=0&&diff>=trn->trntime)
			{
				flogAnyhow("### 交易等待时间超时 ###");
				goto tag16ftrnfree;
			}

			it.it_value.tv_sec=trn->trntime==0?0:trn->trntime-diff;
			result=timer_settime(vzoeTime[index*2+0],0,&it,NULL);
			if(result==-1)
			{
				mlogError("settime",errno,strerror(errno),"");
				goto tag12setretsmmp;
			}

			result=fmmpValSet("pTrnDate",0,flogDate(0),0);
			if(result==-1)
				goto tag11timersettime;
			result=fmmpValSet("pTrnTime",0,flogTime(0),0);
			if(result==-1)
				goto tag11timersettime;

			struct sockaddr_in cliaddress;
			int size;
			size=sizeof(cliaddress);
			result=getpeername(tsk.conid,(struct sockaddr*)&cliaddress,&size);
			if(result==-1)
			{
				mlogError("getpeername",errno,strerror(errno),"");
				goto tag11timersettime;
			}

			char clihost[15+1];
			strcpy(clihost,inet_ntoa(cliaddress.sin_addr));
			flogAnyhow("#CliHost[%s]",clihost);
			result=fmmpValSet("pCliHost",0,clihost,0);
			if(result==-1)
				goto tag11timersettime;
			char cliport[5+1];
			sprintf(cliport,"%d",ntohs(cliaddress.sin_port));
			flogAnyhow("#CliPort[%s]",cliport);
			result=fmmpValSet("pCliPort",0,cliport,0);
			if(result==-1)
				goto tag11timersettime;

			flogAnyhow("");
			flogAnyhow("#[%s]报文解码开始",flogTime(3));
			result=fpkgDec(tsk.lnkcode,trncode,&vzoeFmlData[index],&vzoeTmpData[index],vzoeSize[index]);
			flogAnyhow("#[%s]报文解码结束",flogTime(3));
			if(result==-1)
				goto tag11timersettime;

			flogAnyhow("");
			flogAnyhow("#[%s]交易流程开始",flogTime(3));
			char trnname[32];
			sprintf(trnname,"ftrn%s",trncode);
			dlhand=dlsym(vzoeTrnHandle,trnname);
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"[%s]",trnname);
				goto tag11timersettime;
			}
			if(dlhand!=NULL)
				error=((int(*)(void))dlhand)();

			tag10ftrnerror:;
			if(error!=0)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]调用错误处理函数开始",flogTime(3));
				flogAnyhow("#error[%d]",error);
				dlhand=dlsym(vzoeTrnHandle,"ftrnError");
				if(dlhand==NULL)
				{
					mlogError("dlsym",0,dlerror(),"");
					goto tag11timersettime;
				}
				((void(*)(int))dlhand)(error);
				flogAnyhow("#[%s]调用错误处理函数结束",flogTime(3));
			}
			flogAnyhow("#[%s]交易流程结束",flogTime(3));

			tag11timersettime:;
			it.it_value.tv_sec=0;
			result=timer_settime(vzoeTime[index*2+0],0,&it,NULL);
			if(result==-1)
			{
				mlogError("settime",errno,strerror(errno),"");
				goto tag12setretsmmp;
			}

			tag12setretsmmp:;
			char cliretcode[4+1];
			char cliretstat[1+1];
			char cliretinfo[8+1];
			result=fmmpValGet("pCliRetCode",0,cliretcode,0);
			if(result==-1)
			{
				if(error==0)
				{
					strcpy(cliretcode,"0000");
					strcpy(cliretstat,"S");
					strcpy(cliretinfo,"交易成功");
				}
				else
				if(error<0)
				{
					strcpy(cliretcode,"9999");
					strcpy(cliretstat,"F");
					strcpy(cliretinfo,"交易失败");
				}
				fmmpValSet("pCliRetCode",0,cliretcode,0);
				fmmpValSet("pCliRetStat",0,cliretstat,0);
				fmmpValSet("pCliRetInfo",0,cliretinfo,0);
			}
			else
			{
				cliretstat[0]='\0';
				cliretinfo[0]='\0';

				char errpath[64];
				sprintf(errpath,"%s/%s/ini/ret.ini",getenv("BUSINESS"),vzoeBsnCode);
				FILE *errfp;
				errfp=fopen(errpath,"r");
				if(errfp==NULL)
				{
					mlogError("fopen",errno,strerror(errno),"[%s]",errpath);
					goto tag13fpkgenc;
				}
				while(1)
				{
					char errline[128];
					fgets(errline,sizeof(errline),errfp);
					if(ferror(errfp))
					{
						mlogError("fgets",errno,strerror(errno),"");
						goto tag13fpkgenc;
					}
					if(feof(errfp))
					{
						fclose(errfp);
						break;
					}
					if(errline[0]=='#')
						continue;
					if(errline[0]=='\n')
						continue;

					if(errline[0]=='[')
					{
						if(strncmp(errline+1,"SUCCESS",7)==0)
							strcpy(cliretstat,"S");
						else
						if(strncmp(errline+1,"FAILURE",7)==0)
							strcpy(cliretstat,"F");
						continue;
					}

					char *position;
					position=strchr(errline,'=');
					if(position==NULL)
						continue;
					*position='\0';
					if(strcmp(errline,cliretcode)!=0)
						continue;

					int size;
					size=strlen(position+1)-1;
					strncpy(cliretinfo,position+1,size);
					cliretinfo[size]='\0';

					fclose(errfp);
					break;
				}

				if(cliretinfo[0]=='\0')
				{
					strcpy(cliretstat,"U");
					strcpy(cliretinfo,"未知错误");
				}

				fmmpValSet("pCliRetStat",0,cliretstat,0);
				fmmpValSet("pCliRetInfo",0,cliretinfo,0);
			}
			flogAnyhow("#CliRetCode[%s]",cliretcode);
			flogAnyhow("#CliRetStat[%s]",cliretstat);
			flogAnyhow("#CliRetInfo[%s]",cliretinfo);

			tag13fpkgenc:;
			flogAnyhow("");
			flogAnyhow("#[%s]报文编码开始",flogTime(3));
			result=fpkgEnc(tsk.lnkcode,trncode,&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index]);
			flogAnyhow("#[%s]报文编码结束",flogTime(3));
			if(result==-1)
				goto tag14fpkgfree;

			tag14fpkgfree:;
			dlhand=dlsym(vzoePkgHandle,"fpkgFree");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag15socketwrite;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]报文编码后处理开始",flogTime(3));
				result=((int(*)(char**,char**,int*,char*))dlhand)(&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index],"client");
				flogAnyhow("#[%s]报文编码后处理结束",flogTime(3));
				if(result==-1)
					goto tag15socketwrite;
			}

			tag15socketwrite:;
			int remain;
			remain=vzoeSize[index];
			int record;
			record=0;
			while(remain>0)
			{
				result=write(tsk.conid,vzoeFmlData[index]+record,remain);
				if(result==-1&&errno!=EAGAIN)
				{
					mlogError("write",errno,strerror(errno),"");
					goto tag16ftrnfree;
				}
				remain-=result;
				record+=result;
			}

			tag16ftrnfree:;
			dlhand=dlsym(vzoeTrnHandle,"ftrnFree");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag17flogfree;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]交易后处理开始",flogTime(3));
				result=((int(*)(void))dlhand)();
				flogAnyhow("#[%s]交易后处理结束",flogTime(3));
				if(result==-1)
					goto tag17flogfree;
			}

			tag17flogfree:;
			flogAnyhow("#[%s]交易结束",flogTime(3));
			flogAnyhow("****************************************************************");
			char logfmlpath[64];
			sprintf(logfmlpath,"%s/%s/log/%s-%s-%s.log",getenv("BUSINESS"),vzoeBsnCode,flogDate(0),tsk.lnkcode,trncode);
			result=flogFree(logfmlpath,logtmppath);
			if(result==-1)
				goto tag18socketclose;

			tag18socketclose:;
			close(tsk.conid);

			tag19setstatus:;
			vzoeThrList[index].status=0X00;
		}
	}
	else
	{
		while(1)
		{
			int error;
			error=-100;

			result=fmmpRnit(vzoeMmpSize);
			if(result==-1)
				return NULL;

			void *old;
			void *new;

			fmmpSwap(NULL,&new);
			result=pthread_mutex_lock(&vzoeFreeMutex[vzoeThrList[index].nature]);
			if(result!=0)
			{
				mlogError("pthread_mutex_lock",result,strerror(result),"");
				return NULL;
			}
			vzoeFreeList[vzoeThrList[index].nature][vzoeFreeTail[vzoeThrList[index].nature]]=new;
			vzoeFreeTail[vzoeThrList[index].nature]=(vzoeFreeTail[vzoeThrList[index].nature]+1)%(vzoeCncCnt+1);
			result=pthread_cond_signal(&vzoeFreeCond[vzoeThrList[index].nature]);
			if(result!=0)
			{
				mlogError("pthread_cond_signal",result,strerror(result),"");
				pthread_mutex_unlock(&vzoeFreeMutex[vzoeThrList[index].nature]);
				return NULL;
			}
			result=pthread_mutex_unlock(&vzoeFreeMutex[vzoeThrList[index].nature]);
			if(result!=0)
			{
				mlogError("pthread_mutex_unlock",result,strerror(result),"");
				return NULL;
			}

			result=pthread_mutex_lock(&vzoeBusyMutex[vzoeThrList[index].nature]);
			if(result!=0)
			{
				mlogError("pthread_mutex_lock",result,strerror(result),"");
				return NULL;
			}
			while(vzoeBusyHead[vzoeThrList[index].nature]==vzoeBusyTail[vzoeThrList[index].nature]&&vzoeThrList[index].button!=0X01)
			{
				result=pthread_cond_wait(&vzoeBusyCond[vzoeThrList[index].nature],&vzoeBusyMutex[vzoeThrList[index].nature]);
				if(result!=0)
				{
					mlogError("pthread_cond_wait",result,strerror(result),"");
					pthread_mutex_unlock(&vzoeBusyMutex[vzoeThrList[index].nature]);
					return NULL;
				}
			}
			if(vzoeBusyHead[vzoeThrList[index].nature]==vzoeBusyTail[vzoeThrList[index].nature]&&vzoeThrList[index].button==0X01)
			{
				result=pthread_mutex_unlock(&vzoeBusyMutex[vzoeThrList[index].nature]);
				if(result!=0)
				{
					mlogError("pthread_mutex_unlock",result,strerror(result),"");
					return NULL;
				}

				result=pthread_mutex_lock(&vzoeFreeMutex[vzoeThrList[index].nature]);
				if(result!=0)
				{
					mlogError("pthread_mutex_lock",result,strerror(result),"");
					return NULL;
				}
				old=vzoeFreeList[vzoeThrList[index].nature][vzoeFreeHead[vzoeThrList[index].nature]];
				vzoeFreeHead[vzoeThrList[index].nature]=(vzoeFreeHead[vzoeThrList[index].nature]+1)%(vzoeCncCnt+1);
				result=pthread_mutex_unlock(&vzoeFreeMutex[vzoeThrList[index].nature]);
				if(result!=0)
				{
					mlogError("pthread_mutex_unlock",result,strerror(result),"");
					return NULL;
				}
				fmmpSwap(old,NULL);

				timer_delete(vzoeTime[index*2+1]);
				timer_delete(vzoeTime[index*2+0]);
				free(vzoeTmpData[index]);
				free(vzoeFmlData[index]);
				fdbsFree();
				fmmpFree();

				return NULL;
			}
			new=vzoeBusyList[vzoeThrList[index].nature][vzoeBusyHead[vzoeThrList[index].nature]];
			vzoeBusyHead[vzoeThrList[index].nature]=(vzoeBusyHead[vzoeThrList[index].nature]+1)%(vzoeCncCnt+1);
			result=pthread_mutex_unlock(&vzoeBusyMutex[vzoeThrList[index].nature]);
			if(result!=0)
			{
				mlogError("pthread_mutex_unlock",result,strerror(result),"");
				return NULL;
			}
			fmmpSwap(new,NULL);

			vzoeThrList[index].status=0X01;

			char *lnkcode;
			result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
			if(result==-1)
				goto tag29setstatus;
			char *trncode;
			result=fmmpRefGet("pCliTrnCode",0,&trncode,0);
			if(result==-1)
				goto tag29setstatus;

			char logtmppath[64];
			sprintf(logtmppath,"%s/%s/log/%s-%s-%05d.tmp",getenv("BUSINESS"),vzoeBsnCode,flogDate(0),lnkcode,vzoeThrList[index].linuxid);
			result=flogInit(logtmppath);
			if(result==-1)
				goto tag29setstatus;
			flogAnyhow("");
			flogAnyhow("#[%s]交易开始",flogTime(3));
			flogAnyhow("#linuxid[%05d]",vzoeThrList[index].linuxid);
			flogAnyhow("#posixid[%08X]",vzoeThrList[index].posixid);

			void *dlhand;
			dlhand=dlsym(vzoeTrnHandle,"ftrnInit");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag27flogfree;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]交易前处理开始",flogTime(3));
				result=((int(*)(void))dlhand)();
				flogAnyhow("#[%s]交易前处理结束",flogTime(3));
				if(result==-1)
					goto tag27flogfree;
			}

			flogAnyhow("#CliLnkCode[%s]",lnkcode);
			flogAnyhow("#CliTrnCode[%s]",trncode);

			char mark[32];
			sprintf(mark,"%s_%s",lnkcode,trncode);
			struct tzoeTrnInfo *trn;
			trn=(struct tzoeTrnInfo*)bsearch(mark,vzoeTrnList,vzoeTrnCount,sizeof(struct tzoeTrnInfo),(int(*)(const void*,const void*))strcmp);
			if(trn==NULL)
			{
				mlogError("bsearch",0,"0","[%s][%d]",mark,vzoeTrnCount);
				goto tag27flogfree;
			}

			it.it_value.tv_sec=trn->trntime;
			result=timer_settime(vzoeTime[index*2+0],0,&it,NULL);
			if(result==-1)
			{
				mlogError("settime",errno,strerror(errno),"");
				goto tag26ftrnfree;
			}

			result=fmmpValSet("pTrnDate",0,flogDate(0),0);
			if(result==-1)
				goto tag21timersettime;
			result=fmmpValSet("pTrnTime",0,flogTime(0),0);
			if(result==-1)
				goto tag21timersettime;

			flogAnyhow("");
			flogAnyhow("#[%s]交易流程开始",flogTime(3));
			char trnname[32];
			sprintf(trnname,"ftrn%s",trncode);
			dlhand=dlsym(vzoeTrnHandle,trnname);
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"[%s]",trnname);
				goto tag21timersettime;
			}
			if(dlhand!=NULL)
				error=((int(*)(void))dlhand)();

			tag20ftrnerror:;
			if(error!=0)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]调用错误处理函数开始",flogTime(3));
				flogAnyhow("#error[%d]",error);
				dlhand=dlsym(vzoeTrnHandle,"ftrnError");
				if(dlhand==NULL)
				{
					mlogError("dlsym",0,dlerror(),"");
					goto tag21timersettime;
				}
				((void(*)(int))dlhand)(error);
				flogAnyhow("#[%s]调用错误处理函数结束",flogTime(3));
			}
			flogAnyhow("#[%s]交易流程结束",flogTime(3));

			tag21timersettime:;
			it.it_value.tv_sec=0;
			result=timer_settime(vzoeTime[index*2+0],0,&it,NULL);
			if(result==-1)
			{
				mlogError("settime",errno,strerror(errno),"");
				goto tag22setretsmmp;
			}

			tag22setretsmmp:;
			char cliretcode[4+1];
			char cliretstat[1+1];
			char cliretinfo[8+1];
			result=fmmpValGet("pCliRetCode",0,cliretcode,0);
			if(result==-1)
			{
				if(error==0)
				{
					strcpy(cliretcode,"0000");
					strcpy(cliretstat,"S");
					strcpy(cliretinfo,"交易成功");
				}
				else
				if(error<0)
				{
					strcpy(cliretcode,"9999");
					strcpy(cliretstat,"F");
					strcpy(cliretinfo,"交易失败");
				}
				fmmpValSet("pCliRetCode",0,cliretcode,0);
				fmmpValSet("pCliRetStat",0,cliretstat,0);
				fmmpValSet("pCliRetInfo",0,cliretinfo,0);
			}
			else
			{
				cliretstat[0]='\0';
				cliretinfo[0]='\0';

				char errpath[64];
				sprintf(errpath,"%s/%s/ini/ret.ini",getenv("BUSINESS"),vzoeBsnCode);
				FILE *errfp;
				errfp=fopen(errpath,"r");
				if(errfp==NULL)
				{
					mlogError("fopen",errno,strerror(errno),"[%s]",errpath);
					goto tag13fpkgenc;
				}
				while(1)
				{
					char errline[128];
					fgets(errline,sizeof(errline),errfp);
					if(ferror(errfp))
					{
						mlogError("fgets",errno,strerror(errno),"");
						goto tag13fpkgenc;
					}
					if(feof(errfp))
					{
						fclose(errfp);
						break;
					}
					if(errline[0]=='#')
						continue;
					if(errline[0]=='\n')
						continue;

					if(errline[0]=='[')
					{
						if(strncmp(errline+1,"SUCCESS",7)==0)
							strcpy(cliretstat,"S");
						else
						if(strncmp(errline+1,"FAILURE",7)==0)
							strcpy(cliretstat,"F");
						continue;
					}

					char *position;
					position=strchr(errline,'=');
					if(position==NULL)
						continue;
					*position='\0';
					if(strcmp(errline,cliretcode)!=0)
						continue;

					int size;
					size=strlen(position+1)-1;
					strncpy(cliretinfo,position+1,size);
					cliretinfo[size]='\0';

					fclose(errfp);
					break;
				}

				if(cliretinfo[0]=='\0')
				{
					strcpy(cliretstat,"U");
					strcpy(cliretinfo,"未知错误");
				}

				fmmpValSet("pCliRetStat",0,cliretstat,0);
				fmmpValSet("pCliRetInfo",0,cliretinfo,0);
			}
			flogAnyhow("#CliRetCode[%s]",cliretcode);
			flogAnyhow("#CliRetStat[%s]",cliretstat);
			flogAnyhow("#CliRetInfo[%s]",cliretinfo);

			tag26ftrnfree:;
			dlhand=dlsym(vzoeTrnHandle,"ftrnFree");
			if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
			{
				mlogError("dlsym",0,dlerror(),"");
				goto tag27flogfree;
			}
			if(dlhand!=NULL)
			{
				flogAnyhow("");
				flogAnyhow("#[%s]交易后处理开始",flogTime(3));
				result=((int(*)(void))dlhand)();
				flogAnyhow("#[%s]交易后处理结束",flogTime(3));
				if(result==-1)
					goto tag27flogfree;
			}

			tag27flogfree:;
			flogAnyhow("#[%s]交易结束",flogTime(3));
			flogAnyhow("****************************************************************");
			char logfmlpath[64];
			sprintf(logfmlpath,"%s/%s/log/%s-%s-%s.log",getenv("BUSINESS"),vzoeBsnCode,flogDate(0),lnkcode,trncode);
			result=flogFree(logfmlpath,logtmppath);
			if(result==-1)
				goto tag29setstatus;

			tag29setstatus:;
			vzoeThrList[index].status=0X00;
		}
	}

	return NULL;
}

/*========================================*\
    功能 : 调用服务交易
    参数 : (输入)服务渠道代码
           (输入)服务交易代码
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fzoeSrvTrn(char *lnkcode,char *trncode)
{
	int result;

	int error;
	error=-1;

	flogAnyhow("");
	flogAnyhow("#[%s]调用服务交易开始",flogTime(3));

	flogAnyhow("#SrvLnkCode[%s]",lnkcode);
	flogAnyhow("#SrvTrnCode[%s]",trncode);

	result=fmmpValSet("pSrvLnkCode",0,lnkcode,0);
	if(result==-1)
		goto error;
	result=fmmpValSet("pSrvTrnCode",0,trncode,0);
	if(result==-1)
		goto error;

	int index;
	memcpy(&index,pthread_getspecific(vzoeIndex),sizeof(index));
	memcpy(&vzoeJump[index*2+1],&vzoeJump[index*2+0],sizeof(sigjmp_buf));
	result=sigsetjmp(vzoeJump[index*2+0],1);
	if(result!=0)
		goto error;

	struct tzoeLnkInfo *lnk;
	lnk=(struct tzoeLnkInfo*)bsearch(lnkcode,vzoeLnkList,vzoeLnkCount,sizeof(struct tzoeLnkInfo),(int(*)(const void*,const void*))strcmp);
	if(lnk==NULL)
	{
		mlogError("bsearch",0,"0","[%s][%d]",lnkcode,vzoeLnkCount);
		goto error;
	}

	char mark[32];
	sprintf(mark,"%s_%s",lnkcode,trncode);
	struct tzoeTrnInfo *trn;
	trn=(struct tzoeTrnInfo*)bsearch(mark,vzoeTrnList,vzoeTrnCount,sizeof(struct tzoeTrnInfo),(int(*)(const void*,const void*))strcmp);
	if(trn==NULL)
	{
		mlogError("bsearch",0,"0","[%s][%d]",mark,vzoeTrnCount);
		goto error;
	}

	struct itimerspec it;
	bzero(&it,sizeof(it));
	it.it_value.tv_sec=trn->trntime;
	result=timer_settime(vzoeTime[index*2+1],0,&it,NULL);
	if(result==-1)
	{
		mlogError("settime",errno,strerror(errno),"");
		goto error;
	}

	void *dlhand;

	flogAnyhow("");
	flogAnyhow("#[%s]报文编码开始",flogTime(3));
	result=fpkgEnc(lnkcode,trncode,&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index]);
	flogAnyhow("#[%s]报文编码结束",flogTime(3));
	if(result==-1)
		goto error;

	dlhand=dlsym(vzoePkgHandle,"fpkgFree");
	if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
	{
		mlogError("dlsym",0,dlerror(),"");
		goto error;
	}
	if(dlhand!=NULL)
	{
		flogAnyhow("");
		flogAnyhow("#[%s]报文编码后处理开始",flogTime(3));
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index],"server");
		flogAnyhow("#[%s]报文编码后处理结束",flogTime(3));
		if(result==-1)
			goto error;
	}

	int conid;
	conid=socket(AF_INET,SOCK_STREAM,0);
	if(conid==-1)
	{
		mlogError("socket",errno,strerror(errno),"");
		goto error;
	}

	struct sockaddr_in conaddress;
	bzero(&conaddress,sizeof(conaddress));
	conaddress.sin_family=AF_INET;
	inet_pton(AF_INET,lnk->lnkhost,&conaddress.sin_addr);
	conaddress.sin_port=htons(lnk->lnkport);

	result=connect(conid,(struct sockaddr*)&conaddress,sizeof(struct sockaddr_in));
	if(result==-1)
	{
		mlogError("connect",errno,strerror(errno),"[%s][%d]",lnk->lnkhost,lnk->lnkport);
		goto error;
	}

	int remain;
	remain=vzoeSize[index];
	int record;
	record=0;
	while(remain>0)
	{
		int result;
		result=write(conid,vzoeFmlData[index]+record,remain);
		if(result==-1&&errno!=EAGAIN)
		{
			mlogError("write",errno,strerror(errno),"");
			goto error;
		}
		remain-=result;
		record+=result;
	}

	if(lnk->lnkrule[0]==0)
	{
		vzoeSize[index]=read(conid,vzoeFmlData[index],vzoePkgSize);
		if(vzoeSize[index]==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto error;
		}
	}
	else
	if(lnk->lnkrule[0]<0)
	{
		vzoeSize[index]=-lnk->lnkrule[0];

		int remain;
		remain=vzoeSize[index];
		int record;
		record=0;
		while(remain>0)
		{
			int result;
			result=read(conid,vzoeFmlData[index]+record,remain);
			if(result==-1&&errno!=EAGAIN)
			{
				mlogError("read",errno,strerror(errno),"");
				goto error;
			}
			remain-=result;
			record+=result;
		}
	}
	else
	{
		result=read(conid,vzoeFmlData[index],lnk->lnkrule[0]);
		if(result==-1)
		{
			mlogError("read",errno,strerror(errno),"");
			goto error;
		}
		strncpy(vzoeTmpData[index],vzoeFmlData[index]+lnk->lnkrule[1],lnk->lnkrule[2]);
		vzoeTmpData[index][lnk->lnkrule[2]]='\0';
		vzoeSize[index]=lnk->lnkrule[0]+atoi(vzoeTmpData[index]);

		int remain;
		remain=vzoeSize[index]-lnk->lnkrule[0];
		int record;
		record=0;
		while(remain>0)
		{
			int result;
			result=read(conid,vzoeFmlData[index]+lnk->lnkrule[0]+record,remain);
			if(result==-1&&errno!=EAGAIN)
			{
				mlogError("read",errno,strerror(errno),"");
				goto error;
			}
			remain-=result;
			record+=result;
		}
	}

	dlhand=dlsym(vzoePkgHandle,"fpkgInit");
	if(dlhand==NULL&&strstr(dlerror(),"undefined symbol")==NULL)
	{
		mlogError("dlsym",0,dlerror(),"");
		goto error;
	}
	if(dlhand!=NULL)
	{
		flogAnyhow("");
		flogAnyhow("#[%s]报文解码前处理开始",flogTime(3));
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vzoeFmlData[index],&vzoeTmpData[index],&vzoeSize[index],"server");
		flogAnyhow("#[%s]报文解码前处理结束",flogTime(3));
		if(result==-1)
			goto error;
	}

	flogAnyhow("");
	flogAnyhow("#[%s]报文解码开始",flogTime(3));
	result=fpkgDec(lnkcode,trncode,&vzoeFmlData[index],&vzoeTmpData[index],vzoeSize[index]);
	flogAnyhow("#[%s]报文解码结束",flogTime(3));
	if(result==-1)
		goto error;

	error=0;
	error:;
	it.it_value.tv_sec=0;
	result=timer_settime(vzoeTime[index*2+1],0,&it,NULL);
	if(result==-1)
	{
		mlogError("timer_settime",errno,strerror(errno),"");
		error=-1;
	}

	if(conid>0)
		close(conid);

	memcpy(&vzoeJump[index*2+0],&vzoeJump[index*2+1],sizeof(sigjmp_buf));

	flogAnyhow("#[%s]调用服务交易结束",flogTime(3));

	return error;
}

/*========================================*\
    功能 : 调用批量交易
    参数 : (输入)批量交易代码
           (输入)批量交易函数
           (输入)批量交易参数
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fzoeBtcTrn(char *trncode,void *trnhand,void *trnpara)
{
	int result;

	int error;
	error=-1;

	int btcnum;
	btcnum=-1;
	int cncnum;
	cncnum=0;

	flogAnyhow("");
	flogAnyhow("#[%s]调用批量交易开始",flogTime(3));

	flogAnyhow("#BtcTrnCode[%s]",trncode);

	int index;
	memcpy(&index,pthread_getspecific(vzoeIndex),sizeof(index));
	memcpy(&vzoeJump[index*2+1],&vzoeJump[index*2+0],sizeof(sigjmp_buf));
	result=sigsetjmp(vzoeJump[index*2+0],1);
	if(result!=0)
		goto error;

	if(vzoeBtcCnt<=0)
		return -1;
	if(vzoeCncCnt<=0)
		return -1;

	result=pthread_mutex_lock(&vzoeSlotMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"");
		goto error;
	}
	for(btcnum=0;btcnum<vzoeBtcCnt;btcnum++)
		if(vzoeSlot[btcnum]==0X00)
			break;
	while(btcnum==vzoeBtcCnt)
	{
		result=pthread_cond_wait(&vzoeSlotCond,&vzoeSlotMutex);
		if(result!=0)
		{
			mlogError("pthread_cond_wait",result,strerror(result),"");
			pthread_mutex_unlock(&vzoeSlotMutex);
			goto error;
		}
		for(btcnum=0;btcnum<vzoeBtcCnt;btcnum++)
			if(vzoeSlot[btcnum]==0X00)
				break;
	}
	vzoeSlot[btcnum]=0X01;
	result=pthread_mutex_unlock(&vzoeSlotMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_unlock",result,strerror(result),"");
		goto error;
	}

	vzoeFreeList[btcnum]=(void**)malloc(sizeof(void*)*(vzoeCncCnt+1));
	if(vzoeFreeList[btcnum]==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(void*)*(vzoeCncCnt+1));
		goto error;
	}
	vzoeBusyList[btcnum]=(void**)malloc(sizeof(void*)*(vzoeCncCnt+1));
	if(vzoeBusyList[btcnum]==NULL)
	{
		mlogError("malloc",0,"0","[%d]",sizeof(int)*(vzoeCncCnt+1));
		goto error;
	}

	vzoeFreeHead[btcnum]=0;
	vzoeFreeTail[btcnum]=0;
	vzoeBusyHead[btcnum]=0;
	vzoeBusyTail[btcnum]=0;

	result=pthread_mutex_init(&vzoeFreeMutex[btcnum],NULL);
	if(result!=0)
	{
		mlogError("pthread_mutex_init",result,strerror(result),"");
		goto error;
	}
	result=pthread_mutex_init(&vzoeBusyMutex[btcnum],NULL);
	if(result!=0)
	{
		mlogError("pthread_mutex_init",result,strerror(result),"");
		goto error;
	}
	result=pthread_cond_init(&vzoeFreeCond[btcnum],NULL);
	if(result!=0)
	{
		mlogError("pthread_cond_init",result,strerror(result),"");
		goto error;
	}
	result=pthread_cond_init(&vzoeBusyCond[btcnum],NULL);
	if(result!=0)
	{
		mlogError("pthread_cond_init",result,strerror(result),"");
		goto error;
	}

	for(;cncnum<vzoeCncCnt;cncnum++)
	{
		result=pthread_create(&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+cncnum].posixid,NULL,fzoeEmployBoot,&vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+cncnum]);
		if(result!=0)
		{
			mlogError("pthread_create",result,strerror(result),"");
			goto error;
		}

		vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+cncnum].nature=btcnum;
		vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+cncnum].status=0X00;
		vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+cncnum].button=0X00;
	}

	char *lnkcode;
	result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
	if(result==-1)
	{
		goto error;
	}

	void *old;
	old=NULL;
	void *new;
	new=NULL;

	fmmpSwap(NULL,&old);

	while(1)
	{
		result=pthread_mutex_lock(&vzoeFreeMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_lock",result,strerror(result),"");
			goto error;
		}
		if(vzoeFreeHead[btcnum]==vzoeFreeTail[btcnum])
		{
			result=pthread_cond_wait(&vzoeFreeCond[btcnum],&vzoeFreeMutex[btcnum]);
			if(result!=0)
			{
				mlogError("pthread_cond_wait",result,strerror(result),"");
				pthread_mutex_unlock(&vzoeFreeMutex[btcnum]);
				goto error;
			}
		}
		new=vzoeFreeList[btcnum][vzoeFreeHead[btcnum]];
		vzoeFreeHead[btcnum]=(vzoeFreeHead[btcnum]+1)%(vzoeCncCnt+1);
		result=pthread_mutex_unlock(&vzoeFreeMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_unlock",result,strerror(result),"");
			goto error;
		}
		fmmpSwap(new,NULL);

		result=fmmpValSet("pBsnCode",0,vzoeBsnCode,0);
		if(result==-1)
			goto error;
		result=fmmpValSet("pCliLnkCode",0,lnkcode,0);
		if(result==-1)
			goto error;
		result=fmmpValSet("pCliTrnCode",0,trncode,0);
		if(result==-1)
			goto error;

		result=((int(*)(void*))trnhand)(trnpara);
		if(result==-1)
			goto error;
		if(result==-100)
			break;

		result=pthread_mutex_lock(&vzoeBusyMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_lock",result,strerror(result),"");
			goto error;
		}
		vzoeBusyList[btcnum][vzoeBusyTail[btcnum]]=new;
		vzoeBusyTail[btcnum]=(vzoeBusyTail[btcnum]+1)%(vzoeCncCnt+1);
		result=pthread_cond_signal(&vzoeBusyCond[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_cond_signal",result,strerror(result),"");
			pthread_mutex_unlock(&vzoeBusyMutex[btcnum]);
			goto error;
		}
		result=pthread_mutex_unlock(&vzoeBusyMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_unlock",result,strerror(result),"");
			goto error;
		}

		new=NULL;
	}

	error=0;
	error:;

	if(old!=NULL)
	{
		fmmpSwap(old,NULL);
		old=NULL;
	}

	if(new!=NULL)
	{
		result=pthread_mutex_lock(&vzoeFreeMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_lock",result,strerror(result),"");
			error=-1;
		}
		vzoeFreeList[btcnum][vzoeFreeTail[btcnum]]=new;
		vzoeFreeTail[btcnum]=(vzoeFreeTail[btcnum]+1)%(vzoeCncCnt+1);
		result=pthread_mutex_unlock(&vzoeFreeMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_unlock",result,strerror(result),"");
			error=-1;
		}
		new=NULL;
	}

	if(cncnum!=0)
	{
		result=pthread_mutex_lock(&vzoeBusyMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_lock",result,strerror(result),"");
			error=-1;
		}
		int i;
		for(i=0;i<cncnum;i++)
			vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+i].button=0X01;
		result=pthread_cond_broadcast(&vzoeBusyCond[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_cond_broadcast",result,strerror(result),"");
			error=-1;
		}
		result=pthread_mutex_unlock(&vzoeBusyMutex[btcnum]);
		if(result!=0)
		{
			mlogError("pthread_mutex_unlock",result,strerror(result),"");
			error=-1;
		}
		for(i=0;i<cncnum;i++)
		{
			result=pthread_join(vzoeThrList[vzoeThrMaxCnt+vzoeCncCnt*btcnum+i].posixid,NULL);
			if(result!=0)
			{
				mlogError("pthread_join",result,strerror(result),"");
				error=-1;
			}
		}
	}

	pthread_cond_destroy(&vzoeFreeCond[btcnum]);
	pthread_cond_destroy(&vzoeBusyCond[btcnum]);
	pthread_mutex_destroy(&vzoeFreeMutex[btcnum]);
	pthread_mutex_destroy(&vzoeBusyMutex[btcnum]);

	if(vzoeBusyList[btcnum]!=NULL)
	{
		free(vzoeBusyList[btcnum]);
		vzoeBusyList[btcnum]=NULL;
	}
	if(vzoeFreeList[btcnum]!=NULL)
	{
		free(vzoeFreeList[btcnum]);
		vzoeFreeList[btcnum]=NULL;
	}

	if(btcnum>=0&&btcnum<vzoeBtcCnt)
	{
		result=pthread_mutex_lock(&vzoeSlotMutex);
		if(result!=0)
		{
			mlogError("pthread_mutex_lock",result,strerror(result),"");
			error=-1;
		}
		vzoeSlot[btcnum]=0X00;
		result=pthread_cond_signal(&vzoeSlotCond);
		if(result!=0)
		{
			mlogError("pthread_cond_signal",result,strerror(result),"");
			pthread_mutex_unlock(&vzoeSlotMutex);
			error=-1;
		}
		result=pthread_mutex_unlock(&vzoeSlotMutex);
		if(result!=0)
		{
			mlogError("pthread_mutex_unlock",result,strerror(result),"");
			error=-1;
		}
	}

	memcpy(&vzoeJump[index*2+0],&vzoeJump[index*2+1],sizeof(sigjmp_buf));

	flogAnyhow("#[%s]调用批量交易结束",flogTime(3));

	return error;
}
