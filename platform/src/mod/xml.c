/*========================================*\
    文件 : xml.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xml.h>

#define cxmlStr 1
#define cxmlObj 2

#define mxmlSkip(data) *data+=strspn(*data,"\t\v\n\r\f ")

/*========================================*\
    功能 : 导出所有类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAllExport(struct txmlItem *item,char **dst);
/*========================================*\
    功能 : 导入所有类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAllImport(struct txmlItem *item,char **src);

/*========================================*\
    功能 : 数据结构节点分配
    参数 : (输出)数据结构节点
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
    功能 : 数据结构节点释放
    参数 : (输入)数据结构节点
    返回 : 空
\*========================================*/
void fxmlFree(struct txmlItem *item)
{
	if(item->chld)
		fxmlFree(item->chld);
	if(item->next)
		fxmlFree(item->next);
	if(item->vald)
		free(item->vald);
	free(item);
}

/*========================================*\
    功能 : 创建内容类型节点
    参数 : (输出)数据结构节点
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlValCreate(struct txmlItem **item,char *vald,int vall)
{
	int result;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cxmlVal;
	(*item)->vald=(char*)malloc(vall);
	if((*item)->vald==NULL)
		return -1;
	memcpy((*item)->vald,vald,vall);
	(*item)->vall=vall;
	return 0;
}

/*========================================*\
    功能 : 创建结构类型节点
    参数 : (输出)数据结构节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlStcCreate(struct txmlItem **item)
{
	int result;
	result=fxmlInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cxmlStc;
	return 0;
}

/*========================================*\
    功能 : 插入OBJ类型节点
    参数 : (输入)对象节点
           (输入)插入节点
           (输入)键数据
           (输入)键长度
    返回 : 空
\*========================================*/
void fxmlObjInsert(struct txmlItem *obj,struct txmlItem *item,char *keyd,int keyl)
{
	item->keyd=keyd;
	item->keyl=keyl;
	struct txmlItem *temp;
	temp=obj->chld;
	if(temp==NULL)
	{
		obj->chld=item;
		return;
	}
	while(1)
	{
		if(temp->next==NULL)
			break;
		temp=temp->next;
	}
	temp->next=item;
}

/*========================================*\
    功能 : 导出KEY类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : 空
\*========================================*/
void fxmlKeyExport(struct txmlItem *item,char **dst)
{
	sprintf(*dst,"\"%.*s\"",item->keyl,item->keyd);
	*dst+=item->keyl+2;
}

/*========================================*\
    功能 : 导出STR类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : 空
\*========================================*/
void fxmlStrExport(struct txmlItem *item,char **dst)
{
	sprintf(*dst,"\"%.*s\"",item->vall,item->vald);
	*dst+=item->vall+2;
}

