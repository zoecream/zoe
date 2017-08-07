/*========================================*\
    文件 : emu.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <getopt.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <log.h>
#include <pkg.h>

/*========================================*\
    功能 : 模拟服务端
    参数 : 空`
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuServer();
/*========================================*\
    功能 : 模拟客户端
    参数 : 空`
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuClient();

/*========================================*\
    功能 : 读取渠道配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuLnkFile(void);
/*========================================*\
    功能 : 读取模拟配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuEmuFile(void);
/*========================================*\
    功能 : 调用定制接口
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuHand(void);

/*========================================*\
    功能 : 检查参数格式
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuExam(void);
/*========================================*\
    功能 : 显示帮助信息
    参数 : 空
    返回 : 空
\*========================================*/
void femuHelp(void);

//地址.
char vemuHost[15+1];
//端口.
char vemuPort[5+1];
//业务代码.
char vemuBsnCode[3+1];
//渠道代码.
char vemuLnkCode[3+1];
//交易代码.
char vemuTrnCode[15+1];
//行号.
char vemuLine;
//标记
char vemuMark;
//报文处理函数.
char vemuHandName[64];
//报文处理参数.
char vemuArgument[64];

//正式报文数据.
char _vemuFmlData[1024*8];
char *vemuFmlData=_vemuFmlData;
//临时报文数据.
char _vemuTmpData[1024*8];
char *vemuTmpData=_vemuTmpData;
//报文长度.
int vemuSize;

int femuSetBlock(int id)
{
	int flag;
	flag=fcntl(id,F_GETFL);
	if(flag==-1)
	{
		close(id);
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"fcntl");
		return -1;
	}
	flag&=~O_NONBLOCK;
	int result;
	result=fcntl(id,F_SETFL,flag);
	if(result==-1)
	{
		close(id);
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"fcntl");
		return -1;
	}
	return 0;
}

int femuSetNlock(int id)
{
	int flag;
	flag=fcntl(id,F_GETFL);
	if(flag==-1)
	{
		close(id);
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"fcntl");
		return -1;
	}
	flag|=O_NONBLOCK;
	int result;
	result=fcntl(id,F_SETFL,flag);
	if(result==-1)
	{
		close(id);
		printf("[%s][%d][%s]\n",__FILE__,__LINE__,__FUNCTION__);
		printf("[%d][%s][%s]\n",errno,strerror(errno),"fcntl");
		return -1;
	}
	return 0;
}

