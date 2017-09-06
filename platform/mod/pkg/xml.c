/*========================================*\
    文件 : xml.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xml.h>

#define cxmlSpace "\t\v\n\r\f "
#define mxmlSkip(data,accept) *data+=strspn(*data,accept)
#define mxmlSize(data,reject) strcspn(data,reject)

/*========================================*\
    功能 : 节点分配
    参数 : (输出)节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlInit(struct txmlItem **item)
{
	*item=(struct txmlItem *)malloc(sizeof(struct txmlItem));
	if(*item==NULL)
		return -1;
	bzero(*item,sizeof(struct txmlItem));
	return 0;
}

/*========================================*\
    功能 : 节点释放
    参数 : (出入)节点
    返回 : 空
\*========================================*/
void fxmlFree(struct txmlItem **item)
{
	if((*item)->chld)
		fxmlFree(&(*item)->chld);
	if((*item)->next)
		fxmlFree(&(*item)->next);
	if((*item)->keydata)
		free((*item)->keydata);
	if((*item)->valdata)
		free((*item)->valdata);
	free((*item));
	*item=NULL;
}

/*========================================*\
    功能 : 创建节点
    参数 : (输出)节点
           (输入)路径
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlCreate(struct txmlItem **item,char *path,char *data,int size)
{
	int result;
	if(*path!='/')
		return -1;
	if(data==NULL||size==0)
		return -1;
	if(*item==NULL)
	{
		result=fxmlInit(item);
		if(result!=0)
			return -1;
		(*item)->type=cxmlNode;
		(*item)->keysize=mxmlSize(path+1,"/#");
		(*item)->keydata=(char*)malloc((*item)->keysize);
		if((*item)->keydata==NULL)
			return -1;
		memcpy((*item)->keydata,path+1,(*item)->keysize);
	}
	else
	{
		if((*item)->keysize!=mxmlSize(path+1,"/#"))
			return -1;
		if(memcmp((*item)->keydata,path+1,(*item)->keysize)!=0)
			return -1;
	}
	struct txmlItem **temp;
	temp=item;
	path+=(*item)->keysize+1;
	while(1)
	{
		if(*path=='\0')
			break;
		if(*path!='/'&&*path!='#')
			return -1;
		item=&(*item)->chld;
		int index;
		if(*(path+1+mxmlSize(path+1,"/#:"))==':')
			index=atoi(path+1+mxmlSize(path+1,"/#:")+1);
		else
			index=0;
		while(1)
		{
			if(*item==NULL)
			{
				if(index!=0)
					return -1;
				result=fxmlInit(item);
				if(result!=0)
					return -1;
				if(*path=='/')
					(*item)->type=cxmlNode;
				else
				if(*path=='#')
					(*item)->type=cxmlAttr;
				(*item)->keysize=mxmlSize(path+1,"/#:");
				(*item)->keydata=(char*)malloc((*item)->keysize);
				if((*item)->keydata==NULL)
					return -1;
				memcpy((*item)->keydata,path+1,(*item)->keysize);
				break;
			}
			if(*path=='/'&&(*item)->type!=cxmlNode)
				goto next;
			if(*path=='#'&&(*item)->type!=cxmlAttr)
				goto next;
			if((*item)->keysize!=mxmlSize(path+1,"/#:"))
				goto next;
			if(memcmp((*item)->keydata,path+1,(*item)->keysize)!=0)
				goto next;
			if(index--!=0)
				goto next;
			break;
			next:
			item=&(*item)->next;
		}
		path+=mxmlSize(path+1,"/#:")+1;
		if(*path==':')
			path+=mxmlSize(path+1,"/#:")+1;
	}
	if((*item)->chld!=NULL)
		return -1;
	if((*item)->valdata!=NULL||(*item)->valsize!=0)
		return -1;
	(*item)->valsize=size;
	(*item)->valdata=(char*)malloc((*item)->valsize);
	if((*item)->valdata==NULL)
		return -1;
	memcpy((*item)->valdata,data,(*item)->valsize);
	item=temp;
	return 0;
}

/*========================================*\
    功能 : 查询节点
    参数 : (输入)节点
           (输入)路径
           (输出)值数据
           (输出)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlSelect(struct txmlItem *item,char *path,char *data,int *size)
{
	int result;
	if(*path!='/')
		return -1;
	if(item==NULL)
		return -1;
	if(item->keysize!=mxmlSize(path+1,"/#"))
		return -1;
	if(memcmp(item->keydata,path+1,item->keysize)!=0)
		return -1;
	path+=item->keysize+1;
	while(1)
	{
		if(*path=='\0')
			break;
		if(*path!='/'&&*path!='#')
			return -1;
		item=item->chld;
		int index;
		if(*(path+1+mxmlSize(path+1,"/#:"))==':')
			index=atoi(path+1+mxmlSize(path+1,"/#:")+1);
		else
			index=0;
		while(1)
		{
			if(item==NULL)
				return -1;
			if(*path=='/'&&item->type!=cxmlNode)
				goto next;
			if(*path=='#'&&item->type!=cxmlAttr)
				goto next;
			if(item->keysize!=mxmlSize(path+1,"/#:"))
				goto next;
			if(memcmp(item->keydata,path+1,item->keysize)!=0)
				goto next;
			if(index--!=0)
				goto next;
			break;
			next:
			item=item->next;
		}
		path+=mxmlSize(path+1,"/#:")+1;
		if(*path==':')
			path+=mxmlSize(path+1,"/#:")+1;
	}
	*size=item->valsize;
	memcpy(data,item->valdata,*size);
	data[*size]='\0';
	return 0;
}

/*========================================*\
    功能 : 导出ATTR类型节点
    参数 : (输入)节点
           (出入)报文
    返回 : 空
\*========================================*/
void fxmlAttrExport(struct txmlItem *item,char **data)
{
	*data+=sprintf(*data,"%.*s=\"%.*s\"",item->keysize,item->keydata,item->valsize,item->valdata);
}

