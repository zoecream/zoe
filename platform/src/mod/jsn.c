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
#define mjsnSize(data,reject) strcspn(data,reject)

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
    参数 : (出入)节点
    返回 : 空
\*========================================*/
void fjsnFree(struct tjsnItem **item)
{
	if((*item)->chld)
		fjsnFree(&(*item)->chld);
	if((*item)->next)
		fjsnFree(&(*item)->next);
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
int fjsnCreate(struct tjsnItem **item,char *path,char *data,int size)
{
	int result;
	if(*path!='/'&&*path!=':')
		return -1;
	if(data==NULL||size==0)
		return -1;
	if(*item==NULL)
	{
		result=fjsnInit(item);
		if(result!=0)
			return -1;
		if(*path=='/')
			(*item)->type=cjsnObj;
		else
		if(*path==':')
			(*item)->type=cjsnArr;
	}
	else
	{
		if(*path=='/'&&(*item)->type!=cjsnObj)
			return -1;
		if(*path==':'&&(*item)->type!=cjsnArr)
			return -1;
	}
	struct tjsnItem **temp;
	temp=item;
	path++;
	while(1)
	{
		item=&(*item)->chld;
		int index;
		if(*(path-1)==':')
			index=atoi(path);
		else
			index=0;
		while(1)
		{
			if(*item==NULL)
			{
				if(index!=0)
					return -1;
				result=fjsnInit(item);
				if(result!=0)
					return -1;
				if(*(path+mjsnSize(path,"/:"))=='/')
					(*item)->type=cjsnObj;
				else
				if(*(path+mjsnSize(path,"/:"))==':')
					(*item)->type=cjsnArr;
				else
				if(*(path+mjsnSize(path,"/:"))=='\0')
					(*item)->type=cjsnStr;
				else
					return -1;
				if(*(path-1)=='/')
				{
					(*item)->keysize=mjsnSize(path,"/:");
					(*item)->keydata=(char*)malloc((*item)->keysize);
					if((*item)->keydata==NULL)
						return -1;
					memcpy((*item)->keydata,path,(*item)->keysize);
				}
				break;
			}
			if(*(path-1)=='/')
			{
				if((*item)->keysize!=mjsnSize(path,"/:"))
					goto next;
				if(memcmp((*item)->keydata,path,(*item)->keysize)!=0)
					goto next;
			}
			else
			if(*(path-1)==':')
			{
				if(index--!=0)
					goto next;
			}
			break;
			next:
			item=&((*item)->next);
		}
		if(*(path+mjsnSize(path,"/:"))=='\0')
			break;
		path+=mjsnSize(path,"/:")+1;
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
int fjsnSelect(struct tjsnItem *item,char *path,char *data,int *size)
{
	int result;
	if(*path!='/'&&*path!=':')
		return -1;
	if(item==NULL)
		return -1;
	if(*path=='/'&&item->type!=cjsnObj)
		return -1;
	if(*path==':'&&item->type!=cjsnArr)
		return -1;
	path++;
	while(1)
	{
		item=item->chld;
		int index;
		if(*(path-1)==':')
			index=atoi(path);
		else
			index=0;
		while(1)
		{
			if(item==NULL)
				return -1;
			if(*(path-1)=='/')
			{
				if(item->keysize!=mjsnSize(path,"/:"))
					goto next;
				if(memcmp(item->keydata,path,item->keysize)!=0)
					goto next;
			}
			else
			if(*(path-1)==':')
			{
				if(index--!=0)
					goto next;
			}
			break;
			next:
			item=item->next;
		}
		if(*(path+mjsnSize(path,"/:"))=='\0')
			break;
		path+=mjsnSize(path,"/:")+1;
	}
	*size=item->valsize;
	memcpy(data,item->valdata,*size);
	data[*size]='\0';
	return 0;
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
	(*item)->valsize=mjsnSize(*data,"\"");
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
	(*item)->valsize=mjsnSize(*data,",]}");
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
		struct tjsnItem **temp;
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
		result=fjsnAllImport(temp,data);
		if(result!=0)
			return -1;
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
		keysize=mjsnSize(*data,"\"");
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
		struct tjsnItem **temp;
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
		result=fjsnAllImport(temp,data);
		if(result!=0)
			return -1;
		(*temp)->keysize=keysize;
		(*temp)->keydata=keydata;
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
