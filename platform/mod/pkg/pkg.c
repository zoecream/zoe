/*========================================*\
    文件 : pkg.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <iconv.h>
#include <dlfcn.h>
#include <pthread.h>

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#include <log.h>
#include <mmp.h>
#include <pkg.h>
#include <jsn.h>

//报文规则尺寸.
#define cpkgRuleSize 65536
//报文规则数量.
#define cpkgRuleCount 64

//保障OPENSSL线程安全.
pthread_mutex_t vpkgMutex;

//报文规则指针.
char *vpkgRuleHead;
//报文规则数量.
char vpkgRuleCount;

struct tpkgRule
{
	//规则标识.
	char rulename[32];
	//函数位置.
	char *handp;
	//函数数量.
	char handcount;
};
struct tpkgHand
{
	//函数名称.
	char handname[16];
	//函数参数.
	char handargs[32];
	//函数指针.
	void *handp;
	//节点位置.
	char *nodep;
	//节点数量.
	char nodecount;
	//节点序号.
	char nodeindex;
	//中间数据.
	char middle;
	//视觉数据.
	char visual;
};
struct tpkgNode
{
	//节点名称.
	char nodename[16];
	//函数名称.
	char handname[16];
	//函数参数.
	char handargs[32];
	//函数指针.
	void *handp;
	//格式数据.
	char format[64];
};

struct tpkgStack
{
	//栈所有帧.
	struct tpkgHand full[32];
	//栈结尾帧.
	struct tpkgHand *last;
};

/*========================================*\
    功能 : 报文规则处理函数入栈
    参数 : (输入)栈结构指针
           (输入)帧结构指针
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgStackPush(struct tpkgStack *stack,struct tpkgHand *hand)
{
	if(stack->last==stack->full+sizeof(stack->full)/sizeof(struct tpkgHand)-1)
		return -1;
	if(stack->last==NULL)
		stack->last=stack->full;
	else
		stack->last++;
	memcpy(stack->last,hand,sizeof(struct tpkgHand));
	return 0;
}

/*========================================*\
    功能 : 报文规则处理函数出栈
    参数 : (输入)栈结构指针
           (输入)帧结构指针
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgStackPull(struct tpkgStack *stack,struct tpkgHand *hand)
{
	if(stack->last==NULL)
		return -1;
	memcpy(hand,stack->last,sizeof(struct tpkgHand));
	if(stack->last==stack->full)
		stack->last=NULL;
	else
		stack->last--;
	return 0;
}

/*========================================*\
    功能 : 变长格式报文打包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
           (输入)内存池序号
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgVarEnc(struct tpkgNode *node,char **middle,int count,int index,char **data,int *size)
{
	int result;

	int i;
	for(i=0;i<count;i++)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
		{
			result=fmmpValGet(node->nodename,index,mfmldata,&msize);
			if(result==-1)
				return -1;

			if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
				msize=sprintf(mfmldata,"%d",*(int*)mfmldata);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
				msize=sprintf(mfmldata,"%ld",*(long*)mfmldata);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
				msize=sprintf(mfmldata,"%.2f",*(double*)mfmldata);

			if(node->handp==NULL)
			{
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				memcpy(*data,mfmldata,msize);
			}
			else
			{
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
				if(result==-1)
					return -1;
				flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
				memcpy(*data,mfmldata,msize);
			}
		}
		else
		if(node->nodename[0]<=0X1F)
		{
			if(middle[node->nodename[0]]==NULL)
				return -100;
			msize=*(short*)middle[node->nodename[0]];
			memcpy(*data,middle[node->nodename[0]]+sizeof(short),msize);
			free(middle[node->nodename[0]]);
		}
		else
		if(node->nodename[0]==0X7E)
		{
			msize=*(short*)(node->nodename+1);
			memcpy(*data,node->nodename+1+sizeof(short),msize);
		}

		memcpy(*data+msize,&node->format[1],node->format[0]);

		*data+=msize+node->format[0];
		*size+=msize+node->format[0];

		node++;
	}

	return 0;
}

/*========================================*\
    功能 : 变长格式报文解包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
           (输入)内存池序号
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgVarDec(struct tpkgNode *node,char **middle,int count,int index,char **data,int size)
{
	int result;

	int i;
	for(i=0;i<count;i++)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		msize=0;
		while(1)
		{
			if(msize>=size-node->format[0]+1)
				return -1;
			if(memcmp(*data+msize,&node->format[1],node->format[0])==0)
				break;
			msize++;
		}
		int mrecord;
		mrecord=msize;

		if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
		{
			if(node->handp==NULL)
			{
				memcpy(mfmldata,*data,msize);
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
			}
			else
			{
				memcpy(mfmldata,*data,msize);
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
				if(result==-1)
					return -1;
				flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
			}
			mfmldata[msize]='\0';

			if(node->nodename[node->nodename[0]=='_'?1:0]=='p')
				result=fmmpValSet(node->nodename,index,mfmldata,msize);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
			{
				*(int*)mfmldata=atoi(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
			{
				*(long*)mfmldata=atol(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
			{
				*(double*)mfmldata=atof(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			if(result==-1)
				return -1;
		}
		else
		if(node->nodename[0]<=0X1F)
		{
			middle[node->nodename[0]]=(char*)malloc(sizeof(short)+msize);
			if(middle[node->nodename[0]]==NULL)
				return -1;
			*(short*)middle[node->nodename[0]]=msize;
			memcpy(middle[node->nodename[0]]+sizeof(short),*data,msize);
		}
		else
		if(node->nodename[0]==0X7E)
		{
		}

		*data+=mrecord+node->format[0];
		size-=mrecord+node->format[0];

		node++;
	}

	return 0;
}

/*========================================*\
    功能 : 定长格式报文打包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
           (输入)内存池序号
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgFixEnc(struct tpkgNode *node,char **middle,int count,int index,char **data,int *size)
{
	int result;

	int i;
	for(i=0;i<count;i++)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
		{
			result=fmmpValGet(node->nodename,index,mfmldata,&msize);
			if(result==-1)
				return -1;

			if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
				msize=sprintf(mfmldata,"%d",*(int*)mfmldata);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
				msize=sprintf(mfmldata,"%ld",*(long*)mfmldata);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
				msize=sprintf(mfmldata,"%.2f",*(double*)mfmldata);

			if(node->handp==NULL)
			{
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				memcpy(*data,mfmldata,msize);
			}
			else
			{
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
				if(result==-1)
					return -1;
				flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
				memcpy(*data,mfmldata,msize);
			}
		}
		else
		if(node->nodename[0]<=0X1F)
		{
			if(middle[node->nodename[0]]==NULL)
				return -100;
			msize=*(short*)middle[node->nodename[0]];
			memcpy(*data,middle[node->nodename[0]]+sizeof(short),msize);
			free(middle[node->nodename[0]]);
		}
		else
		if(node->nodename[0]==0X7E)
		{
			msize=*(short*)(node->nodename+1);
			memcpy(*data,node->nodename+1+sizeof(short),msize);
		}

		if(*(short*)node->format>msize)
			memset(*data+msize,node->format[sizeof(short)],*(short*)node->format-msize);

		*data+=*(short*)node->format;
		*size+=*(short*)node->format;

		node++;
	}

	return 0;
}

/*========================================*\
    功能 : 定长格式报文解包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
		   (输入)内存池序号
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgFixDec(struct tpkgNode *node,char **middle,int count,int index,char **data,int size)
{
	int result;

	int i;
	for(i=0;i<count;i++)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		if(size<*(short*)node->format)
			return -1;
		char *temp;
		temp=memchr(*data,node->format[sizeof(short)],*(short*)node->format);
		if(temp==NULL)
			msize=*(short*)node->format;
		else
			msize=temp-*data;

		if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
		{
			if(node->handp==NULL)
			{
				memcpy(mfmldata,*data,msize);
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
			}
			else
			{
				memcpy(mfmldata,*data,msize);
				flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
				result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
				if(result==-1)
					return -1;
				flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
			}
			mfmldata[msize]='\0';

			if(node->nodename[node->nodename[0]=='_'?1:0]=='p')
				result=fmmpValSet(node->nodename,index,mfmldata,msize);
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
			{
				*(int*)mfmldata=atoi(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
			{
				*(long*)mfmldata=atol(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			else
			if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
			{
				*(double*)mfmldata=atof(mfmldata);
				result=fmmpValSet(node->nodename,index,mfmldata,0);
			}
			if(result==-1)
				return -1;
		}
		else
		if(node->nodename[0]<=0X1F)
		{
			middle[node->nodename[0]]=(char*)malloc(sizeof(short)+msize);
			if(middle[node->nodename[0]]==NULL)
				return -1;
			*(short*)middle[node->nodename[0]]=msize;
			memcpy(middle[node->nodename[0]]+sizeof(short),*data,msize);
		}
		else
		if(node->nodename[0]==0X7E)
		{
		}

		*data+=*(short*)node->format;
		size-=*(short*)node->format;

		node++;
	}

	return 0;
}

/*========================================*\
    功能 : JSON格式报文打包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
           (输入)内存池序号
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgJsnEnc(struct tpkgNode *node,char **middle,int count,int index,char **data,int *size)
{
	/*
	int result;

	struct tjsnItem *head;
	result=fjsnObjCreate(&head);
	if(result==-1)
		return -1;

	int i;
	for(i=0;i<count;i++)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		struct tjsnItem *item1;
		struct tjsnItem *item2;
		item1=head;

		char *position1;
		char *position2;
		position1=node->format;

		while(1)
		{
			position2=strchr(position1,'/');

			if(*(position2+1)=='\0')
			{
				if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
				{
					result=fmmpValGet(node->nodename,index,mfmldata,&msize);
					if(result==-1)
						return -1;

					if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
						msize=sprintf(mfmldata,"%d",*(int*)mfmldata);
					else
					if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
						msize=sprintf(mfmldata,"%ld",*(long*)mfmldata);
					else
					if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
						msize=sprintf(mfmldata,"%.2f",*(double*)mfmldata);

					if(node->handp==NULL)
					{
						flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
						if(node->nodename[node->nodename[0]=='_'?1:0]=='p')
							result=fjsnStrCreate(&item2,mfmldata,msize);
						else
							result=fjsnNumCreate(&item2,mfmldata,msize);
						if(result==-1)
							return -1;
					}
					else
					{
						flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
						result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
						if(result==-1)
							return -1;
						flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
						if(node->nodename[node->nodename[0]=='_'?1:0]=='p')
							result=fjsnStrCreate(&item2,mfmldata,msize);
						else
							result=fjsnNumCreate(&item2,mfmldata,msize);
						if(result==-1)
							return -1;
					}
				}
				else
				if(node->nodename[0]<=0X1F)
				{
					if(middle[node->nodename[0]]==NULL)
						return -100;
					msize=*(short*)middle[node->nodename[0]];
					result=fjsnStrCreate(&item2,middle[node->nodename[0]]+sizeof(short),msize);
					if(result==-1)
						return -1;
					free(middle[node->nodename[0]]);
				}
				else
				if(node->nodename[0]==0X7E)
				{
					result=fjsnStrCreate(&item2,node->nodename+1+sizeof(short),*(short*)(node->nodename+1));
					if(result==-1)
						return -1;
				}

				fjsnObjInsert(item1,item2,position1,position2-position1);

				break;
			}
			else
			{
				result=fjsnObjSelect(item1,&item2,position1,position2-position1);
				if(result==-1)
				{
					result=fjsnObjCreate(&item2);
					if(result==-1)
						return -1;
					fjsnObjInsert(item1,item2,position1,position2-position1);
				}

				item1=item2;
			}

			position1=position2+1;
		}

		node++;
	}

	result=fjsnExport(head,data,size);
	if(result==-1)
		return -1;
	fjsnFree(head);

	return 0;
	*/
}