int main(int argc,char *argv[])
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	struct option options[]=
	{
		{"server",required_argument,NULL,'s'},
		{"client",required_argument,NULL,'c'},
		{"help",no_argument,NULL,'h'},
		{0,0,0,0}
	};

	int isserver=0;
	int isclient=0;
	int count=1;
	char *record;

	int option;
	option=getopt_long(argc,argv,":s:c:h",options,NULL);

	switch(option)
	{
		case 's':
		result=femuServer();
		if(result==-1)
			return -1;
		break;

		case 'c':
		result=femuClient();
		if(result==-1)
			return -1;
		break;

		case 'h':
		case ':':
		case '?':
		case -1:
		femuHelp();
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 模拟服务端
    参数 : 空`
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuServer()
{
	int result;

	result=fork();
	if(result==-1)
	{
		mlogError("fork",errno,strerror(errno),"[]");
		return -1;
	}
	if(result>0)
		return 0;

	result=femuExam();
	if(result==-1)
		return -1;

	result=femuLnkFile();
	if(result==-1)
		return -1;

	int lisid;
	lisid=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(lisid==-1)
	{
		mlogError("socket",errno,strerror(errno),"[]");
		return -1;
	}

	struct sockaddr_in lisaddress;
	bzero(&lisaddress,sizeof(lisaddress));
	lisaddress.sin_family=AF_INET;
	inet_pton(AF_INET,vemuHost,&lisaddress.sin_addr);
	lisaddress.sin_port=htons(atoi(vemuPort));

	int option;
	option=1;
	result=setsockopt(lisid,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option));
	if(result==-1)
	{
		close(lisid);
		mlogError("setsockopt",errno,strerror(errno),"[]");
		return -1;
	}
	result=bind(lisid,(struct sockaddr*)&lisaddress,sizeof(struct sockaddr_in));
	if(result==-1)
	{
		close(lisid);
		mlogError("bind",errno,strerror(errno),"[%s][%s]",vemuHost,vemuPort);
		return -1;
	}
	result=listen(lisid,1024);
	if(result==-1)
	{
		close(lisid);
		mlogError("listen",errno,strerror(errno),"[]");
		return -1;
	}

	while(1)
	{
		struct sockaddr_in conaddress;
		bzero(&conaddress,sizeof(conaddress));
		int size;
		size=sizeof(conaddress);
		int conid;
		conid=accept(lisid,(struct sockaddr*)&conaddress,&size);
		if(conid==-1)
		{
			mlogError("accept",errno,strerror(errno),"[]");
			return -1;
		}
	
		/*
		struct timeval timeout={150,0};
		setsockopt(conid,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
		setsockopt(conid,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
		*/

		vemuSize=read(conid,vemuFmlData,sizeof(_vemuFmlData));
		if(vemuSize==-1)
		{
			mlogError("read",errno,strerror(errno),"[]");
			return -1;
		}
		vemuFmlData[vemuSize]='\0';

		if(vemuMark=='-')
		{
			printf("\033[0;31m[%s]服务端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize,vemuFmlData);
		}
		else
		if(vemuMark=='+')
		{
			fpkgHexEnc(&vemuFmlData,&vemuTmpData,&vemuSize,"upper");
			printf("\033[0;31m[%s]服务端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize/=2,vemuFmlData);
			mpkgSwap(vemuFmlData,vemuTmpData);
		}

		result=femuEmuFile();
		if(result==-1)
			return -1;

		result=femuHand();
		if(result==-1)
			return -1;

		if(vemuMark=='-')
		{
			printf("\033[0;33m[%s]服务端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize,vemuFmlData);
		}
		else
		if(vemuMark=='+')
		{
			fpkgHexEnc(&vemuFmlData,&vemuTmpData,&vemuSize,"upper");
			printf("\033[0;33m[%s]服务端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize/=2,vemuFmlData);
			mpkgSwap(vemuFmlData,vemuTmpData);
		}

		int remain;
		remain=vemuSize;
		int record;
		record=0;
		while(remain>0)
		{
			int result;
			result=write(conid,vemuFmlData+record,remain);
			if(result==-1&&errno!=EAGAIN)
			{
				mlogError("write",errno,strerror(errno),"[]");
				return -1;
			}
			remain-=result;
			record+=result;
		}

		close(conid);
	}

	close(lisid);

	return 0;
}

/*========================================*\
    功能 : 模拟客户端
    参数 : 空`
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuClient()
{
	int result;

	result=femuExam();
	if(result==-1)
		return -1;

	result=femuLnkFile();
	if(result==-1)
		return -1;

	int conid;
	conid=socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(conid==-1)
	{
		mlogError("socket",errno,strerror(errno),"[]");
		return -1;
	}

	fd_set set;
	struct timeval timeout={8,0};

	result=femuSetNlock(conid);
	if(result==-1)
		return -1;

	struct sockaddr_in conaddress;
	bzero(&conaddress,sizeof(conaddress));
	conaddress.sin_family=AF_INET;
	inet_pton(AF_INET,vemuHost,&conaddress.sin_addr);
	conaddress.sin_port=htons(atoi(vemuPort));

	result=connect(conid,(struct sockaddr*)&conaddress,sizeof(struct sockaddr_in));
	if(result==-1&&errno!=EINPROGRESS)
	{
		mlogError("connect",errno,strerror(errno),"[%s][%s]",vemuHost,vemuPort);
		return -1;
	}
	if(result==-1&&errno==EINPROGRESS)
	{
		FD_ZERO(&set);
		FD_SET(conid,&set);

		do
			result=select(conid+1,NULL,&set,NULL,&timeout);
		while(result==-1&&errno==EINTR);
		if(result==-1)
		{
			mlogError("select",errno,strerror(errno),"[]");
			return -1;
		}
		if(result==0)
			return -1;
		if(result>0)
		{
			int error;
			int length;
			length=sizeof(error);
			result=getsockopt(conid,SOL_SOCKET,SO_ERROR,&error,(socklen_t*)&length);
			if(result==-1)
			{
				mlogError("getsockopt",errno,strerror(errno),"[]");
				return -1;
			}
			if(error!=0)
			{
				mlogError("getsockopt",error,strerror(error),"[]");
				return -1;
			}
		}
	}

	result=femuSetBlock(conid);
	if(result==-1)
		return -1;

	/*
	struct timeval timeout={10,0};
	setsockopt(conid,SOL_SOCKET,SO_SNDTIMEO,&timeout,sizeof(timeout));
	setsockopt(conid,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
	*/

	result=femuEmuFile();
	if(result==-1)
		return -1;

	result=femuHand();
	if(result==-1)
		return -1;

	if(vemuMark=='-')
	{
		printf("\033[0;32m[%s]客户端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize,vemuFmlData);
	}
	else
	if(vemuMark=='+')
	{
		fpkgHexEnc(&vemuFmlData,&vemuTmpData,&vemuSize,"upper");
		printf("\033[0;32m[%s]客户端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize=vemuSize/2,vemuFmlData);
		mpkgSwap(vemuFmlData,vemuTmpData);
	}

	FD_ZERO(&set);
	FD_SET(conid,&set);
	do
		result=select(conid+1,NULL,&set,NULL,&timeout);
	while(result==-1&&errno==EINTR);
	if(result==-1)
	{
		mlogError("select",errno,strerror(errno),"[]");
		return -1;
	}
	if(result==0)
		return -1;
	int remain;
	remain=vemuSize;
	int record;
	record=0;
	while(remain>0)
	{
		int result;
		result=write(conid,vemuFmlData+record,remain);
		if(result==-1&&errno!=EAGAIN)
		{
			mlogError("write",errno,strerror(errno),"[]");
			return -1;
		}
		remain-=result;
		record+=result;
	}

	FD_ZERO(&set);
	FD_SET(conid,&set);
	do
		result=select(conid+1,&set,NULL,NULL,&timeout);
	while(result==-1&&errno==EINTR);
	if(result==-1)
	{
		mlogError("select",errno,strerror(errno),"[]");
		return -1;
	}
	if(result==0)
		return -1;
	vemuSize=read(conid,vemuFmlData,sizeof(_vemuFmlData));
	if(vemuSize==-1)
	{
		mlogError("read",errno,strerror(errno),"[]");
		return -1;
	}

	if(vemuMark=='-')
	{
		printf("\033[0;34m[%s]客户端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize,vemuFmlData);
	}
	else
	if(vemuMark=='+')
	{
		fpkgHexEnc(&vemuFmlData,&vemuTmpData,&vemuSize,"upper");
		printf("\033[0;34m[%s]客户端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSize/=2,vemuFmlData);
		mpkgSwap(vemuFmlData,vemuTmpData);
	}

	close(conid);

	return 0;
}

/*========================================*\
    功能 : 读取渠道配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuLnkFile(void)
{
	int result;

	char lnkpath[64];
	sprintf(lnkpath,"%s/%s/ini/lnk.ini",getenv("BUSINESS"),vemuBsnCode);
	FILE *lnkfp;
	lnkfp=fopen(lnkpath,"r");
	if(lnkfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",lnkpath);
		return -1;
	}

	while(1)
	{
		char lnkline[128];
		fgets(lnkline,sizeof(lnkline),lnkfp);
		if(ferror(lnkfp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			return -1;
		}
		if(feof(lnkfp))
		{
			fclose(lnkfp);
			return -1;
		}
		if(lnkline[0]=='#')
			continue;
		if(lnkline[0]=='\n')
			continue;

		if(lnkline[0]=='[')
		{
			if(strncmp(lnkline+1,vemuLnkCode,3)!=0)
				continue;

			while(1)
			{
				fgets(lnkline,sizeof(lnkline),lnkfp);
				if(ferror(lnkfp))
				{
					mlogError("fgets",errno,strerror(errno),"[]");
					return -1;
				}
				if(feof(lnkfp))
				{
					fclose(lnkfp);
					return -1;
				}
				if(lnkline[0]=='#')
					continue;
				if(lnkline[0]=='\n')
					continue;

				if(lnkline[0]=='[')
				{
					fclose(lnkfp);
					return -1;
				}

				if(strncmp(lnkline,"LnkHost",7)==0)
				{
					lnkline[strlen(lnkline)-1]='\0';
					strcpy(vemuHost,strchr(lnkline,'=')+1);
				}
				if(strncmp(lnkline,"LnkPort",7)==0)
				{
					lnkline[strlen(lnkline)-1]='\0';
					strcpy(vemuPort,strchr(lnkline,'=')+1);
				}

				if(vemuHost[0]!='\0'&&vemuPort[0]!='\0')
				{
					fclose(lnkfp);
					break;
				}
			}
			break;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 读取模拟配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuEmuFile(void)
{
	int result;

	char emupath[64];
	sprintf(emupath,"%s/%s/emu/emu.ini",getenv("BUSINESS"),vemuBsnCode);
	FILE *emufp;
	emufp=fopen(emupath,"r");
	if(emufp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",emupath);
		return -1;
	}

	while(1)
	{
		char emuline[1024*8];
		fgets(emuline,sizeof(emuline),emufp);
		if(ferror(emufp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			return -1;
		}
		if(feof(emufp))
		{
			fclose(emufp);
			return -1;
		}
		if(emuline[0]=='#')
			continue;
		if(emuline[0]=='\n')
			continue;

		if(emuline[0]=='[')
		{
			char *position1=emuline+1;
			char *position2=position1;

			if(*position2!='-'&&*position2!='+')
				return -1;
			vemuMark=*position2;

			position1=position2+1;
			position2=strchr(position1,'_');
			if(position2==NULL)
				return -1;
			*position2='\0';
			if(strcmp(position1,vemuLnkCode)!=0)
				continue;

			position1=position2+1;
			position2=strchr(position1,'_');
			if(position2==NULL)
				return -1;
			*position2='\0';
			if(strcmp(position1,vemuTrnCode)!=0)
				continue;

			position1=position2+1;
			position2=strchr(position1,'_');
			if(position2==NULL)
				return -1;
			*position2='\0';
			strcpy(vemuHandName,position1);

			position1=position2+1;
			position2=strchr(position1,']');
			if(position2==NULL)
				return -1;
			*position2='\0';
			strcpy(vemuArgument,position1);

			int line=0;

			while(1)
			{
				fgets(emuline,sizeof(emuline),emufp);
				if(ferror(emufp))
				{
					mlogError("fgets",errno,strerror(errno),"[]");
					return -1;
				}
				if(feof(emufp))
				{
					fclose(emufp);
					return -1;
				}
				if(emuline[0]=='#')
					continue;
				if(emuline[0]=='\n')
					continue;

				if(emuline[0]=='[')
				{
					fclose(emufp);
					return -1;
				}

				if(line++!=vemuLine)
					continue;

				emuline[strlen(emuline)-1]='\0';
				vemuSize=strlen(emuline);
				strcpy(vemuFmlData,emuline);

				fclose(emufp);
				break;
			}
			break;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 调用定制接口
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuHand(void)
{
	int result;

	if(strcasecmp(vemuHandName,"null")==0)
		return 0;

	char path[64];
	sprintf(path,"%s/%s/emu/libemu.so",getenv("BUSINESS"),vemuBsnCode,vemuLnkCode);
	void *handle;
	handle=dlopen(path,RTLD_NOW|RTLD_GLOBAL);
	if(handle==NULL)
	{
		mlogError("dlopen",0,dlerror(),"[%s]",path);
		return -1;
	}

	void *dlhand;
	dlhand=dlsym(handle,vemuHandName);
	if(dlhand==NULL)
	{
		mlogError("dlsym",0,dlerror(),"[%s]",vemuHandName);
		return -1;
	}

	if(strcasecmp(vemuArgument,"null")==0)
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vemuFmlData,&vemuTmpData,&vemuSize,NULL);
	else
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vemuFmlData,&vemuTmpData,&vemuSize,vemuArgument);
	if(result<0)
		return -1;

	dlclose(handle);

	return 0;
}

/*========================================*\
    功能 : 检查参数格式
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuExam(void)
{
	char *position1;
	char *position2;

	position1=optarg;
	position2=strchr(position1,'_');
	if(position2==NULL)
	{
		femuHelp();
		return -1;
	}
	if(position2-position1!=3)
	{
		femuHelp();
		return -1;
	}
	strncpy(vemuBsnCode,position1,3);

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
	{
		femuHelp();
		return -1;
	}
	if(position2-position1!=3)
	{
		femuHelp();
		return -1;
	}
	strncpy(vemuLnkCode,position1,3);

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
	{
		femuHelp();
		return -1;
	}
	strncpy(vemuTrnCode,position1,position2-position1);

	position1=position2+1;
	if(strlen(position1)==0)
	{
		femuHelp();
		return -1;
	}
	vemuLine=atoi(position1);

	return 0;
}

/*========================================*\
    功能 : 显示帮助信息
    参数 : 空
    返回 : 空
\*========================================*/
void femuHelp(void)
{
	printf("\n");
	printf("-s|--server : 模拟服务端,参数:业务_渠道_交易_行号.\n");
	printf("-c|--client : 模拟客户端,参数:业务_渠道_交易_行号.\n");
	printf("-p          : 启动指定数量的进程并发处理.\n");
	printf("\n");
	return;
}
