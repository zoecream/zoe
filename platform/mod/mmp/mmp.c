/*========================================*\
    文件 : mmp.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include <pthread.h>

#include <log.h>
#include <mmp.h>

//一次性执行.
pthread_once_t vmmpOnce=PTHREAD_ONCE_INIT;
//本地化存储.
pthread_key_t vmmpHead;

//内存池哈系表分组.
#define cmmpHashSize 13

struct tmmpHeapItem
{
	//空间状态.
	short stat;
	//空间尺寸.
	short size;
	//上一空间偏移.
	short prev;
	//下一空间偏移.
	short next;
};
struct tmmpHashItem
{
	//节点标记.
	short mark;
	//节点尺寸.
	short size;
	//数组空间.
	short room;
	//节点数据.
	short data;
	//下一节点.
	short next;
};

/*========================================*\
    功能 : 分配内存池堆空间
    参数 : (输入)堆空间指针
           (输入)堆空间尺寸
           (输出)堆空间位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpHeapInit(char *head,short size,short *position)
{
	struct tmmpHeapItem *temp1;
	temp1=(struct tmmpHeapItem*)(head+*(short*)head);

	while(1)
	{
		if(temp1->stat==0)
		{
			if(temp1->size>=size)
			{
				temp1->stat=1;

				if(temp1->size-size>=sizeof(struct tmmpHeapItem)+sizeof(int))
				{
					struct tmmpHeapItem *temp2;
					temp2=(struct tmmpHeapItem*)((char*)temp1+sizeof(struct tmmpHeapItem)+size);
					temp2->stat=0;
					temp2->size=temp1->size-size-sizeof(struct tmmpHeapItem);
					temp2->prev=(char*)temp1-head;
					temp2->next=temp1->next;
					temp1->size=size;
					temp1->next=(char*)temp2-head;
				}

				*(short*)head=temp1->next;
				if(position!=NULL)
					*position=(char*)temp1+sizeof(struct tmmpHeapItem)-head;
				return 0;
			}
		}

		if(temp1->next==*(short*)head)
			return -1;
		temp1=(struct tmmpHeapItem*)(head+temp1->next);
	}
}

/*========================================*\
    功能 : 释放内存池堆空间
    参数 : (输入)堆空间指针
           (输入)堆空间位置
    返回 : 空
\*========================================*/
void fmmpHeapFree(char *head,short position)
{
	struct tmmpHeapItem *temp1;
	temp1=(struct tmmpHeapItem*)(head+position-sizeof(struct tmmpHeapItem));

	temp1->stat=0;
	//bzero((char*)temp1+sizeof(struct tmmpHeapItem),temp1->size);

	if(((struct tmmpHeapItem*)(head+temp1->next))->stat==0
		&&temp1->next!=sizeof(short))
	{
		struct tmmpHeapItem *temp2;
		temp2=(struct tmmpHeapItem*)(head+temp1->next);
		temp1->size+=sizeof(struct tmmpHeapItem)+temp2->size;
		((struct tmmpHeapItem*)(head+temp2->next))->prev=temp2->prev;
		((struct tmmpHeapItem*)(head+temp2->prev))->next=temp2->next;
		//bzero(temp2,sizeof(struct tmmpHeapItem));

		if(*(short*)head==(char*)temp2-head)
			*(short*)head=(char*)temp1-head;
	}
	if(((struct tmmpHeapItem*)(head+temp1->prev))->stat==0
		&&temp1->prev!=0)
	{
		((struct tmmpHeapItem*)(head+temp1->prev))->size
			+=sizeof(struct tmmpHeapItem)+temp1->size;
		((struct tmmpHeapItem*)(head+temp1->next))->prev=temp1->prev;
		((struct tmmpHeapItem*)(head+temp1->prev))->next=temp1->next;
		//bzero(temp1,sizeof(struct tmmpHeapItem));

		if(*(short*)head==(char*)temp1-head)
			*(short*)head=temp1->prev;
	}
	return;
}

