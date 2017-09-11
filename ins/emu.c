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
#include <xml.h>

/*========================================*\
    功能 : 模拟服务端
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuServer(void);
/*========================================*\
    功能 : 模拟客户端
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuClient(void);

/*========================================*\
    功能 : 读取配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuFile(void);
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
char vemuLnkHost[15+1];
//端口.
char vemuLnkPort[5+1];
//业务代码.
char vemuBsnCode[3+1];
//渠道代码.
char vemuLnkCode[3+1];
//交易代码.
char vemuTrnCode[15+1];
//报文处理函数.
char vemuPkgHandName[64];
//报文处理参数.
char vemuPkgHandArgs[64];

//发送正式报文数据.
char _vemuSndFmlData[1024*8];
char *vemuSndFmlData=_vemuSndFmlData;
//发送临时报文数据.
char _vemuSndTmpData[1024*8];
char *vemuSndTmpData=_vemuSndTmpData;
//发送报文长度.
int vemuSndSize;
//接收正式报文数据.
char _vemuRcvFmlData[1024*8];
char *vemuRcvFmlData=_vemuRcvFmlData;
//接收临时报文数据.
char _vemuRcvTmpData[1024*8];
char *vemuRcvTmpData=_vemuRcvTmpData;
//接收报文长度.
int vemuRcvSize;
//发送报文是否可读.
char vemuSndVisual;
//接收报文是否可读.
char vemuRcvVisual;

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
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuServer(void)
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

	result=femuFile();
	if(result==-1)
		return -1;

	result=femuHand();
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
	inet_pton(AF_INET,vemuLnkHost,&lisaddress.sin_addr);
	lisaddress.sin_port=htons(atoi(vemuLnkPort));

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
		mlogError("bind",errno,strerror(errno),"[%s][%s]",vemuLnkHost,vemuLnkPort);
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

		vemuRcvSize=read(conid,vemuRcvFmlData,sizeof(_vemuRcvFmlData));
		if(vemuRcvSize==-1)
		{
			mlogError("read",errno,strerror(errno),"[]");
			return -1;
		}
		vemuRcvFmlData[vemuRcvSize]='\0';

		if(vemuRcvVisual==1)
		{
			fpkgHexEnc(&vemuRcvFmlData,&vemuRcvTmpData,&vemuRcvSize,"upper");
			vemuRcvSize/=2;
		}
		printf("\033[0;31m[%s]服务端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuRcvSize,vemuRcvFmlData);

		if(vemuSndVisual==1)
		{
			fpkgHexEnc(&vemuSndFmlData,&vemuSndTmpData,&vemuSndSize,"upper");
			vemuSndSize/=2;
		}
		printf("\033[0;33m[%s]服务端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSndSize,vemuSndFmlData);

		int remain;
		remain=vemuSndSize;
		int record;
		record=0;
		while(remain>0)
		{
			int result;
			result=write(conid,vemuSndFmlData+record,remain);
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
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuClient(void)
{
	int result;

	result=femuExam();
	if(result==-1)
		return -1;

	result=femuFile();
	if(result==-1)
		return -1;

	result=femuHand();
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
	inet_pton(AF_INET,vemuLnkHost,&conaddress.sin_addr);
	conaddress.sin_port=htons(atoi(vemuLnkPort));

	result=connect(conid,(struct sockaddr*)&conaddress,sizeof(struct sockaddr_in));
	if(result==-1&&errno!=EINPROGRESS)
	{
		mlogError("connect",errno,strerror(errno),"[%s][%s]",vemuLnkHost,vemuLnkPort);
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

	if(vemuSndVisual==1)
	{
		fpkgHexEnc(&vemuSndFmlData,&vemuSndTmpData,&vemuSndSize,"upper");
		vemuSndSize/=2;
	}
	printf("\033[0;32m[%s]客户端发送报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuSndSize,vemuSndFmlData);

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
	remain=vemuSndSize;
	int record;
	record=0;
	while(remain>0)
	{
		int result;
		result=write(conid,vemuSndFmlData+record,remain);
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
	vemuRcvSize=read(conid,vemuRcvFmlData,sizeof(_vemuRcvFmlData));
	if(vemuRcvSize==-1)
	{
		mlogError("read",errno,strerror(errno),"[]");
		return -1;
	}

	if(vemuRcvVisual==1)
	{
		fpkgHexEnc(&vemuRcvFmlData,&vemuRcvTmpData,&vemuRcvSize,"upper");
		vemuRcvSize/=2;
	}
	printf("\033[0;34m[%s]客户端接收报文[%4d][%s]\033[0;37m\n",flogTime(3),vemuRcvSize,vemuRcvFmlData);

	close(conid);

	return 0;
}

/*========================================*\
    功能 : 读取配置文件
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuFile(void)
{
	int result;

	strcpy(vemuPkgHandName,"null");
	strcpy(vemuPkgHandArgs,"null");
	vemuSndVisual=0;
	vemuRcvVisual=0;

	char xmlpath[64];
	sprintf(xmlpath,"%s/%s/emu/emu.xml",getenv("BUSINESS"),vemuBsnCode);
	FILE *xmlfp;
	xmlfp=fopen(xmlpath,"r");
	if(xmlfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",xmlpath);
		return -1;
	}
	char xmldata[16384];
	int xmlsize;
	xmlsize=0;
	while(1)
	{
		fgets(xmldata+xmlsize,sizeof(xmldata)-xmlsize,xmlfp);
		if(ferror(xmlfp))
		{
			mlogError("fgets",errno,strerror(errno),"");
			return -1;
		}
		if(feof(xmlfp))
		{
			fclose(xmlfp);
			break;
		}
		xmlsize+=strlen(xmldata+xmlsize);
	}
	struct txmlItem *item;
	result=fxmlImport(&item,xmldata,xmlsize);
	if(result==-1)
		return -1;
	struct txmlItem *temp1;
	struct txmlItem *temp2;
	struct txmlItem *temp3;
	temp1=item->chld;
	if(temp1==NULL)
		return -1;
	while(1)
	{
		if(strcmp(temp1->keydata,"lnks")==0)
		{
			temp2=temp1->chld;
			if(temp2!=NULL)
			{
				while(1)
				{
					if(strcmp(temp2->keydata,vemuLnkCode)==0)
					{
						temp3=temp2->chld;
						if(temp3!=NULL)
						{
							while(1)
							{
								if(strcmp(temp3->keydata,"LnkHost")==0)
									strcpy(vemuLnkHost,temp3->valdata);
								else
								if(strcmp(temp3->keydata,"LnkPort")==0)
									strcpy(vemuLnkPort,temp3->valdata);
								temp3=temp3->next;
								if(temp3==NULL)
									break;
							}
						}
					}
					temp2=temp2->next;
					if(temp2==NULL)
						break;
				}
			}
		}
		else
		if(strcmp(temp1->keydata,"trns")==0)
		{
			temp2=temp1->chld;
			if(temp2!=NULL)
			{
				while(1)
				{
					char *position1;
					position1=temp2->keydata;
					char *position2;
					position2=strchr(position1,'.');
					if(position2==NULL)
						return -1;
					*position2='\0';
					if(strcmp(position1,vemuLnkCode)!=0)
					{
						temp2=temp2->next;
						if(temp2==NULL)
							break;
						continue;
					}
					position1=position2+1;
					if(strcmp(position1,vemuTrnCode)!=0)
					{
						temp2=temp2->next;
						if(temp2==NULL)
							break;
						continue;
					}
					temp3=temp2->chld;
					if(temp3!=NULL)
					{
						while(1)
						{
							if(strcmp(temp3->keydata,"PkgData")==0)
								vemuSndSize=sprintf(vemuSndFmlData,"%s",temp3->valdata);
							else
							if(strcmp(temp3->keydata,"PkgSndVisual")==0)
							{
								if(strcasecmp(temp3->valdata,"true")==0)
									vemuSndVisual=0;
								else
								if(strcasecmp(temp3->valdata,"false")==0)
									vemuSndVisual=1;
								else
									return -1;
							}
							else
							if(strcmp(temp3->keydata,"PkgRcvVisual")==0)
							{
								if(strcasecmp(temp3->valdata,"true")==0)
									vemuRcvVisual=0;
								else
								if(strcasecmp(temp3->valdata,"false")==0)
									vemuRcvVisual=1;
								else
									return -1;
							}
							else
							if(strcmp(temp3->keydata,"PkgHandName")==0)
								strcpy(vemuPkgHandName,temp3->valdata);
							else
							if(strcmp(temp3->keydata,"PkgHandArgs")==0)
								strcpy(vemuPkgHandArgs,temp3->valdata);
							temp3=temp3->next;
							if(temp3==NULL)
								break;
						}
					}
					temp2=temp2->next;
					if(temp2==NULL)
						break;
				}
			}
		}
		temp1=temp1->next;
		if(temp1==NULL)
			break;
	}
	fxmlFree(&item);

	if(vemuLnkHost[0]=='\0')
		return -1;
	if(vemuLnkPort[0]=='\0')
		return -1;
	if(vemuSndFmlData[0]=='\0')
		return -1;

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

	if(strcasecmp(vemuPkgHandName,"null")==0)
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
	dlhand=dlsym(handle,vemuPkgHandName);
	if(dlhand==NULL)
	{
		mlogError("dlsym",0,dlerror(),"[%s]",vemuPkgHandName);
		return -1;
	}

	if(strcasecmp(vemuPkgHandArgs,"null")==0)
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vemuSndFmlData,&vemuSndTmpData,&vemuSndSize,NULL);
	else
		result=((int(*)(char**,char**,int*,char*))dlhand)(&vemuSndFmlData,&vemuSndTmpData,&vemuSndSize,vemuPkgHandArgs);
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
	if(strlen(position1)==0)
	{
		femuHelp();
		return -1;
	}
	strcpy(vemuTrnCode,position1);

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
