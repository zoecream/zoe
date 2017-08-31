/*========================================*\
    文件 : jsn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jsn.h>

#define cjsnSpace "\t\v\n\r\f "
#define mjsnSkip(data,accept) *data+=strspn(*data,accept)
#define mjsnSize(data,reject) strcspn(*data,reject)

/*========================================*\
    功能 : 导出节点
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllExport(struct tjsnItem *item,char **data);
/*========================================*\
    功能 : 导入节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllImport(struct tjsnItem **item,char **data);

/*========================================*\
    功能 : 节点分配
    参数 : (输出)节点
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
    功能 : 节点释放
    参数 : (输入)节点
    返回 : 空
\*========================================*/
void fjsnFree(struct tjsnItem *item)
{
	if(item->chld)
		fjsnFree(item->chld);
	if(item->next)
		fjsnFree(item->next);
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
int fjsnCreate(struct tjsnItem **item,int type,char *keydata,int keysize,char *valdata,int valsize)
{
	int result;
	result=fjsnInit(item);
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
    返回 : 空
\*========================================*/
void fjsnInsert(struct tjsnItem *target,struct tjsnItem *insert)
{
	struct tjsnItem *temp;
	temp=target->chld;
	if(temp==NULL)
	{
		target->chld=insert;
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
           (输入)键数据
           (输入)键长度
           (输入)序号
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnSelect(struct tjsnItem *target,struct tjsnItem **select,char *keydata,int keysize,int index)
{
	*select=target->chld;
	while(1)
	{
		if(*select==NULL)
			return -1;
		if(target->type==cjsnArr)
		{
			if(index==0)
				return 0;
			else
				index--;
		}
		else
		if(target->type==cjsnObj)
		{
			if(strncmp((*select)->keydata,keydata,keysize)==0&&(*select)->keysize==keysize)
				return 0;
		}
		*select=(*select)->next;
	}
}

/*========================================*\
    功能 : 导出STR类型节点
    参数 : (输入)节点
           (输出)报文
    返回 : 空
\*========================================*/
void fjsnStrExport(struct tjsnItem *item,char **data)
{
	*data+=sprintf(*data,"\"%.*s\"",item->valsize,item->valdata);
}

/*========================================*\
    功能 : 导出NUM类型节点
    参数 : (输入)节点
           (输出)报文
    返回 : 空
\*========================================*/
void fjsnNumExport(struct tjsnItem *item,char **data)
{
	*data+=sprintf(*data,"%.*s",item->valsize,item->valdata);
}

/*========================================*\
    功能 : 导出ARR类型节点
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrExport(struct tjsnItem *item,char **data)
{
	int result;
	*(*data)++='[';
	struct tjsnItem *temp;
	temp=item->chld;
	while(1)
	{
		if(temp==NULL)
			break;
		result=fjsnAllExport(temp,data);
		if(result!=0)
			return -1;
		temp=temp->next;
		if(temp!=NULL)
			*(*data)++=',';
	}
	*(*data)++=']';
	return 0;
}

/*========================================*\
    功能 : 导出OBJ类型节点
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjExport(struct tjsnItem *item,char **data)
{
	int result;

	*(*data)++='{';
	struct tjsnItem *temp;
	temp=item->chld;
	while(1)
	{
		if(temp==NULL)
			break;
		*data+=sprintf(*data,"\"%.*s\"",temp->keysize,temp->keydata);
		*(*data)++=':';
		result=fjsnAllExport(temp,data);
		if(result!=0)
			return -1;
		temp=temp->next;
		if(temp!=NULL)
			*(*data)++=',';
	}
	*(*data)++='}';

	return 0;
}

/*========================================*\
    功能 : 导出节点
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllExport(struct tjsnItem *item,char **data)
{
	int result;

	switch(item->type)
	{
		case cjsnArr:
		result=fjsnArrExport(item,data);
		if(result!=0)
			return -1;
		break;

		case cjsnObj:
		result=fjsnObjExport(item,data);
		if(result!=0)
			return -1;
		break;

		case cjsnStr:
		fjsnStrExport(item,data);
		break;

		case cjsnNum:
		fjsnNumExport(item,data);
		break;
	}
	**data='\0';

	return 0;
}

/*========================================*\
    功能 : 导入STR节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnStrImport(struct tjsnItem **item,char **data)
{
	int result;

	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*data)++;
	(*item)->valsize=mjsnSize(data,"\"");
	(*item)->valdata=(char*)malloc((*item)->valsize);
	if((*item)->valdata==NULL)
		return -1;
	memcpy((*item)->valdata,*data,(*item)->valsize);
	*data+=(*item)->valsize;
	(*data)++;

	return 0;
}

/*========================================*\
    功能 : 导入NUM类型节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnNumImport(struct tjsnItem **item,char **data)
{
	int result;

	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*item)->valsize=mjsnSize(data,",]}");
	(*item)->valdata=(char*)malloc((*item)->valsize);
	if((*item)->valdata==NULL)
		return -1;
	memcpy((*item)->valdata,*data,(*item)->valsize);
	*data+=(*item)->valsize;

	return 0;
}

/*========================================*\
    功能 : 导入ARR类型节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnArrImport(struct tjsnItem **item,char **data)
{
	int result;

	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*data)++;
	while(1)
	{
		mjsnSkip(data,cjsnSpace);
		if(**data==']')
			break;
		struct tjsnItem *temp;
		result=fjsnAllImport(&temp,data);
		if(result!=0)
			return -1;
		fjsnInsert((*item),temp);
		mjsnSkip(data,cjsnSpace);
		if(**data!=','&&**data!=']')
			return -1;
		if(**data==',')
			(*data)++;
	}
	(*data)++;

	return 0;
}

/*========================================*\
    功能 : 导入OBJ类型节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnObjImport(struct tjsnItem **item,char **data)
{
	int result;

	result=fjsnInit(item);
	if(result!=0)
		return -1;
	(*data)++;
	while(1)
	{
		mjsnSkip(data,cjsnSpace);
		if(**data=='}')
			break;
		if(*(*data)++!='\"')
			return -1;
		int keysize;
		char *keydata;
		keysize=mjsnSize(data,"\"");
		keydata=(char*)malloc(keysize);
		if(keydata==NULL)
			return -1;
		memcpy(keydata,*data,keysize);
		*data+=keysize;
		(*data)++;
		mjsnSkip(data,cjsnSpace);
		if(*(*data)++!=':')
			return -1;
		mjsnSkip(data,cjsnSpace);
		struct tjsnItem *temp;
		result=fjsnAllImport(&temp,data);
		if(result!=0)
			return -1;
		fjsnInsert((*item),temp);
		temp->keysize=keysize;
		temp->keydata=keydata;
		mjsnSkip(data,cjsnSpace);
		if(**data!=','&&**data!='}')
			return -1;
		if(**data==',')
			(*data)++;
	}
	(*data)++;

	return 0;
}

/*========================================*\
    功能 : 导入节点
    参数 : (输出)节点
           (出入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnAllImport(struct tjsnItem **item,char **data)
{
	int result;

	switch(**data)
	{
		case '[':
		result=fjsnArrImport(item,data);
		if(result!=0)
			return -1;
		(*item)->type=cjsnArr;
		break;

		case '{':
		result=fjsnObjImport(item,data);
		if(result!=0)
			return -1;
		(*item)->type=cjsnObj;
		break;

		case '\"':
		result=fjsnStrImport(item,data);
		if(result!=0)
			return -1;
		(*item)->type=cjsnStr;
		break;

		default:
		result=fjsnNumImport(item,data);
		if(result!=0)
			return -1;
		(*item)->type=cjsnNum;
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnExport(struct tjsnItem *item,char *data)
{
	char *temp;
	temp=data;
	int result;
	result=fjsnAllExport(item,&temp);
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
int fjsnImport(struct tjsnItem **item,char *data)
{
	char *temp;
	temp=data;
	int result;
	result=fjsnAllImport(item,&temp);
	if(result!=0)
		return -1;
	return 0;
}
