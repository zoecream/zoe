/*========================================*\
    文件 : ini.h
    作者 : 陈乐群
\*========================================*/

#ifndef __INI__
#define __INI__

/*========================================*\
    功能 : 获取配置
    参数 : (输入)业务代码
           (输入)段
           (输入)键
           (输出)数据
           (输出)长度
           (输入)类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int finiGet(char *bsncode,char *sec,char *key,void *data,int *size,int type);
/*========================================*\
    功能 : 设置配置
    参数 : (输入)业务代码
           (输入)段
           (输入)键
           (输入)数据
           (输入)长度
           (输入)类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int finiSet(char *bsncode,char *sec,char *key,void *data,int size,int type);

//设置STR类型配置.
#define miniGetStr(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,0)
//获取STR类型配置.
#define miniSetStr(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,size,0)
//设置CHR类型配置.
#define miniGetChr(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,1)
//获取CHR类型配置.
#define miniSetChr(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(char),1)
//设置SHT类型配置.
#define miniGetSht(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,2)
//获取SHT类型配置.
#define miniSetSht(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(short),2)
//设置INT类型配置.
#define miniGetInt(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,3)
//获取INT类型配置.
#define miniSetInt(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(int),3)
//设置LNG类型配置.
#define miniGetLng(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,4)
//获取LNG类型配置.
#define miniSetLng(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(long),4)
//设置FLT类型配置.
#define miniGetFlt(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,5)
//获取FLT类型配置.
#define miniSetFlt(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(float),5)
//设置DBL类型配置.
#define miniGetDbl(bsncode,sec,key,data,size) finiGet(bsncode,sec,key,data,size,6)
//获取DBL类型配置.
#define miniSetDbl(bsncode,sec,key,data,size) finiSet(bsncode,sec,key,data,sizeof(double),6)

#endif