/*========================================*\
    功能 : 导出OBJ类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlObjExport(struct txmlItem *item,char **dst)
{
	int result;

	struct txmlItem *current;
	current=item->chld;
	*(*dst)++='{';
	while(1)
	{
		if(current==NULL)
			break;
		fxmlKeyExport(current,dst);
		*(*dst)++=':';
		result=fxmlAllExport(current,dst);
		if(result!=0)
			return -1;
		current=current->next;
		if(current!=NULL)
			*(*dst)++=',';
	}
	*(*dst)++='}';

	return 0;
}

/*========================================*\
    功能 : 导出所有类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAllExport(struct txmlItem *item,char **dst)
{
	int result;

	switch(item->type)
	{
		case cxmlArr:
		result=fxmlArrExport(item,dst);
		if(result!=0)
			return -1;
		break;

		case cxmlObj:
		result=fxmlObjExport(item,dst);
		if(result!=0)
			return -1;
		break;

		case cxmlStr:
		fxmlStrExport(item,dst);
		break;

		case cxmlNum:
		fxmlNumExport(item,dst);
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 导入KEY类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : 空
\*========================================*/
void fxmlKeyImport(struct txmlItem *item,char **src)
{
	item->keyd=*src+1;

	(*src)++;
	while(1)
	{
		int escape;
		if(**src=='\\')
			escape=1;
		else
			escape=0;

		if(**src=='\"'&&escape==0)
			break;

		(*src)++;

		item->keyl++;
	}
	(*src)++;
}
/*========================================*\
    功能 : 导入STR类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlStrImport(struct txmlItem *item,char **src)
{
	char *record;
	record=*src+1;

	(*src)++;
	while(1)
	{
		int escape;
		if(**src=='\\')
			escape=1;
		else
			escape=0;

		if(**src=='\"'&&escape==0)
			break;

		(*src)++;

		item->vall++;
	}
	(*src)++;

	item->vald=(char*)malloc(item->vall);
	if(item->vald==NULL)
		return -1;
	memcpy(item->vald,record,item->vall);

	return 0;
}

/*========================================*\
    功能 : 导入NUM类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlNumImport(struct txmlItem *item,char **src)
{
	char *record;
	record=*src;
	strtod(*src,src);
	item->vall=*src-record;
	item->vald=(char*)malloc(item->vall);
	if(item->vald==NULL)
		return -1;
	memcpy(item->vald,record,item->vall);
	return 0;
}

/*========================================*\
    功能 : 导入ARR类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlArrImport(struct txmlItem *item,char **src)
{
	int result;

	(*src)++;
	while(1)
	{
		mxmlSkip(src);
		if(**src==']')
			break;
		struct txmlItem *current;
		result=fxmlInit(&current);
		if(result!=0)
			return -1;
		current->next=item->chld;
		item->chld=current;
		mxmlSkip(src);
		result=fxmlAllImport(current,src);
		if(result!=0)
			return -1;
		mxmlSkip(src);
		if(**src!=','&&**src!=']')
			return -1;
		if(**src==',')
			(*src)++;
	}
	(*src)++;

	return 0;
}

/*========================================*\
    功能 : 导入OBJ类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlObjImport(struct txmlItem *item,char **src)
{
	int result;

	(*src)++;
	while(1)
	{
		mxmlSkip(src);
		if(**src=='}')
			break;
		struct txmlItem *current;
		result=fxmlInit(&current);
		if(result!=0)
			return -1;
		current->next=item->chld;
		item->chld=current;
		mxmlSkip(src);
		fxmlKeyImport(current,src);
		mxmlSkip(src);
		if(*(*src)++!=':')
			return -1;
		mxmlSkip(src);
		result=fxmlAllImport(current,src);
		if(result!=0)
			return -1;
		mxmlSkip(src);
		if(**src!=','&&**src!='}')
			return -1;
		if(**src==',')
			(*src)++;
	}
	(*src)++;

	return 0;
}

/*========================================*\
    功能 : 导入所有类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlAllImport(struct txmlItem *item,char **src)
{
	int result;

	switch(**src)
	{
		case '[':
		item->type=cxmlArr;
		result=fxmlArrImport(item,src);
		if(result!=0)
			return -1;
		break;

		case '{':
		item->type=cxmlObj;
		result=fxmlObjImport(item,src);
		if(result!=0)
			return -1;
		break;

		case '\"':
		item->type=cxmlStr;
		result=fxmlStrImport(item,src);
		if(result!=0)
			return -1;
		break;

		default:
		item->type=cxmlNum;
		result=fxmlNumImport(item,src);
		if(result!=0)
			return -1;
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 查询OBJ类型节点
    参数 : (输入)数组节点
           (输出)查询节点
           (输入)序
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlArrSelect(struct txmlItem *arr,struct txmlItem **item,int ind)
{
	*item=arr->chld;
	while(1)
	{
		if(*item==NULL)
			return -1;
		if(ind==0)
			return 0;
		*item=(*item)->next;
		ind--;
	}
}

/*========================================*\
    功能 : 查询OBJ类型节点
    参数 : (输入)对象节点
           (输出)查询节点
           (输入)键数据
           (输入)键长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlObjSelect(struct txmlItem *obj,struct txmlItem **item,char *keyd,int keyl)
{
	*item=obj->chld;
	while(1)
	{
		if(*item==NULL)
			return -1;
		if(strncmp((*item)->keyd,keyd,keyl)==0&&(*item)->keyl==keyl)
			return 0;
		*item=(*item)->next;
	}
}

/*========================================*\
    功能 : 将数据结构导出到报文
    参数 : (输入)数据结构节点
           (输出)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char **pd,int *pl)
{
	char *record;
	record=*pd;
	int result;
	result=fxmlAllExport(item,pd);
	if(result!=0)
		return -1;
	*pl=*pd-record;
	return 0;
}

/*========================================*\
    功能 : 将报文导入到数据结构
    参数 : (输出)数据结构节点
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem *item,char **pd,int pl)
{
	char *record;
	record=*pd;
	int result;
	result=fxmlAllImport(item,pd);
	if(result!=0)
		return -1;
	if(*pd-record!=pl)
		return -1;
	return 0;
}