/*========================================*\
    功能 : JSON格式报文解包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
		   (输入)内存池序号
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgJsnDec(struct tpkgNode *node,char **middle,int count,int index,char **data,int size)
{
	/*
	int result;

	struct tjsnItem *head;
	result=fjsnObjCreate(&head);
	if(result==-1)
		return -1;
	result=fjsnImport(head,data,size);
	if(result==-1)
		return -1;

	int i;
	for(i=0;i<count;i++)
	while(1)
	{
		char _mfmldata[1024];
		char *mfmldata=_mfmldata;
		char _mtmpdata[1024];
		char *mtmpdata=_mtmpdata;
		int msize;

		struct tjsnItem *item1;
		struct tjsnItem *item2;
		item1=head;

		char *position1;
		char *position2;
		position1=node->format;

		while(1)
		{
			position2=strchr(position1,'/');

			result=fjsnObjSelect(item1,&item2,position1,position2-position1);
			if(result==-1)
				return -1;

			if(*(position2+1)=='\0')
			{
				msize=item2->vall;

				if(node->nodename[0]>0X1F&&node->nodename[0]<0X7E)
				{
					if(node->handp==NULL)
					{
						memcpy(mfmldata,item2->vald,msize);
						flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
					}
					else
					{
						memcpy(mfmldata,item2->vald,msize);
						flogDepend("#[%-15s][%4d][%.*s]",node->nodename,msize,msize,mfmldata);
						result=((int(*)(char**,char**,int*,char*))node->handp)(&mfmldata,&mtmpdata,&msize,node->handargs);
						if(result==-1)
							return -1;
						flogDepend("#[%-15s][%4d][%.*s]",node->handname,msize,msize,mfmldata);
					}

					if(node->nodename[node->nodename[0]=='_'?1:0]=='p')
					{
						mfmldata[msize]='\0';
						result=fmmpValSet(node->nodename,index,mfmldata,msize);
					}
					else
					if(node->nodename[node->nodename[0]=='_'?1:0]=='i')
					{
						*(int*)mfmldata=atoi(mfmldata);
						result=fmmpValSet(node->nodename,index,mfmldata,0);
					}
					else
					if(node->nodename[node->nodename[0]=='_'?1:0]=='l')
					{
						*(long*)mfmldata=atol(mfmldata);
						result=fmmpValSet(node->nodename,index,mfmldata,0);
					}
					else
					if(node->nodename[node->nodename[0]=='_'?1:0]=='d')
					{
						*(double*)mfmldata=atof(mfmldata);
						result=fmmpValSet(node->nodename,index,mfmldata,0);
					}
					if(result==-1)
						return -1;
				}
				else
				if(node->nodename[0]<=0X1F)
				{
					middle[node->nodename[0]]=(char*)malloc(sizeof(short)+msize);
					if(middle[node->nodename[0]]==NULL)
						return -1;
					*(short*)middle[node->nodename[0]]=msize;
					memcpy(middle[node->nodename[0]]+sizeof(short),item2->vald,msize);
				}
				else
				if(node->nodename[0]==0X7E)
				{
				}

				break;
			}

			item1=item2;

			position1=position2+1;
		}

		node++;
	}

	fjsnFree(head);

	return 0;
	*/
}