/*========================================*\
    功能 : 重分内存池堆空间
    参数 : (输入)堆空间指针
           (输入)原空间位置
           (输入)新空间尺寸
           (输出)新空间位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpHeapRnit(char *head,short old,short size,short *new)
{
	struct tmmpHeapItem *temp1;
	temp1=(struct tmmpHeapItem*)(head+old-sizeof(struct tmmpHeapItem));

	if(temp1->size>=size)
	{
		//bzero((char*)temp1+sizeof(struct tmmpHeapItem),temp1->size);
		if(new!=NULL)
			*new=old;
		return 0;
	}

	if(((struct tmmpHeapItem*)(head+temp1->next))->stat==0)
	{
		if(temp1->size+sizeof(struct tmmpHeapItem)+
			((struct tmmpHeapItem*)(head+temp1->next))->size>=size)
		{
			//bzero((char*)temp1+sizeof(struct tmmpHeapItem),temp1->size);

			struct tmmpHeapItem *temp2;
			temp2=(struct tmmpHeapItem*)(head+temp1->next);
			temp1->size+=sizeof(struct tmmpHeapItem)+temp2->size;
			((struct tmmpHeapItem*)(head+temp2->next))->prev=temp2->prev;
			((struct tmmpHeapItem*)(head+temp2->prev))->next=temp2->next;
			//bzero(temp2,sizeof(struct tmmpHeapItem));

			if(temp1->size-size>=sizeof(struct tmmpHeapItem)+sizeof(int))
			{
				struct tmmpHeapItem *temp2;
				temp2=(struct tmmpHeapItem*)((char*)temp1+sizeof(struct tmmpHeapItem)+size);
				temp2->stat=0;
				temp2->size=temp1->size-size-sizeof(struct tmmpHeapItem);
				temp2->prev=(char*)temp1-head;
				temp2->next=temp1->next;
				temp1->size=size;
				temp1->next=(char*)temp2-head;
			}

			if
			(
				*(short*)head>(char*)temp1-head&&
				*(short*)head<temp1->next
			)
				*(short*)head=temp1->next;

			if(new!=NULL)
				*new=old;
			return 0;
		}
	}

	int result;
	result=fmmpHeapInit(head,size,new);
	if(result==-1)
		return -1;
	fmmpHeapFree(head,old);
	return 0;
}

/*========================================*\
    功能 : 计算内存池名称哈希值
    参数 : (输入)内存池名称
    返回 : (成功)内存池名称哈希值
           (失败)空
\*========================================*/
int fmmpHash(char *mark)
{
	int hash;
	hash=0;
	while(*mark!='\0')
	{
		hash=(hash<<4)+*mark;
		unsigned int temp;
		temp=hash&0XF0000000;
		if(temp!=0)
		{
			hash^=temp>>24;
			hash^=temp;
		}
		mark++;
	}
	hash%=cmmpHashSize;
	return hash;
}

/*========================================*\
    功能 : 创建内存池
    参数 : (输入)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpInit(int size)
{
	int result;

	void create(void)
	{
		pthread_key_create(&vmmpHead,NULL);
	}
	result=pthread_once(&vmmpOnce,create);
	if(result!=0)
	{
		mlogError("pthread_once",result,strerror(result),"[]");
		return -1;
	}
	char *head;
	head=(char*)malloc(size);
	if(head==NULL)
	{
		mlogError("malloc",0,"0","[]");
		return -1;
	}
	result=pthread_setspecific(vmmpHead,head);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 删除内存池
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpFree(void)
{
	int result;

	char *head;
	head=(char*)pthread_getspecific(vmmpHead);
	if(head!=NULL)
	{
		free(head);
		result=pthread_setspecific(vmmpHead,NULL);
		if(result!=0)
		{
			mlogError("pthread_setspecific",result,strerror(result),"[]");
			return -1;
		}
	}

	return 0;
}

/*========================================*\
    功能 : 重置内存池
    参数 : (输入)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpRnit(int size)
{
	char *head;
	head=(char*)pthread_getspecific(vmmpHead);
	*(short*)head=sizeof(short);
	((struct tmmpHeapItem*)(head+sizeof(short)))->stat=0;
	((struct tmmpHeapItem*)(head+sizeof(short)))->size=
		size-sizeof(short)-sizeof(struct tmmpHeapItem);
	((struct tmmpHeapItem*)(head+sizeof(short)))->prev=0;
	((struct tmmpHeapItem*)(head+sizeof(short)))->next=sizeof(short);
	short position;
	int result;
	result=fmmpHeapInit(head,sizeof(struct tmmpHashItem)*cmmpHashSize,&position);
	if(result==-1)
		return -1;
	bzero(head+position,sizeof(struct tmmpHashItem)*cmmpHashSize);
	return 0;
}

/*========================================*\
    功能 : 切换内存池
    参数 : (输入)新内存池指针
           (输出)原内存池指针
    返回 : 空
\*========================================*/
void fmmpSwap(void *new,void **old)
{
	if(old!=NULL)
		*old=pthread_getspecific(vmmpHead);
	if(new!=NULL)
		pthread_setspecific(vmmpHead,new);
}

