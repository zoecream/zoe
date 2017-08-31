/*========================================*\
    文件 : jsn.h
    作者 : 陈乐群
\*========================================*/

#ifndef __JSN__
#define __JSN__

#define cjsnStr 1
#define cjsnNum 2
#define cjsnArr 3
#define cjsnObj 4

struct tjsnItem
{
	char type;
	struct tjsnItem *chld;
	struct tjsnItem *next;
	char *keydata;
	int keysize;
	char *valdata;
	int valsize;
};

/*========================================*\
    功能 : 节点分配
    参数 : (输出)节点
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnInit(struct tjsnItem **item);
/*========================================*\
    功能 : 节点释放
    参数 : (输入)节点
    返回 : 空
\*========================================*/
void fjsnFree(struct tjsnItem *item);

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
int fjsnCreate(struct tjsnItem **item,int type,char *keydata,int keysize,char *valdata,int valsize);
/*========================================*\
    功能 : 插入节点
    参数 : (输入)目标节点
           (输入)插入节点
    返回 : 空
\*========================================*/
void fjsnInsert(struct tjsnItem *targete,struct tjsnItem *insert);
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
int fjsnSelect(struct tjsnItem *arr,struct tjsnItem **item,char *keydata,int keysize,int index);

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (输出)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnExport(struct tjsnItem *item,char *data);
/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (输入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fjsnImport(struct tjsnItem **item,char *data);

#endif
