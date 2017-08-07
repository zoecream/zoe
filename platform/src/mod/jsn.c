/*========================================*\
    文件 : jsn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jsn.h>

#define cjsnStr 1
#define cjsnNum 2
#define cjsnArr 3
#define cjsnObj 4

#define mjsnSkip(data) *data+=strspn(*data,"\t\v\n\r\f ")

/*========================================*\
    功能 : 导出所有类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllExport(struct tjsnItem *item,char **dst);
/*========================================*\
    功能 : 导入所有类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllImport(struct tjsnItem *item,char **src);

/*========================================*\
    功能 : 数据结构节点分配
    参数 : (输出)数据结构节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnInit(struct tjsnItem **item)
{
	*item=(struct tjsnItem *)malloc(sizeof(struct tjsnItem));
	if(*item==NULL)
		return -1;
	bzero(*item,sizeof(struct tjsnItem));
	return 0;
}

/*========================================*\
    功能 : 数据结构节点释放
    参数 : (输入)数据结构节点
    返回 : 空
\*========================================*/
void fjsnFree(struct tjsnItem *item)
{
	if(item->chld)
		fjsnFree(item->chld);
	if(item->next)
		fjsnFree(item->next);
	if(item->vald)
		free(item->vald);
	free(item);
}

/*========================================*\
    功能 : 创建字符类型节点
    参数 : (输出)数据结构节点
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnStrCreate(struct tjsnItem **item,char *vald,int vall)
{
	int result;
	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cjsnStr;
	(*item)->vald=(char*)malloc(vall);
	if((*item)->vald==NULL)
		return -1;
	memcpy((*item)->vald,vald,vall);
	(*item)->vall=vall;
	return 0;
}

/*========================================*\
    功能 : 创建数值类型节点
    参数 : (输出)数据结构节点
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnNumCreate(struct tjsnItem **item,char *vald,int vall)
{
	int result;
	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cjsnNum;
	(*item)->vald=(char*)malloc(vall);
	if((*item)->vald==NULL)
		return -1;
	memcpy((*item)->vald,vald,vall);
	(*item)->vall=vall;
	return 0;
}

/*========================================*\
    功能 : 创建数组类型节点
    参数 : (输出)数据结构节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrCreate(struct tjsnItem **item)
{
	int result;
	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cjsnArr;
	return 0;
}

/*========================================*\
    功能 : 创建对象类型节点
    参数 : (输出)数据结构节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjCreate(struct tjsnItem **item)
{
	int result;
	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*item)->type=cjsnObj;
	return 0;
}

/*========================================*\
    功能 : 插入数组类型节点
    参数 : (输入)数组节点
           (输入)插入节点
    返回 : 空
\*========================================*/
void fjsnArrInsert(struct tjsnItem *arr,struct tjsnItem *item)
{
	struct tjsnItem *temp;
	temp=arr->chld;
	if(temp==NULL)
	{
		arr->chld=item;
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
    功能 : 插入对象类型节点
    参数 : (输入)对象节点
           (输入)插入节点
           (输入)键数据
           (输入)键长度
    返回 : 空
\*========================================*/
void fjsnObjInsert(struct tjsnItem *obj,struct tjsnItem *item,char *keyd,int keyl)
{
	item->keyd=keyd;
	item->keyl=keyl;
	struct tjsnItem *temp;
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
void fjsnKeyExport(struct tjsnItem *item,char **dst)
{
	sprintf(*dst,"\"%.*s\"",item->keyl,item->keyd);
	*dst+=item->keyl+2;
}

/*========================================*\
    功能 : 导出字符类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : 空
\*========================================*/
void fjsnStrExport(struct tjsnItem *item,char **dst)
{
	sprintf(*dst,"\"%.*s\"",item->vall,item->vald);
	*dst+=item->vall+2;
}

/*========================================*\
    功能 : 导出数值类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : 空
\*========================================*/
void fjsnNumExport(struct tjsnItem *item,char **dst)
{
	memcpy(*dst,item->vald,item->vall);
	*dst+=item->vall;
}

/*========================================*\
    功能 : 导出数组类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrExport(struct tjsnItem *item,char **dst)
{
	int result;
	struct tjsnItem *current;
	current=item->chld;
	*(*dst)++='[';
	while(1)
	{
		if(current==NULL)
			break;
		result=fjsnAllExport(current,dst);
		if(result!=0)
			return -1;
		current=current->next;
		if(current!=NULL)
			*(*dst)++=',';
	}
	*(*dst)++=']';
	return 0;
}

/*========================================*\
    功能 : 导出对象类型节点
    参数 : (输入)数据结构节点
           (输出)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjExport(struct tjsnItem *item,char **dst)
{
	int result;

	struct tjsnItem *current;
	current=item->chld;
	*(*dst)++='{';
	while(1)
	{
		if(current==NULL)
			break;
		fjsnKeyExport(current,dst);
		*(*dst)++=':';
		result=fjsnAllExport(current,dst);
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
int fjsnAllExport(struct tjsnItem *item,char **dst)
{
	int result;

	switch(item->type)
	{
		case cjsnArr:
		result=fjsnArrExport(item,dst);
		if(result!=0)
			return -1;
		break;

		case cjsnObj:
		result=fjsnObjExport(item,dst);
		if(result!=0)
			return -1;
		break;

		case cjsnStr:
		fjsnStrExport(item,dst);
		break;

		case cjsnNum:
		fjsnNumExport(item,dst);
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
void fjsnKeyImport(struct tjsnItem *item,char **src)
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
    功能 : 导入字符类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnStrImport(struct tjsnItem *item,char **src)
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
    功能 : 导入数值类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnNumImport(struct tjsnItem *item,char **src)
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
    功能 : 导入数组类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrImport(struct tjsnItem *item,char **src)
{
	int result;

	(*src)++;
	while(1)
	{
		mjsnSkip(src);
		if(**src==']')
			break;
		struct tjsnItem *current;
		result=fjsnInit(&current);
		if(result!=0)
			return -1;
		current->next=item->chld;
		item->chld=current;
		mjsnSkip(src);
		result=fjsnAllImport(current,src);
		if(result!=0)
			return -1;
		mjsnSkip(src);
		if(**src!=','&&**src!=']')
			return -1;
		if(**src==',')
			(*src)++;
	}
	(*src)++;

	return 0;
}

/*========================================*\
    功能 : 导入对象类型节点
    参数 : (输入)数据结构节点
           (出入)起始结束位置
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjImport(struct tjsnItem *item,char **src)
{
	int result;

	(*src)++;
	while(1)
	{
		mjsnSkip(src);
		if(**src=='}')
			break;
		struct tjsnItem *current;
		result=fjsnInit(&current);
		if(result!=0)
			return -1;
		current->next=item->chld;
		item->chld=current;
		mjsnSkip(src);
		fjsnKeyImport(current,src);
		mjsnSkip(src);
		if(*(*src)++!=':')
			return -1;
		mjsnSkip(src);
		result=fjsnAllImport(current,src);
		if(result!=0)
			return -1;
		mjsnSkip(src);
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
int fjsnAllImport(struct tjsnItem *item,char **src)
{
	int result;

	switch(**src)
	{
		case '[':
		item->type=cjsnArr;
		result=fjsnArrImport(item,src);
		if(result!=0)
			return -1;
		break;

		case '{':
		item->type=cjsnObj;
		result=fjsnObjImport(item,src);
		if(result!=0)
			return -1;
		break;

		case '\"':
		item->type=cjsnStr;
		result=fjsnStrImport(item,src);
		if(result!=0)
			return -1;
		break;

		default:
		item->type=cjsnNum;
		result=fjsnNumImport(item,src);
		if(result!=0)
			return -1;
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 查询数组类型节点
    参数 : (输入)数组节点
           (输出)查询节点
           (输入)序
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrSelect(struct tjsnItem *arr,struct tjsnItem **item,int ind)
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
    功能 : 查询对象类型节点
    参数 : (输入)对象节点
           (输出)查询节点
           (输入)键数据
           (输入)键长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjSelect(struct tjsnItem *obj,struct tjsnItem **item,char *keyd,int keyl)
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
int fjsnExport(struct tjsnItem *item,char **pd,int *pl)
{
	char *record;
	record=*pd;
	int result;
	result=fjsnAllExport(item,pd);
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
int fjsnImport(struct tjsnItem *item,char **pd,int pl)
{
	char *record;
	record=*pd;
	int result;
	result=fjsnAllImport(item,pd);
	if(result!=0)
		return -1;
	if(*pd-record!=pl)
		return -1;
	return 0;
}