/*========================================*\
    功能 : XML格式报文打包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
           (输入)内存池序号
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgXmlEnc(struct tpkgNode *node,char **middle,int count,int index,char **data,int *size)
{
}

/*========================================*\
    功能 : XML格式报文解包
    参数 : (输入)报文节点
           (输入)中间数据
           (输入)节点数量
		   (输入)内存池序号
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgXmlDec(struct tpkgNode *node,char **middle,int count,int index,char **data,int size)
{
}

/*========================================*\
    功能 : 创建报文规则
    参数 : (输入)业务代码
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgRuleInit(char *bsncode)
{
	int result;

	result=pthread_mutex_init(&vpkgMutex,NULL);
	if(result!=0)
	{
		mlogError("pthread_mutex_init",result,strerror(result),"[]");
		return -1;
	}

	char bsnpkgpath[64];
	sprintf(bsnpkgpath,"%s/%s/pkg/libpkg.so",getenv("BUSINESS"),bsncode);
	void *bsnhandle;
	bsnhandle=dlopen(bsnpkgpath,RTLD_NOW|RTLD_GLOBAL);
	if(bsnhandle==NULL)
	{
		mlogError("dlopen",0,dlerror(),"[%d]",bsnpkgpath);
		return -1;
	}

	vpkgRuleHead=(char*)malloc(cpkgRuleSize);
	if(vpkgRuleHead==NULL)
	{
		mlogError("malloc",0,"0","[%d]",cpkgRuleSize);
		return -1;
	}

	struct tpkgRule *rulep;
	rulep=(struct tpkgRule*)vpkgRuleHead;
	struct tpkgHand *handp;
	handp=(struct tpkgHand*)(vpkgRuleHead+cpkgRuleSize-sizeof(struct tpkgHand));
	bzero(handp--,sizeof(struct tpkgHand));
	struct tpkgNode *nodep;
	nodep=(struct tpkgNode*)(vpkgRuleHead+sizeof(struct tpkgRule)*cpkgRuleCount);

	char pkgpath[64];
	sprintf(pkgpath,"%s/%s/ini/pkg.ini",getenv("BUSINESS"),bsncode);
	FILE *pkgfp;
	pkgfp=fopen(pkgpath,"r");
	if(pkgfp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",pkgpath);
		return -1;
	}

	char text[2048];
	int i=0;
	int count=0;
	int markpos;
	int notepos=-1;

	while(1)
	{
		text[i]=fgetc(pkgfp);
		if(ferror(pkgfp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			return -1;
		}
		if(feof(pkgfp))
			break;

		switch(text[i])
		{
			case '*':
			if(i!=0&&text[i-1]=='/')
			{
				notepos=i-1;
			}
			break;
			case '/':
			if(i!=0&&text[i-1]=='*')
			{
				i=notepos-1;
				notepos=-1;
			}
			break;

			case '[':
			markpos=i;
			break;
			case ']':
			strncpy(rulep->rulename,text+markpos+1,i-markpos-1);
			rulep->rulename[i-markpos-1]='\0';
			i=0;
			continue;

			case '(':
			count++;
			if(i!=0&&text[i-1]!=' '&&text[i-1]!='('&&text[i-1]!=')')
			{
				text[i]=' ';
				i++;
				text[i]='(';
			}
			break;
			case ')':
			count--;
			if(i!=0&&text[i-1]!=' '&&text[i-1]!='('&&text[i-1]!=')')
			{
				text[i]=' ';
				i++;
				text[i]=')';
			}
			break;

			case ' ':
			case '\t':
			case '\v':
			case '\n':
			case '\r':
			case '\f':
			if(i==0||text[i-1]==' '||text[i-1]=='('||text[i-1]==')')
				continue;
			else
				text[i]=' ';
		}

		if(text[i++]!=')'||count!=0||notepos!=-1)
			continue;

		text[i]='\0';
		i=0;

		char *position1;
		position1=text;
		char *position2;

		struct tpkgHand hand;
		bzero(&hand,sizeof(hand));
		struct tpkgStack stack;
		bzero(&stack,sizeof(stack));

		struct tpkgHand *record;
		record=handp;

		struct tpkgNode *nodes[4];
		char alloc;
		alloc=0;
		char lasts[4];
		bzero(lasts,sizeof(lasts));
		char stats[4];
		bzero(stats,sizeof(stats));

		char index;
		index=0X00;

		while(1)
		{
			if(*position1=='\0')
			{
				if(strstr(rulep->rulename,"ENCODE")!=NULL)
					rulep->handp=(char*)record;
				else
				if(strstr(rulep->rulename,"DECODE")!=NULL)
					rulep->handp=(char*)(handp+1);
				rulep->handcount=record-handp;
				break;
			}
			else
			if(*position1=='(')
			{
				position1++;

				if(strncmp(position1,"array",5)!=0)
				{
					hand.visual=*position1;
					position1++;
				}

				position2=strchr(position1,'_');
				strncpy(hand.handname,position1,position2-position1);
				hand.handname[position2-position1]='\0';

				position1=position2+1;
				position2=strchr(position1,' ');
				strncpy(hand.handargs,position1,position2-position1);
				hand.handargs[position2-position1]='\0';

				if(strcmp(hand.handname,"array")!=0)
				{
					if(strcmp(hand.handname,"fpkgVarEnc")==0)
						hand.handp=fpkgVarEnc;
					else
					if(strcmp(hand.handname,"fpkgVarDec")==0)
						hand.handp=fpkgVarDec;
					else
					if(strcmp(hand.handname,"fpkgFixEnc")==0)
						hand.handp=fpkgFixEnc;
					else
					if(strcmp(hand.handname,"fpkgFixDec")==0)
						hand.handp=fpkgFixDec;
					else
					if(strcmp(hand.handname,"fpkgXmlEnc")==0)
						hand.handp=fpkgXmlEnc;
					else
					if(strcmp(hand.handname,"fpkgXmlDec")==0)
						hand.handp=fpkgXmlDec;
					else
					if(strcmp(hand.handname,"fpkgJsnEnc")==0)
						hand.handp=fpkgJsnEnc;
					else
					if(strcmp(hand.handname,"fpkgJsnDec")==0)
						hand.handp=fpkgJsnDec;
					else
					if(strcmp(hand.handname,"fpkgSetEnc")==0)
						hand.handp=fpkgSetEnc;
					else
					if(strcmp(hand.handname,"fpkgUrlEnc")==0)
						hand.handp=fpkgUrlEnc;
					else
					if(strcmp(hand.handname,"fpkgUrlDec")==0)
						hand.handp=fpkgUrlDec;
					else
					if(strcmp(hand.handname,"fpkgHexEnc")==0)
						hand.handp=fpkgHexEnc;
					else
					if(strcmp(hand.handname,"fpkgHexDec")==0)
						hand.handp=fpkgHexDec;
					else
					if(strcmp(hand.handname,"fpkgB64Enc")==0)
						hand.handp=fpkgB64Enc;
					else
					if(strcmp(hand.handname,"fpkgB64Dec")==0)
						hand.handp=fpkgB64Dec;
					else
					if(strcmp(hand.handname,"fpkgDigEnc")==0)
						hand.handp=fpkgDigEnc;
					else
					if(strcmp(hand.handname,"fpkgCipEnc")==0)
						hand.handp=fpkgCipEnc;
					else
					if(strcmp(hand.handname,"fpkgCipDec")==0)
						hand.handp=fpkgCipDec;
					else
					if(strcmp(hand.handname,"fpkgRsaEnc")==0)
						hand.handp=fpkgRsaEnc;
					else
					if(strcmp(hand.handname,"fpkgRsaDec")==0)
						hand.handp=fpkgRsaDec;
					else
					{
						hand.handp=dlsym(bsnhandle,hand.handname);
						if(hand.handp==NULL)
						{
							mlogError("dlsym",0,dlerror(),"[%s]",hand.handname);
							return -1;
						}
					}
				}

				if
				(
					strncmp(hand.handname,"fpkgVar",7)==0||
					strncmp(hand.handname,"fpkgFix",7)==0||
					strncmp(hand.handname,"fpkgXml",7)==0||
					strncmp(hand.handname,"fpkgJsn",7)==0
				)
				{
					nodes[alloc]=calloc(64,sizeof(struct tpkgNode));
					hand.nodeindex=alloc++;
				}

				result=fpkgStackPush(&stack,&hand);
				if(result==-1)
					return -1;

				position1=position2+1;
			}
			else
			if(*position1==')')
			{
				position1++;

				result=fpkgStackPull(&stack,&hand);
				if(result==-1)
					return -1;

				if
				(
					strncmp(hand.handname,"fpkgVar",7)==0||
					strncmp(hand.handname,"fpkgFix",7)==0||
					strncmp(hand.handname,"fpkgXml",7)==0||
					strncmp(hand.handname,"fpkgJsn",7)==0
				)
				{
					hand.nodep=(char*)nodep;
					memcpy(nodep,nodes[hand.nodeindex],sizeof(struct tpkgNode)*lasts[hand.nodeindex]);
					nodep+=lasts[hand.nodeindex];
					if((char*)handp<=(char*)nodep)
						return -1;
					hand.nodecount=lasts[hand.nodeindex];
					free(nodes[hand.nodeindex]);
				}

				if
				(	stack.last&&
					(
						strncmp(stack.last->handname,"fpkgVar",7)==0||
						strncmp(stack.last->handname,"fpkgFix",7)==0||
						strncmp(stack.last->handname,"fpkgXml",7)==0||
						strncmp(stack.last->handname,"fpkgJsn",7)==0
					)
				)
				{
					struct tpkgNode *node;
					node=nodes[stack.last->nodeindex]+lasts[stack.last->nodeindex];
					node->nodename[0]=index;
					hand.middle=index++;
					stats[stack.last->nodeindex]=1;
				}
				else
				{
					hand.middle=0X20;
				}

				memcpy(handp--,&hand,sizeof(struct tpkgHand));
				if((char*)handp<=(char*)nodep)
					return -1;
			}
			else
			{
				struct tpkgNode *node;
				node=nodes[stack.last->nodeindex]+lasts[stack.last->nodeindex];
				lasts[stack.last->nodeindex]++;

				if(stats[stack.last->nodeindex]==0)
				{
					position2=strchr(position1,' ');
					*position2='\0';

					if(*position1==0X7E)
					{
						node->nodename[0]=0X7E;
						*(short*)(node->nodename+1)=strlen(position1+1);
						strcpy(node->nodename+1+sizeof(short),position1+1);
					}
					else
					{
						position2=strchr(position1+1,'_');
						if(position2==NULL)
						{
							strcpy(node->nodename,position1);
							node->handp=NULL;
						}
						else
						{
							strncpy(node->nodename,position1,position2-position1);
							node->nodename[position2-position1]='\0';

							position1=position2+1;
							position2=strchr(position1,'_');
							if(position2==NULL)
								return -1;
							strncpy(node->handname,position1,position2-position1);
							node->handname[position2-position1]='\0';

							position1=position2+1;
							strcpy(node->handargs,position1);

							if(strcmp(node->handname,"fpkgVarEnc")==0)
								node->handp=fpkgVarEnc;
							else
							if(strcmp(node->handname,"fpkgVarDec")==0)
								node->handp=fpkgVarDec;
							else
							if(strcmp(node->handname,"fpkgFixEnc")==0)
								node->handp=fpkgFixEnc;
							else
							if(strcmp(node->handname,"fpkgFixDec")==0)
								node->handp=fpkgFixDec;
							else
							if(strcmp(node->handname,"fpkgXmlEnc")==0)
								node->handp=fpkgXmlEnc;
							else
							if(strcmp(node->handname,"fpkgXmlDec")==0)
								node->handp=fpkgXmlDec;
							else
							if(strcmp(node->handname,"fpkgJsnEnc")==0)
								node->handp=fpkgJsnEnc;
							else
							if(strcmp(node->handname,"fpkgJsnDec")==0)
								node->handp=fpkgJsnDec;
							else
							if(strcmp(node->handname,"fpkgSetEnc")==0)
								node->handp=fpkgSetEnc;
							else
							if(strcmp(node->handname,"fpkgUrlEnc")==0)
								node->handp=fpkgUrlEnc;
							else
							if(strcmp(node->handname,"fpkgUrlDec")==0)
								node->handp=fpkgUrlDec;
							else
							if(strcmp(node->handname,"fpkgHexEnc")==0)
								node->handp=fpkgHexEnc;
							else
							if(strcmp(node->handname,"fpkgHexDec")==0)
								node->handp=fpkgHexDec;
							else
							if(strcmp(node->handname,"fpkgB64Enc")==0)
								node->handp=fpkgB64Enc;
							else
							if(strcmp(node->handname,"fpkgB64Dec")==0)
								node->handp=fpkgB64Dec;
							else
							if(strcmp(node->handname,"fpkgDigEnc")==0)
								node->handp=fpkgDigEnc;
							else
							if(strcmp(node->handname,"fpkgCipEnc")==0)
								node->handp=fpkgCipEnc;
							else
							if(strcmp(node->handname,"fpkgCipDec")==0)
								node->handp=fpkgCipDec;
							else
							if(strcmp(node->handname,"fpkgRsaEnc")==0)
								node->handp=fpkgRsaEnc;
							else
							if(strcmp(node->handname,"fpkgRsaDec")==0)
								node->handp=fpkgRsaDec;
							else
							{
								node->handp=dlsym(bsnhandle,node->handname);
								if(node->handp==NULL)
								{
									mlogError("dlsym",0,dlerror(),"[%s]",node->handname);
									return -1;
								}
							}
						}
					}

					position1+=strlen(position1)+1;
				}
				else
				{
					stats[stack.last->nodeindex]=0;
				}

				if(strncmp(stack.last->handname,"fpkgVar",7)==0)
				{
					position2=strchr(position1,' ');
					int m;
					int n;
					for(m=0,n=0;m<position2-position1;m++)
					{
						if(m%2==0)
							if(isdigit(position1[m]))
								node->format[1+n]=position1[m]-'0'<<4;
							else
								node->format[1+n]=position1[m]-'A'+0X0A<<4;
						else
							if(isdigit(position1[m]))
								node->format[1+n++]|=position1[m]-'0';
							else
								node->format[1+n++]|=position1[m]-'A'+0X0A;
					}
					node->format[1+n]='\0';

					node->format[0]=(position2-position1)/2;

					position1=position2+1;
				}
				else
				if(strncmp(stack.last->handname,"fpkgFix",7)==0)
				{
					*(short*)node->format=atoi(position1);

					position2=strchr(position1,',');
					position1=position2+1;
					position2=strchr(position1,' ');
					if(isdigit(position1[0]))
						node->format[sizeof(short)]=position1[0]-'0'<<4&0XF0;
					else
						node->format[sizeof(short)]=position1[0]-'A'+0X0A<<4&0XF0;
					if(isdigit(position1[1]))
						node->format[sizeof(short)]|=position1[1]-'0'&0X0F;
					else
						node->format[sizeof(short)]|=position1[1]-'A'+0X0A&0X0F;

					position1=position2+1;
				}
				else
				if(strncmp(stack.last->handname,"fpkgXml",7)==0)
				{
					position2=strchr(position1,' ');
					strncpy(node->format,position1+1,position2-position1-1);
					node->format[position2-position1-1]='/';
					node->format[position2-position1]='\0';
					position1=position2+1;
				}
				else
				if(strncmp(stack.last->handname,"fpkgJsn",7)==0)
				{
					position2=strchr(position1,' ');
					strncpy(node->format,position1+1,position2-position1-1);
					node->format[position2-position1-1]='/';
					node->format[position2-position1]='\0';
					position1=position2+1;
				}
			}
		}

		rulep++;
		vpkgRuleCount++;
	}

	qsort(vpkgRuleHead,vpkgRuleCount,sizeof(struct tpkgRule),
		(int(*)(const void*,const void*))strcmp);

	fclose(pkgfp);
	return 0;
}

/*========================================*\
    功能 : 报文编码
    参数 : (输入)渠道代码
           (输入)交易代码
           (输出)正式报文数据
           (输出)临时报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgEnc(char *lnkcode,char *trncode,char **fmldata,char **tmpdata,int *size)
{
	int result;

	int i;
	int j;

	char pkgcode[32];
	sprintf(pkgcode,"%s_%s_ENCODE",lnkcode,trncode);

	struct tpkgRule *rule;
	rule=(struct tpkgRule*)bsearch(pkgcode,vpkgRuleHead,vpkgRuleCount,sizeof(struct tpkgRule),(int(*)(const void*,const void*))strcmp);
	if(rule==NULL)
		return -100;
	struct tpkgHand *hand;
	hand=(struct tpkgHand*)rule->handp;

	char *middle[16];

	for(i=0;i<rule->handcount;i++)
	{
		if(strcmp(hand->handname,"array")!=0)
		{
			if
			(
				strncmp(hand->handname,"fpkgVar",7)==0||
				strncmp(hand->handname,"fpkgFix",7)==0||
				strncmp(hand->handname,"fpkgXml",7)==0||
				strncmp(hand->handname,"fpkgJsn",7)==0
			)
			{
				char *move;
				move=*fmldata;

				*size=0;

				if(strcmp((hand-1)->handname,"array")!=0)
				{
					result=((int(*)(struct tpkgNode*,char**,int,int,char**,int*))hand->handp)((struct tpkgNode*)hand->nodep,middle,hand->nodecount,0,&move,size);
					if(result==-1)
						return -1;
				}
				else
				{
					int count;
					result=fmmpValGet((hand-1)->handargs,0,&count,0);
					if(result==-1)
						return -1;

					for(j=0;j<count;j++)
					{
						result=((int(*)(struct tpkgNode*,char**,int,int,char**,int*))hand->handp)((struct tpkgNode*)hand->nodep,middle,hand->nodecount,j,&move,size);
						if(result==-1)
							return -1;
					}
				}
			}
			else
			{
				result=((int(*)(char**,char**,int*,char*))hand->handp)(fmldata,tmpdata,size,hand->handargs);
				if(result==-1)
					return -1;
			}

			if(hand->visual=='-')
			{
				flogDepend("#[%-15s][%4d][%.*s]",hand->handname,*size,*size,*fmldata);
			}
			else
			if(hand->visual=='+')
			{
				fpkgHexEnc(fmldata,tmpdata,size,"upper");
				flogDepend("#[%-15s][%4d][%.*s]",hand->handname,*size/2,*size,*fmldata);
				mpkgSwap(*fmldata,*tmpdata);
				*size/=2;
			}
		}

		if(hand->middle!=0X20)
		{
			middle[hand->middle]=(char*)malloc(sizeof(short)+*size);
			if(middle[hand->middle]==NULL)
			{
				mlogError("malloc",0,"0","[%d]",sizeof(short)+*size);
				return -1;
			}
			*(short*)middle[hand->middle]=*size;
			memcpy(middle[hand->middle]+sizeof(short),*fmldata,*size);
		}

		hand--;
	}

	return 0;
}

/*========================================*\
    功能 : 报文解码
    参数 : (输入)渠道代码
           (输入)交易代码
           (出入)正式报文数据
           (出入)临时报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgDec(char *lnkcode,char *trncode,char **fmldata,char **tmpdata,int size)
{
	int result;

	int i;
	int j;

	char pkgcode[32];
	sprintf(pkgcode,"%s_%s_DECODE",lnkcode,trncode);

	struct tpkgRule *rule;
	rule=(struct tpkgRule*)bsearch(pkgcode,vpkgRuleHead,vpkgRuleCount,sizeof(struct tpkgRule),(int(*)(const void*,const void*))strcmp);
	if(rule==NULL)
		return -100;
	struct tpkgHand *hand;
	hand=(struct tpkgHand*)rule->handp;

	char *middle[16];

	for(i=0;i<rule->handcount;i++)
	{
		if(hand->middle!=0X20)
		{
			if(middle[hand->middle]==NULL)
				return -100;
			size=*(short*)middle[hand->middle];
			memcpy(*fmldata,middle[hand->middle]+sizeof(short),size);
			free(middle[hand->middle]);
		}

		if(strcmp(hand->handname,"array")!=0)
		{
			if(hand->visual=='-')
			{
				flogDepend("#[%-15s][%4d][%.*s]",hand->handname,size,size,*fmldata);
			}
			else
			if(hand->visual=='+')
			{
				fpkgHexEnc(fmldata,tmpdata,&size,"upper");
				flogDepend("#[%-15s][%4d][%.*s]",hand->handname,size/2,size,*fmldata);
				mpkgSwap(*fmldata,*tmpdata);
				size/=2;
			}

			if
			(
				strncmp(hand->handname,"fpkgVar",7)==0||
				strncmp(hand->handname,"fpkgFix",7)==0||
				strncmp(hand->handname,"fpkgXml",7)==0||
				strncmp(hand->handname,"fpkgJsn",7)==0
			)
			{
				char *move;
				move=*fmldata;

				if(strcmp((hand-1)->handname,"array")!=0)
				{
					result=((int(*)(struct tpkgNode*,char**,int,int,char**,int))hand->handp)((struct tpkgNode*)hand->nodep,middle,hand->nodecount,0,&move,size);
					if(result==-1)
						return -1;
				}
				else
				{
					for(j=0;size>move-*fmldata;j++)
					{
						result=((int(*)(struct tpkgNode*,char**,int,int,char**,int))hand->handp)((struct tpkgNode*)hand->nodep,middle,hand->nodecount,j,&move,size-(move-*fmldata));
						if(result==-1)
							return -1;
					}

					result=fmmpValSet((hand-1)->handargs,0,&i,0);
					if(result==-1)
						return -1;
				}
			}
			else
			{
				result=((int(*)(char**,char**,int*,char*))hand->handp)(fmldata,tmpdata,&size,hand->handargs);
				if(result==-1)
					return -1;
			}
		}

		hand++;
	}

	return 0;
}

/*========================================*\
    功能 : 字符编码转换
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)来源字符集
                 目的字符集
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgSetEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *position1;
	char *position2;

	position1=argument;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char srcset[16];
	assert(position2-position1<=sizeof(srcset)-1);
	strncpy(srcset,position1,position2-position1);
	srcset[position2-position1]='\0';

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char dstset[16];
	assert(position2-position1<=sizeof(dstset)-1);
	strncpy(dstset,position1,position2-position1);
	dstset[position2-position1]='\0';

	position1=position2+1;
	int buffersize;
	buffersize=atoi(position1);

	iconv_t cvt;
	cvt=iconv_open(dstset,srcset);
	if(cvt==(iconv_t)-1)
	{
		mlogError("iconv_open",errno,strerror(errno),"[%s][%s]",dstset,srcset);
		return -1;
	}

	size_t srcsize;
	srcsize=*size;
	size_t dstsize;
	dstsize=buffersize;

	char *_fmldata;
	_fmldata=*fmldata;
	char *_tmpdata;
	_tmpdata=*tmpdata;

	result=iconv(cvt,&_fmldata,&srcsize,&_tmpdata,&dstsize);
	if(result==-1)
	{
		iconv_close(cvt);
		mlogError("iconv",errno,strerror(errno),"[]");
		return -1;
	}

	*size=buffersize-dstsize;
	(*tmpdata)[*size]='\0';

	iconv_close(cvt);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : URL转换编码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)大写(upper)/小写(lower)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgUrlEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int i;
	int j;
	for(i=0,j=0;i<*size;i++,j++)
	{
		if(isalnum((*fmldata)[i]))
			(*tmpdata)[j]=(*fmldata)[i];
		else
		if
		(
			(*fmldata)[i]=='.'||
			(*fmldata)[i]=='-'||
			(*fmldata)[i]=='_'||
			(*fmldata)[i]=='*'
		)
			(*tmpdata)[j]=(*fmldata)[i];
		else
		if((*fmldata)[i]==' ')
			(*tmpdata)[j]='+';
		else
		{
			(*tmpdata)[j]='%';

			char temp;

			j++;
			temp=(*fmldata)[i]>>4&0X0F;
			if(temp<0X0A)
				(*tmpdata)[j]=temp+'0';
			else
			{
				if(argument[0]=='l')
					(*tmpdata)[j]=temp-0X0A+'a';
				else
					(*tmpdata)[j]=temp-0X0A+'A';
			}

			j++;
			temp=(*fmldata)[i]&0X0F;
			if(temp<0X0A)
				(*tmpdata)[j]=temp+'0';
			else
			{
				if(argument[0]=='l')
					(*tmpdata)[j]=temp-0X0A+'a';
				else
					(*tmpdata)[j]=temp-0X0A+'A';
			}
		}
	}

	*size=j;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : URL转换解码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)大写(upper)/小写(lower)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgUrlDec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int i;
	int j;
	for(i=0,j=0;i<*size;i++,j++)
	{
		if((*fmldata)[i]=='+')
			(*tmpdata)[j]=' ';
		else
		if((*fmldata)[i]=='%')
		{
			char temp;

			i++;
			if(isdigit((*fmldata)[i]))
				temp=(*fmldata)[i]-'0';
			else
			if(argument[0]=='l'&&islower((*fmldata)[i]))
				temp=(*fmldata)[i]-'a'+0X0A;
			else
			if(argument[0]=='u'&&isupper((*fmldata)[i]))
				temp=(*fmldata)[i]-'A'+0X0A;
			else
				return -1;
			(*tmpdata)[j]=temp<<4&0XF0;

			i++;
			if(isdigit((*fmldata)[i]))
				temp=(*fmldata)[i]-'0';
			else
			if(argument[0]=='l'&&islower((*fmldata)[i]))
				temp=(*fmldata)[i]-'a'+0X0A;
			else
			if(argument[0]=='u'&&isupper((*fmldata)[i]))
				temp=(*fmldata)[i]-'A'+0X0A;
			else
				return -1;
			(*tmpdata)[j]|=temp&0X0F;
		}
		else
			(*tmpdata)[j]=(*fmldata)[i];
	}

	*size=j;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 十六进制转换编码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)大写(upper)/小写(lower)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgHexEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int i;
	for(i=0;i<*size;i++)
	{
		char temp;

		temp=(*fmldata)[i]>>4&0X0F;
		if(temp<0X0A)
			(*tmpdata)[2*i+0]=temp+'0';
		else
		{
			if(argument[0]=='l')
				(*tmpdata)[2*i+0]=temp-0X0A+'a';
			else
				(*tmpdata)[2*i+0]=temp-0X0A+'A';
		}

		temp=(*fmldata)[i]&0X0F;
		if(temp<0X0A)
			(*tmpdata)[2*i+1]=temp+'0';
		else
		{
			if(argument[0]=='l')
				(*tmpdata)[2*i+1]=temp-0X0A+'a';
			else
				(*tmpdata)[2*i+1]=temp-0X0A+'A';
		}
	}

	*size*=2;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 十六进制转换解码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)大写(upper)/小写(lower)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgHexDec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int i;
	for(i=0;i<*size;i++)
	{
		char temp;

		if(isdigit((*fmldata)[i]))
			temp=(*fmldata)[i]-'0';
		else
		if(argument[0]=='l'&&islower((*fmldata)[i]))
			temp=(*fmldata)[i]-'a'+0X0A;
		else
		if(argument[0]=='u'&&isupper((*fmldata)[i]))
			temp=(*fmldata)[i]-'A'+0X0A;
		else
			return -1;

		if(i%2==0)
			(*tmpdata)[i/2]=temp<<4;
		else
			(*tmpdata)[i/2]|=temp&0X0F;
	}

	if(*size%2!=0)
	{
		(*tmpdata)[i/2]&=0XF0;
		(*size)++;
	}

	*size/=2;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : BASE64算法编码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)无
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgB64Enc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;
	result=EVP_EncodeBlock(*tmpdata,*fmldata,*size);
	if(result==-1)
	{
		mlogError("EVP_EncodeBlock",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%d][%s]",*size,*fmldata);
		ERR_free_strings();
		ERR_remove_state(0);
	}

	*size=result;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : BASE64算法解码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)无
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgB64Dec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;
	result=EVP_DecodeBlock(*tmpdata,*fmldata,*size);
	if(result==-1)
	{
		mlogError("EVP_DecodeBlock",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%d][%s]",*size,*fmldata);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}


	if((*fmldata)[*size-2]=='=')
		result-=2;
	else
	if((*fmldata)[*size-1]=='=')
		result-=1;

	*size=result;
	(*tmpdata)[*size]='\0';

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 信息摘要算法
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)算法
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgDigEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	result=pthread_mutex_lock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}
	OpenSSL_add_all_digests();
	const EVP_MD *digest;
	digest=EVP_get_digestbyname(argument);
	if(digest==NULL)
	{
		EVP_cleanup();
		mlogError("EVP_get_digestbyname",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}
	EVP_cleanup();
	result=pthread_mutex_unlock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}

	EVP_MD_CTX ctx;
	EVP_MD_CTX_init(&ctx);

	result=EVP_DigestInit_ex(&ctx,digest,NULL);
	if(result==-1)
	{
		EVP_MD_CTX_cleanup(&ctx);
		mlogError("EVP_DigestInit_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	result=EVP_DigestUpdate(&ctx,*fmldata,*size);
	if(result==-1)
	{
		EVP_MD_CTX_cleanup(&ctx);
		mlogError("EVP_DigestUpdate",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%d][%s]",*size,*fmldata);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	result=EVP_DigestFinal_ex(&ctx,*tmpdata,size);
	if(result==-1)
	{
		EVP_MD_CTX_cleanup(&ctx);
		mlogError("EVP_DigestFinal_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	EVP_MD_CTX_cleanup(&ctx);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 对称加密算法加密
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)算法
                 密钥
                 向量
                 填充模式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgCipEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *position1;
	char *position2;

	position1=argument;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char algorithm[32];
	strncpy(algorithm,position1,position2-position1);
	algorithm[position2-position1]='\0';

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char keymmp[64];
	strncpy(keymmp,position1,position2-position1);
	keymmp[position2-position1]='\0';
	char *key;
	result=fmmpRefGet(keymmp,0,&key,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char vecmmp[64];
	strncpy(vecmmp,position1,position2-position1);
	vecmmp[position2-position1]='\0';
	char *vec;
	result=fmmpRefGet(vecmmp,0,&vec,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	char mode;
	mode=*position1-'0';

	result=pthread_mutex_lock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}
	OpenSSL_add_all_ciphers();
	const EVP_CIPHER *cipher;
	cipher=EVP_get_cipherbyname(algorithm);
	if(cipher==NULL)
	{
		EVP_cleanup();
		mlogError("EVP_get_cipherbyname",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%s]",algorithm);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}
	EVP_cleanup();
	result=pthread_mutex_unlock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);

	result=EVP_EncryptInit_ex(&ctx,cipher,NULL,key,vec);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_EncryptInit_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%s][%s]",key,vec);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	EVP_CIPHER_CTX_set_padding(&ctx,mode);

	result=EVP_EncryptUpdate(&ctx,*tmpdata,size,*fmldata,*size);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_EncryptUpdate",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%d][%s]",*size,*fmldata);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	int final;
	result=EVP_EncryptFinal_ex(&ctx,*tmpdata+*size,&final);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_EncryptFinal_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	*size+=final;
	(*tmpdata)[*size]='\0';

	EVP_CIPHER_CTX_cleanup(&ctx);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 对称加密算法解密
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)算法
                 密钥
                 向量
                 填充模式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgCipDec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *position1;
	char *position2;

	position1=argument;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char algorithm[32];
	strncpy(algorithm,position1,position2-position1);
	algorithm[position2-position1]='\0';

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char keymmp[64];
	strncpy(keymmp,position1,position2-position1);
	keymmp[position2-position1]='\0';
	char *key;
	result=fmmpRefGet(keymmp,0,&key,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char vecmmp[64];
	strncpy(vecmmp,position1,position2-position1);
	vecmmp[position2-position1]='\0';
	char *vec;
	result=fmmpRefGet(vecmmp,0,&vec,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	char mode;
	mode=*position1-'0';

	result=pthread_mutex_lock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}
	OpenSSL_add_all_ciphers();
	const EVP_CIPHER *cipher;
	cipher=EVP_get_cipherbyname(algorithm);
	if(cipher==NULL)
	{
		EVP_cleanup();
		mlogError("EVP_get_cipherbyname",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%s]",algorithm);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}
	EVP_cleanup();
	result=pthread_mutex_unlock(&vpkgMutex);
	if(result!=0)
	{
		mlogError("pthread_mutex_lock",result,strerror(result),"[]");
		return -1;
	}

	EVP_CIPHER_CTX ctx;
	EVP_CIPHER_CTX_init(&ctx);

	result=EVP_DecryptInit_ex(&ctx,cipher,NULL,key,vec);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_DecryptInit_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%s][%s]",key,vec);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	EVP_CIPHER_CTX_set_padding(&ctx,mode);

	result=EVP_DecryptUpdate(&ctx,*tmpdata,size,*fmldata,*size);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_DecryptUpdate",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[%d][%s]",*size,*fmldata);
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	int final;
	result=EVP_DecryptFinal_ex(&ctx,*tmpdata+*size,&final);
	if(result==-1)
	{
		EVP_CIPHER_CTX_cleanup(&ctx);
		mlogError("EVP_DecryptFinal_ex",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
		ERR_free_strings();
		ERR_remove_state(0);
		return -1;
	}

	*size+=final;
	(*tmpdata)[*size]='\0';

	EVP_CIPHER_CTX_cleanup(&ctx);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*
#define RSA_PKCS1_PADDING      1
#define RSA_SSLV23_PADDING     2
#define RSA_NO_PADDING         3
#define RSA_PKCS1_OAEP_PADDING 4
#define RSA_X931_PADDING       5
*/