/*========================================*\
    功能 : 导出NODE类型节点
    参数 : (输入)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlNodeExport(struct txmlItem *item,char **data)
{
	int result;

	*(*data)++='<';
	*data+=sprintf(*data,"%.*s",item->keysize,item->keydata);
	struct txmlItem *temp;
	temp=item->chld;
	while(1)
	{
		if(temp==NULL)
			break;
		if(temp->type==cxmlAttr)
		{
			*(*data)++=' ';
			fxmlAttrExport(temp,data);
		}
		temp=temp->next;
	}
	*(*data)++='>';
	*data+=sprintf(*data,"%.*s",item->valsize,item->valdata);
	temp=item->chld;
	while(1)
	{
		if(temp==NULL)
			break;
		if(temp->type==cxmlNode)
		{
			result=fxmlNodeExport(temp,data);
			if(result!=0)
				return -1;
		}
		temp=temp->next;
	}
	*(*data)++='<';
	*(*data)++='/';
	*data+=sprintf(*data,"%.*s",item->keysize,item->keydata);
	*(*data)++='>';
	**data='\0';

	return 0;
}

/*========================================*\
    功能 : 导入ATTR类型节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAttrImport(struct txmlItem **item,char **data)
{
	int result;

	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->keysize=mxmlSize(*data,cxmlSpace"=");
	(*item)->keydata=(char*)malloc((*item)->keysize);
	if((*item)->keydata==NULL)
		return -1;
	memcpy((*item)->keydata,*data,(*item)->keysize);
	*data+=(*item)->keysize;
	mxmlSkip(data,cxmlSpace);
	if(*(*data)++!='=')
		return -1;
	mxmlSkip(data,cxmlSpace);
	if(*(*data)++!='\"')
		return -1;
	(*item)->valsize=mxmlSize(*data,"\"");
	(*item)->valdata=(char*)malloc((*item)->valsize);
	if((*item)->valdata==NULL)
		return -1;
	memcpy((*item)->valdata,*data,(*item)->valsize);
	*data+=(*item)->valsize;
	(*data)++;

	return 0;
}

/*========================================*\
    功能 : 导入NODE类型节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlNodeImport(struct txmlItem **item,char **data)
{
	int result;

	if(*(*data)++!='<')
		return -1;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->keysize=mxmlSize(*data,cxmlSpace">");
	(*item)->keydata=(char*)malloc((*item)->keysize);
	if((*item)->keydata==NULL)
		return -1;
	memcpy((*item)->keydata,*data,(*item)->keysize);
	*data+=(*item)->keysize;
	while(1)
	{
		mxmlSkip(data,cxmlSpace);
		if(**data=='>')
			break;
		struct txmlItem **temp;
		temp=&(*item)->chld;
		if(*temp!=NULL)
		{
			while(1)
			{
				temp=&(*temp)->next;
				if(*temp==NULL)
					break;
			}
		}
		result=fxmlAttrImport(temp,data);
		if(result!=0)
			return -1;
		(*temp)->type=cxmlAttr;
	}
	(*data)++;
	while(1)
	{
		mxmlSkip(data,cxmlSpace);
		if(**data=='<'&&*(*data+1)=='/')
			break;
		if(**data!='<')
		{
			(*item)->valsize=mxmlSize(*data,"<");
			(*item)->valdata=(char*)malloc((*item)->valsize);
			if((*item)->valdata==NULL)
				return -1;
			memcpy((*item)->valdata,*data,(*item)->valsize);
			*data+=(*item)->valsize;
			continue;
		}
		struct txmlItem **temp;
		temp=&(*item)->chld;
		if(*temp!=NULL)
		{
			while(1)
			{
				temp=&(*temp)->next;
				if(*temp==NULL)
					break;
			}
		}
		result=fxmlNodeImport(temp,data);
		if(result!=0)
			return -1;
		(*temp)->type=cxmlNode;
	}
	if(*(*data)++!='<')
		return -1;
	if(*(*data)++!='/')
		return -1;
	if(memcmp((*item)->keydata,*data,(*item)->keysize)!=0)
		return -1;
	(*data)+=(*item)->keysize;
	if(*(*data)++!='>')
		return -1;

	return 0;
}

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char *data,int *size)
{
	char *temp;
	temp=data;
	int result;
	result=fxmlNodeExport(item,&temp);
	if(result!=0)
		return -1;
	*size=temp-data;
	return 0;
}

/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (输入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem **item,char *data,int size)
{
	char *temp;
	temp=data;
	int result;
	result=fxmlNodeImport(item,&temp);
	if(result!=0)
		return -1;
	return 0;
}
