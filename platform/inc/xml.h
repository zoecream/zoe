/*========================================*\
    文件 : xml.h
    作者 : 陈乐群
\*========================================*/

#ifndef __JSN__
#define __JSN__

#define cxmlNode 1
#define cxmlAttr 2

struct txmlItem
{
	char type;
	struct txmlItem *nodechld;
	struct txmlItem *attrchld;
	struct txmlItem *next;
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
int fxmlInit(struct txmlItem **item);
/*========================================*\
    功能 : 节点释放
    参数 : (出入)节点
    返回 : 空
\*========================================*/
void fxmlFree(struct txmlItem **item);

/*========================================*\
    功能 : 创建节点
    参数 : (输出)节点
           (输入)路径
           (输入)值数据
           (输入)值长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlCreate(struct txmlItem **item,char *path,char *data,int size);
/*========================================*\
    功能 : 查询节点
    参数 : (输入)节点
           (输入)路径
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlSelect(struct txmlItem *item,char *path,char *data,int *size);

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (输入)报文
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char *data);
/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (输入)报文数据
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem **item,char *data);

#endif