/*========================================*\
    功能 : RSA算法加密
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)E键
                 N键
                 D键
                 密钥类型
                 填充模式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgRsaEnc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *position1;
	char *position2;

	position1=argument;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char ekeymmp[64];
	strncpy(ekeymmp,position1,position2-position1);
	ekeymmp[position2-position1]='\0';
	char *ekey;
	result=fmmpRefGet(ekeymmp,0,&ekey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char nkeymmp[64];
	strncpy(nkeymmp,position1,position2-position1);
	nkeymmp[position2-position1]='\0';
	char *nkey;
	result=fmmpRefGet(nkeymmp,0,&nkey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char dkeymmp[64];
	strncpy(dkeymmp,position1,position2-position1);
	dkeymmp[position2-position1]='\0';
	char *dkey;
	result=fmmpRefGet(dkeymmp,0,&dkey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	if(position2-position1!=1)
		return -1;
	char mode1;
	mode1=*position1;
	if(mode1!='1'&&mode1!='2')
		return -1;

	position1=position2+1;
	if(strlen(position1)!=1)
		return -1;
	char mode2;
	mode2=*position1-'0';

	RSA *ctx;
	ctx=RSA_new();

	ctx->e=BN_new();
	BN_dec2bn(&ctx->e,ekey);
	ctx->n=BN_new();
	BN_dec2bn(&ctx->n,nkey);
	ctx->d=BN_new();
	BN_dec2bn(&ctx->d,dkey);

	int dstsize=0;

	int max;
	max=BN_num_bytes(ctx->n)-11;
	int i;
	for(i=0;max*i<*size;i++)
	{
		if(mode1=='1')
		{
			result=RSA_public_encrypt(max<(*size-max*i)?max:(*size-max*i),*fmldata+max*i,*tmpdata+dstsize,ctx,mode2);
			if(result==-1)
			{
				RSA_free(ctx);
				mlogError("RSA_public_encrypt",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
				ERR_free_strings();
				ERR_remove_state(0);
				return -1;
			}
		}
		else
		{
			result=RSA_private_encrypt(max<(*size-max*i)?max:(*size-max*i),*fmldata+max*i,*tmpdata+dstsize,ctx,mode2);
			if(result==-1)
			{
				RSA_free(ctx);
				mlogError("RSA_private_encrypt",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
				ERR_free_strings();
				ERR_remove_state(0);
				return -1;
			}
		}

		dstsize+=result;
	}

	*size=dstsize;
	(*tmpdata)[*size]='\0';

	RSA_free(ctx);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : RSA算法解密
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)E键
                 N键
                 D键
                 密钥类型
                 填充模式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgRsaDec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *position1;
	char *position2;

	position1=argument;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char ekeymmp[64];
	strncpy(ekeymmp,position1,position2-position1);
	ekeymmp[position2-position1]='\0';
	char *ekey;
	result=fmmpRefGet(ekeymmp,0,&ekey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char nkeymmp[64];
	strncpy(nkeymmp,position1,position2-position1);
	nkeymmp[position2-position1]='\0';
	char *nkey;
	result=fmmpRefGet(nkeymmp,0,&nkey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	char dkeymmp[64];
	strncpy(dkeymmp,position1,position2-position1);
	dkeymmp[position2-position1]='\0';
	char *dkey;
	result=fmmpRefGet(dkeymmp,0,&dkey,0);
	if(result==-1)
		return -1;

	position1=position2+1;
	position2=strchr(position1,'_');
	if(position2==NULL)
		return -1;
	if(position2-position1!=1)
		return -1;
	char mode1;
	mode1=*position1;
	if(mode1!='1'&&mode1!='2')
		return -1;

	position1=position2+1;
	if(strlen(position1)!=1)
		return -1;
	char mode2;
	mode2=*position1-'0';

	RSA *ctx;
	ctx=RSA_new();

	ctx->e=BN_new();
	BN_dec2bn(&ctx->e,ekey);
	ctx->n=BN_new();
	BN_dec2bn(&ctx->n,nkey);
	ctx->d=BN_new();
	BN_dec2bn(&ctx->d,dkey);

	int dstsize=0;

	int max;
	max=BN_num_bytes(ctx->n);
	int i;
	for(i=0;max*i<*size;i++)
	{
		if(mode1=='1')
		{
			result=RSA_private_decrypt(max,*fmldata+max*i,*tmpdata+dstsize,ctx,mode2);
			if(result==-1)
			{
				RSA_free(ctx);
				mlogError("RSA_private_decrypt",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
				ERR_free_strings();
				ERR_remove_state(0);
				return -1;
			}
		}
		else
		{
			result=RSA_public_decrypt(max,*fmldata+max*i,*tmpdata+dstsize,ctx,mode2);
			if(result==-1)
			{
				RSA_free(ctx);
				mlogError("RSA_public_decrypt",ERR_get_error(),ERR_error_string(ERR_get_error(),NULL),"[]");
				ERR_free_strings();
				ERR_remove_state(0);
				return -1;
			}
		}

		dstsize+=result;
	}
	if(max*i!=*size)
		return -1;

	*size=dstsize;
	(*tmpdata)[*size]='\0';

	RSA_free(ctx);

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}
