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
	if(item->keyd)
		free(item->keyd);
	if(item->vald)
		free(item->vald);
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
int fxmlCreate(struct txmlItem **item,int type,char *keyd,int keyl,char *vald,int vall)
{
	int result;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->type=type;
	if(keyd!=NULL&&keyl!=0)
	{
		(*item)->keyd=(char*)malloc(keyl);
		if((*item)->keyd==NULL)
			return -1;
		memcpy((*item)->keyd,keyd,keyl);
		(*item)->keyl=keyl;
	}
	if(vald!=NULL&&vall!=0)
	{
		(*item)->vald=(char*)malloc(vall);
		if((*item)->vald==NULL)
			return -1;
		memcpy((*item)->vald,vald,vall);
		(*item)->vall=vall;
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
int fxmlSelect(struct txmlItem *target,struct txmlItem **select,int type,char *keyd,int keyl,int index)
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
		if(strncmp((*select)->keyd,keyd,keyl)==0&&(*select)->keyl==keyl)
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
           (出入)位置
    返回 : 空
\*========================================*/
void fxmlAttrExport(struct txmlItem *item,char **dst)
{
	*dst+=sprintf(*dst,"%.*s=\"%.*s\"",item->keyl,item->keyd,item->vall,item->vald);
}

/*========================================*\
    功能 : 导出NODE类型节点
    参数 : (输入)节点
           (出入)位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlNodeExport(struct txmlItem *item,char **dst)
{
	int result;
	*(*dst)++='<';
	*dst+=sprintf(*dst,"%.*s",item->keyl,item->keyd);
	struct txmlItem *current;
	current=item->attrchld;
	while(1)
	{
		if(current==NULL)
			break;
		*(*dst)++=' ';
		fxmlAttrExport(current,dst);
		current=current->next;
	}
	*(*dst)++='>';
	*dst+=sprintf(*dst,"%.*s",item->vall,item->vald);
	current=item->nodechld;
	while(1)
	{
		if(current==NULL)
			break;
		result=fxmlNodeExport(current,dst);
		if(result!=0)
			return -1;
		current=current->next;
	}
	*(*dst)++='<';
	*(*dst)++='/';
	*dst+=sprintf(*dst,"%.*s",item->keyl,item->keyd);
	*(*dst)++='>';
	return 0;
}

/*========================================*\
    功能 : 导入ATTR类型节点
    参数 : (输出)节点
           (出入)位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAttrImport(struct txmlItem **item,char **src)
{
	int result;

	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->keyl=mxmlSize(src,cxmlSpace"=");
	(*item)->keyd=(char*)malloc((*item)->keyl);
	if((*item)->keyd==NULL)
		return -1;
	memcpy((*item)->keyd,*src,(*item)->keyl);
	*src+=(*item)->keyl;
	mxmlSkip(src,cxmlSpace);
	if(*(*src)++!='=')
		return -1;
	mxmlSkip(src,cxmlSpace);
	if(*(*src)++!='\"')
		return -1;
	(*item)->vall=mxmlSize(src,"\"");
	(*item)->vald=(char*)malloc((*item)->vall);
	if((*item)->vald==NULL)
		return -1;
	memcpy((*item)->vald,*src,(*item)->vall);
	*src+=(*item)->vall;
	(*src)++;

	return 0;
}

/*========================================*\
    功能 : 导入NODE类型节点
    参数 : (输出)节点
           (出入)位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlNodeImport(struct txmlItem **item,char **src)
{
	int result;

	if(*(*src)++!='<')
		return -1;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->keyl=mxmlSize(src,cxmlSpace">");
	(*item)->keyd=(char*)malloc((*item)->keyl);
	if((*item)->keyd==NULL)
		return -1;
	memcpy((*item)->keyd,*src,(*item)->keyl);
	*src+=(*item)->keyl;
	while(1)
	{
		mxmlSkip(src,cxmlSpace);
		if(**src=='>')
			break;
		struct txmlItem *temp;
		result=fxmlAttrImport(&temp,src);
		if(result!=0)
			return -1;
		fxmlInsert((*item),temp,cxmlAttr);
	}
	(*src)++;
	while(1)
	{
		mxmlSkip(src,cxmlSpace);
		if(**src=='<'&&*(*src+1)=='/')
			break;
		if(**src!='<')
		{
			(*item)->vall=mxmlSize(src,"<");
			(*item)->vald=(char*)malloc((*item)->vall);
			if((*item)->vald==NULL)
				return -1;
			memcpy((*item)->vald,*src,(*item)->vall);
			*src+=(*item)->vall;
			continue;
		}
		struct txmlItem *temp;
		result=fxmlNodeImport(&temp,src);
		if(result!=0)
			return -1;
		fxmlInsert((*item),temp,cxmlNode);
	}
	(*src)+=(*item)->keyl;
	(*src)+=3;

	return 0;
}

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (出入)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char **pd,int *pl)
{
	char *record;
	record=*pd;
	int result;
	result=fxmlNodeExport(item,pd);
	if(result!=0)
		return -1;
	*pl=*pd-record;
	return 0;
}

/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem **item,char **pd,int pl)
{
	char *record;
	record=*pd;
	mxmlSkip(pd,cxmlSpace);
	int result;
	result=fxmlNodeImport(item,pd);
	if(result!=0)
		return -1;
	mxmlSkip(pd,cxmlSpace);
	if(*pd-record!=pl)
		return -1;
	return 0;
}