/*========================================*\
    功能 : 设置内存池拷贝
    参数 : (输入)内存池名称
           (输入)内存池序号
           (输入)内存池拷贝
           (输入)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpValSet(char *mark,int number,void *data,int size)
{
	int result;

	char *head;
	head=(char*)pthread_getspecific(vmmpHead);

	if(mark[mark[0]=='_'?1:0]=='p')
		size=(size==0?strlen(data):size)+1;
	else
	if(mark[mark[0]=='_'?1:0]=='i')
		size=sizeof(int);
	else
	if(mark[mark[0]=='_'?1:0]=='l')
		size=sizeof(long);
	else
	if(mark[mark[0]=='_'?1:0]=='d')
		size=sizeof(double);
	else
	if(mark[mark[0]=='_'?1:0]=='c')
		size=sizeof(char);
	else
	if(mark[mark[0]=='_'?1:0]=='s')
		size=sizeof(short);
	else
	if(mark[mark[0]=='_'?1:0]=='f')
		size=sizeof(float);

	struct tmmpHashItem *item;
	item=(struct tmmpHashItem*)(head+sizeof(short)+sizeof(struct tmmpHeapItem))+fmmpHash(mark);
	if(item->mark==0)
	{
		result=fmmpHeapInit(head,strlen(mark)+1,&item->mark);
		if(result==-1)
			return -1;
		strcpy(head+item->mark,mark);

		if(mark[0]!='_')
		{
			item->size=size;

			result=fmmpHeapInit(head,item->size,&item->data);
			if(result==-1)
				return -1;
			memcpy(head+item->data,data,size);

			if(mark[mark[0]=='_'?1:0]=='p')
				*(head+item->data+size-1)='\0';
		}
		else
		{
			item->room=(number/8+1)*8;

			result=fmmpHeapInit(head,sizeof(short)*item->room,&item->size);
			if(result==-1)
				return -1;
			bzero(head+item->size,sizeof(short)*item->room);
			*(short*)(head+item->size+sizeof(short)*number)=size;

			result=fmmpHeapInit(head,sizeof(short)*item->room,&item->data);
			if(result==-1)
				return -1;
			bzero(head+item->data,sizeof(short)*item->room);
			short temp;
			result=fmmpHeapInit(head,size,&temp);
			if(result==-1)
				return -1;
			*(short*)(head+item->data+sizeof(short)*number)=temp;
			memcpy(head+temp,data,size);

			if(mark[mark[0]=='_'?1:0]=='p')
				*(head+temp+size-1)='\0';
		}
		return 0;
	}

	while(1)
	{
		if(strcmp(head+item->mark,mark)==0)
		{
			if(mark[0]!='_')
			{
				item->size=size;

				result=fmmpHeapRnit(head,item->data,size,&item->data);
				if(result==-1)
					return -1;
				memcpy(head+item->data,data,size);

				if(mark[mark[0]=='_'?1:0]=='p')
					*(head+item->data+size-1)='\0';
			}
			else
			{
				if(item->room<=number)
				{
					short roomrecord;
					roomrecord=item->room;
					item->room=(number/8+1)*8;

					short sizerecord;
					sizerecord=item->size;
					result=fmmpHeapRnit(head,item->size,sizeof(short)*item->room,&item->size);
					if(result==-1)
						return -1;
					bzero(head+item->size,sizeof(short)*item->room);
					memmove(head+item->size,head+sizerecord,sizeof(short)*roomrecord);

					short datarecord;
					datarecord=item->data;
					result=fmmpHeapRnit(head,item->data,sizeof(short)*item->room,&item->data);
					if(result==-1)
						return -1;
					bzero(head+item->data,sizeof(short)*item->room);
					memmove(head+item->data,head+datarecord,sizeof(short)*roomrecord);
				}

				*(short*)(head+item->size+sizeof(short)*number)=size;

				short temp;
				result=fmmpHeapInit(head,size,&temp);
				if(result==-1)
					return -1;
				*(short*)(head+item->data+sizeof(short)*number)=temp;
				memcpy(head+temp,data,size);

				if(mark[mark[0]=='_'?1:0]=='p')
					*(head+temp+size-1)='\0';
			}
			return 0;
		}
		if(item->next==0)
		{
			result=fmmpHeapInit(head,sizeof(struct tmmpHashItem),&item->next);
			if(result==-1)
				return -1;
			item=(struct tmmpHashItem*)(head+item->next);

			result=fmmpHeapInit(head,strlen(mark)+1,&item->mark);
			if(result==-1)
				return -1;
			strcpy(head+item->mark,mark);

			item->next=0;

			if(mark[0]!='_')
			{
				item->size=size;

				result=fmmpHeapInit(head,item->size,&item->data);
				if(result==-1)
					return -1;
				memcpy(head+item->data,data,size);

				if(mark[mark[0]=='_'?1:0]=='p')
					*(head+item->data+size-1)='\0';
			}
			else
			{
				item->room=(number/8+1)*8;

				result=fmmpHeapInit(head,sizeof(short)*item->room,&item->size);
				if(result==-1)
					return -1;
				bzero(head+item->size,sizeof(short)*item->room);
				*(short*)(head+item->size+sizeof(short)*number)=size;

				result=fmmpHeapInit(head,sizeof(short)*item->room,&item->data);
				if(result==-1)
					return -1;
				bzero(head+item->data,sizeof(short)*item->room);
				short temp;
				result=fmmpHeapInit(head,size,&temp);
				if(result==-1)
					return -1;
				*(short*)(head+item->data+sizeof(short)*number)=temp;
				memcpy(head+temp,data,size);

				if(mark[mark[0]=='_'?1:0]=='p')
					*(head+temp+size-1)='\0';
			}
			return 0;
		}
		item=(struct tmmpHashItem*)(head+item->next);
	}
}

/*========================================*\
    功能 : 获取内存池拷贝
    参数 : (输入)内存池名称
           (输入)内存池序号
           (输出)内存池拷贝
           (输出)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpValGet(char *mark,int number,void *data,int *size)
{
	char *head;
	head=(char*)pthread_getspecific(vmmpHead);

	struct tmmpHashItem *item;
	item=(struct tmmpHashItem*)(head+sizeof(short)+sizeof(struct tmmpHeapItem))+fmmpHash(mark);
	if(item->mark==0)
		return -1;

	while(1)
	{
		if(strcmp(head+item->mark,mark)==0)
		{
			if(mark[0]!='_')
			{
				if(size!=NULL)
					if(mark[0]=='p')
						*size=item->size-1;
					else
						*size=item->size;
				memcpy(data,head+item->data,item->size);
			}
			else
			{
				if(*(short*)(head+item->size+sizeof(short)*number)==0)
					return -1;
				if(size!=NULL)
					if(mark[1]=='p')
						*size=*(short*)(head+item->size+sizeof(short)*number)-1;
					else
						*size=*(short*)(head+item->size+sizeof(short)*number);
				memcpy(data,head+
					*(short*)(head+item->data+sizeof(short)*number),
					*(short*)(head+item->size+sizeof(short)*number));
			}
			return 0;
		}
		if(item->next==0)
			return -1;
		item=(struct tmmpHashItem*)(head+item->next);
	}
}

/*========================================*\
    功能 : 设置内存池引用
    参数 : (输入)内存池名称
           (输入)内存池序号
           (输入)内存池引用
           (输入)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpRefSet(char *mark,int number,void *data,int size)
{
	int result;

	char *head;
	head=(char*)pthread_getspecific(vmmpHead);

	if(mark[mark[0]=='_'?1:0]=='p')
		size=size+1;
	else
	if(mark[mark[0]=='_'?1:0]=='i')
		size=sizeof(int);
	else
	if(mark[mark[0]=='_'?1:0]=='l')
		size=sizeof(long);
	else
	if(mark[mark[0]=='_'?1:0]=='d')
		size=sizeof(double);
	else
	if(mark[mark[0]=='_'?1:0]=='c')
		size=sizeof(char);
	else
	if(mark[mark[0]=='_'?1:0]=='s')
		size=sizeof(short);
	else
	if(mark[mark[0]=='_'?1:0]=='f')
		size=sizeof(float);

	struct tmmpHashItem *item;
	item=(struct tmmpHashItem*)(head+sizeof(short)+sizeof(struct tmmpHeapItem))+fmmpHash(mark);
	if(item->mark==0)
	{
		result=fmmpHeapInit(head,strlen(mark)+1,&item->mark);
		if(result==-1)
			return -1;
		strcpy(head+item->mark,mark);

		if(mark[0]!='_')
		{
			item->size=size;

			result=fmmpHeapInit(head,item->size,&item->data);
			if(result==-1)
				return -1;
			*(char**)data=head+item->data;
		}
		else
		{
			item->room=(number/8+1)*8;

			result=fmmpHeapInit(head,sizeof(short)*item->room,&item->size);
			if(result==-1)
				return -1;
			bzero(head+item->size,sizeof(short)*item->room);
			*(short*)(head+item->size+sizeof(short)*number)=size;

			result=fmmpHeapInit(head,sizeof(short)*item->room,&item->data);
			if(result==-1)
				return -1;
			bzero(head+item->data,sizeof(short)*item->room);
			short temp;
			result=fmmpHeapInit(head,size,&temp);
			if(result==-1)
				return -1;
			*(short*)(head+item->data+sizeof(short)*number)=temp;
			*(char**)data=head+temp;
		}
		return 0;
	}

	while(1)
	{
		if(strcmp(head+item->mark,mark)==0)
		{
			if(mark[0]!='_')
			{
				item->size=size;

				result=fmmpHeapRnit(head,item->data,size,&item->data);
				if(result==-1)
					return -1;
				*(char**)data=head+item->data;
			}
			else
			{
				if(item->room<=number)
				{
					short roomrecord;
					roomrecord=item->room;
					item->room=(number/8+1)*8;

					short sizerecord;
					sizerecord=item->size;
					result=fmmpHeapRnit(head,item->size,sizeof(short)*item->room,&item->size);
					if(result==-1)
						return -1;
					bzero(head+item->size,sizeof(short)*item->room);
					memmove(head+item->size,head+sizerecord,sizeof(short)*roomrecord);

					short datarecord;
					datarecord=item->data;
					result=fmmpHeapRnit(head,item->data,sizeof(short)*item->room,&item->data);
					if(result==-1)
						return -1;
					bzero(head+item->data,sizeof(short)*item->room);
					memmove(head+item->data,head+datarecord,sizeof(short)*roomrecord);
				}

				*(short*)(head+item->size+sizeof(short)*number)=size;

				short temp;
				result=fmmpHeapInit(head,size,&temp);
				if(result==-1)
					return -1;
				*(short*)(head+item->data+sizeof(short)*number)=temp;
				*(char**)data=head+temp;
			}
			return 0;
		}
		if(item->next==0)
		{
			result=fmmpHeapInit(head,sizeof(struct tmmpHashItem),&item->next);
			if(result==-1)
				return -1;
			item=(struct tmmpHashItem*)(head+item->next);

			result=fmmpHeapInit(head,strlen(mark)+1,&item->mark);
			if(result==-1)
				return -1;
			strcpy(head+item->mark,mark);

			item->next=0;

			if(mark[0]!='_')
			{
				item->size=size;

				result=fmmpHeapInit(head,item->size,&item->data);
				if(result==-1)
					return -1;
				*(char**)data=head+item->data;
			}
			else
			{
				item->room=(number/8+1)*8;

				result=fmmpHeapInit(head,sizeof(short)*item->room,&item->size);
				if(result==-1)
					return -1;
				bzero(head+item->size,sizeof(short)*item->room);
				*(short*)(head+item->size+sizeof(short)*number)=size;

				result=fmmpHeapInit(head,sizeof(short)*item->room,&item->data);
				if(result==-1)
					return -1;
				bzero(head+item->data,sizeof(short)*item->room);
				short temp;
				result=fmmpHeapInit(head,size,&temp);
				if(result==-1)
					return -1;
				*(short*)(head+item->data+sizeof(short)*number)=temp;
				*(char**)data=head+temp;
			}
			return 0;
		}
		item=(struct tmmpHashItem*)(head+item->next);
	}
}

/*========================================*\
    功能 : 获取内存池引用
    参数 : (输入)内存池名称
           (输入)内存池序号
           (输出)内存池引用
           (输出)内存池尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fmmpRefGet(char *mark,int number,void *data,int *size)
{
	char *head;
	head=(char*)pthread_getspecific(vmmpHead);

	struct tmmpHashItem *item;
	item=(struct tmmpHashItem*)(head+sizeof(short)+sizeof(struct tmmpHeapItem))+fmmpHash(mark);
	if(item->mark==0)
		return -1;

	while(1)
	{
		if(strcmp(head+item->mark,mark)==0)
		{
			if(mark[0]!='_')
			{
				if(size!=NULL)
					if(mark[0]=='p')
						*size=item->size-1;
					else
						*size=item->size;
				*(char**)data=head+item->data;
			}
			else
			{
				if(*(short*)(head+item->size+sizeof(short)*number)==0)
					return -1;
				if(size!=NULL)
					if(mark[1]=='p')
						*size=*(short*)(head+item->size+sizeof(short)*number)-1;
					else
						*size=*(short*)(head+item->size+sizeof(short)*number);
				*(char**)data=head+*(short*)(head+item->data+sizeof(short)*number);
			}
			return 0;
		}
		if(item->next==0)
			return -1;
		item=(struct tmmpHashItem*)(head+item->next);
	}
}
