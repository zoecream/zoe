/*========================================*\
    文件 : emu.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>

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
