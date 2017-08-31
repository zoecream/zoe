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
#define mxmlSize(data,reject) strcspn(*data,reject)

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
    参数 : (输入)节点
    返回 : 空
\*========================================*/
void fxmlFree(struct txmlItem *item)
{
	if(item->nodechld)
		fxmlFree(item->nodechld);
	if(item->attrchld)
		fxmlFree(item->attrchld);
	if(item->next)
		fxmlFree(item->next);
	if(item->keydata)
		free(item->keydata);
	if(item->valdata)
		free(item->valdata);
	free(item);
}

/*========================================*\
    功能 : 创建节点
    参数 : (输出)节点
           (输入)类型
           (输入)键数据
           (输入)键长度
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlCreate(struct txmlItem **item,int type,char *keydata,int keysize,char *valdata,int valsize)
{
	int result;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->type=type;
	if(keydata!=NULL&&keysize!=0)
	{
		(*item)->keydata=(char*)malloc(keysize);
		if((*item)->keydata==NULL)
			return -1;
		memcpy((*item)->keydata,keydata,keysize);
		(*item)->keysize=keysize;
	}
	if(valdata!=NULL&&valsize!=0)
	{
		(*item)->valdata=(char*)malloc(valsize);
		if((*item)->valdata==NULL)
			return -1;
		memcpy((*item)->valdata,valdata,valsize);
		(*item)->valsize=valsize;
	}
	return 0;
}

/*========================================*\
    功能 : 插入节点
    参数 : (输入)目标节点
           (输入)插入节点
           (输入)类型
    返回 : 空
\*========================================*/
void fxmlInsert(struct txmlItem *target,struct txmlItem *insert,int type)
{
	struct txmlItem *temp;
	if(type==cxmlNode)
		temp=target->nodechld;
	else
	if(type==cxmlAttr)
		temp=target->attrchld;
	if(temp==NULL)
	{
		if(type==cxmlNode)
			target->nodechld=insert;
		else
		if(type==cxmlAttr)
			target->attrchld=insert;
		return;
	}
	while(1)
	{
		if(temp->next==NULL)
			break;
		temp=temp->next;
	}
	temp->next=insert;
}

/*========================================*\
    功能 : 查询节点
    参数 : (输入)目标节点
           (输出)查询节点
           (输入)类型
           (输入)键数据
           (输入)键长度
           (输入)序号
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlSelect(struct txmlItem *target,struct txmlItem **select,int type,char *keydata,int keysize,int index)
{
	if(type==cxmlNode)
		*select=target->nodechld;
	else
	if(type==cxmlAttr)
		*select=target->attrchld;
	while(1)
	{
		if(*select==NULL)
			return -1;
		if(strncmp((*select)->keydata,keydata,keysize)==0&&(*select)->keysize==keysize)
			if(index==0)
				return 0;
			else
				index--;
		*select=(*select)->next;
	}
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
	temp=item->attrchld;
	while(1)
	{
		if(temp==NULL)
			break;
		*(*data)++=' ';
		fxmlAttrExport(temp,data);
		temp=temp->next;
	}
	*(*data)++='>';
	*data+=sprintf(*data,"%.*s",item->valsize,item->valdata);
	temp=item->nodechld;
	while(1)
	{
		if(temp==NULL)
			break;
		result=fxmlNodeExport(temp,data);
		if(result!=0)
			return -1;
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
	(*item)->keysize=mxmlSize(data,cxmlSpace"=");
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
	(*item)->valsize=mxmlSize(data,"\"");
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
	(*item)->keysize=mxmlSize(data,cxmlSpace">");
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
		struct txmlItem *temp;
		result=fxmlAttrImport(&temp,data);
		if(result!=0)
			return -1;
		fxmlInsert((*item),temp,cxmlAttr);
	}
	(*data)++;
	while(1)
	{
		mxmlSkip(data,cxmlSpace);
		if(**data=='<'&&*(*data+1)=='/')
			break;
		if(**data!='<')
		{
			(*item)->valsize=mxmlSize(data,"<");
			(*item)->valdata=(char*)malloc((*item)->valsize);
			if((*item)->valdata==NULL)
				return -1;
			memcpy((*item)->valdata,*data,(*item)->valsize);
			*data+=(*item)->valsize;
			continue;
		}
		struct txmlItem *temp;
		result=fxmlNodeImport(&temp,data);
		if(result!=0)
			return -1;
		fxmlInsert((*item),temp,cxmlNode);
	}
	(*data)+=(*item)->keysize;
	(*data)+=3;

	return 0;
}

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char *data)
{
	char *temp;
	temp=data;
	int result;
	result=fxmlNodeExport(item,&temp);
	if(result!=0)
		return -1;
	return 0;
}

/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (输入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem **item,char *data)
{
	char *temp;
	temp=data;
	int result;
	result=fxmlNodeImport(item,&temp);
	if(result!=0)
		return -1;
	return 0;
}
