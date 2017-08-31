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
	char *keyd;
	int keyl;
	char *vald;
	int vall;
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
    参数 : (输入)节点
    返回 : 空
\*========================================*/
void fxmlFree(struct txmlItem *item);

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
int fxmlCreate(struct txmlItem **item,int type,char *keyd,int keyl,char *vald,int vall);
/*========================================*\
    功能 : 插入节点
    参数 : (输入)目标节点
           (输入)插入节点
           (输入)类型
    返回 : 空
\*========================================*/
void fxmlInsert(struct txmlItem *target,struct txmlItem *insert,int type);
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
int fxmlSelect(struct txmlItem *target,struct txmlItem **select,int type,char *keyd,int keyl,int index);

/*========================================*\
    功能 : 将节点导出到报文
    参数 : (输入)节点
           (出入)报文数据
           (输出)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlExport(struct txmlItem *item,char **pd,int *pl);
/*========================================*\
    功能 : 将报文导入到节点
    参数 : (输出)节点
           (出入)报文数据
           (输入)报文长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fxmlImport(struct txmlItem **item,char **pd,int pl);

#endif
