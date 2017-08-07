/*========================================*\
    文件 : emu.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>

#include <pkg.h>

/*========================================*\
    功能 : 添加报文长度
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)无
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuLength(char **fmldata,char **tmpdata,int *size,char *argument)
{
	sprintf(*tmpdata,"%04d",*size-3);
	memcpy(*tmpdata+4,*fmldata,*size);
	*size+=4;

	char *temp;
	temp=*fmldata;
	*fmldata=*tmpdata;
	*tmpdata=temp;

	return 0;
}

/*========================================*\
    功能 : 十六进制解码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)大写(upper)/小写(lower)
    返回 : (成功)0
           (失败)-1
\*========================================*/
int femuHexDec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;
	result=fpkgHexDec(fmldata,tmpdata,size,argument);
	if(result==-1)
		return -1;
	return 0;
}
