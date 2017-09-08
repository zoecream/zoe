/*========================================*\
    文件 : pkg.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>

#include <log.h>
#include <mmp.h>
#include <dbs.h>

/*========================================*\
    功能 : 报文入口函数
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)渠道类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgInit(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *lnkcode;
	if(strcmp(argument,"client")==0)
		result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
	else
	if(strcmp(argument,"server")==0)
		result=fmmpRefGet("pSrvLnkCode",0,&lnkcode,0);
	if(result==-1)
		return -1;

	if(strcmp(lnkcode,"001")==0)
	{
		result=fmmpValSet("pkey",0,"ABCDEFGH",0);
		if(result==-1)
			return -1;
		result=fmmpValSet("pvec",0,"abcdefgh",0);
		if(result==-1)
			return -1;

		result=fmmpValSet("pCliTrnCode",0,*fmldata+4,3);
		if(result==-1)
			return -1;
		*size-=7;
		memcpy(*tmpdata,*fmldata+7,*size);

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}
	else
	if(strcmp(lnkcode,"002")==0)
	{
		*size-=21;
		memcpy(*tmpdata,*fmldata+21,*size);

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}

	return 0;
}

/*========================================*\
    功能 : 报文出口函数
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)渠道类型
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgFree(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *lnkcode;
	if(strcmp(argument,"client")==0)
		result=fmmpRefGet("pCliLnkCode",0,&lnkcode,0);
	else
	if(strcmp(argument,"server")==0)
		result=fmmpRefGet("pSrvLnkCode",0,&lnkcode,0);
	if(result==-1)
		return -1;

	if(strcmp(lnkcode,"001")==0)
	{
		*size=0;
		sprintf(*tmpdata,"%04d",*size);
		result=fmmpValGet("pCliTrnCode",0,*tmpdata+4,0);
		if(result==-1)
			return -1;
		*size+=7;

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}
	else
	if(strcmp(lnkcode,"002")==0)
	{
		*size+=17;
		(*tmpdata)[0]=*((unsigned char*)size+3);
		(*tmpdata)[1]=*((unsigned char*)size+2);
		(*tmpdata)[2]=*((unsigned char*)size+1);
		(*tmpdata)[3]=*((unsigned char*)size+0);
		memcpy(*tmpdata+4,"\x60\x00\x90\x00\x00\x36\x39\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",17);
		memcpy(*tmpdata+21,*fmldata,*size-17);
		*size+=4;

		char *temp;
		temp=*fmldata;
		*fmldata=*tmpdata;
		*tmpdata=temp;
	}

	return 0;
}

/*========================================*\
    功能 : ETC PIN码编码
    参数 : (输入)来源数据
           (输入)来源长度
           (输出)目的数据
           (输出)目的长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgPinEnc(char *srcdata,int srcsize,char *dstdata,int *dstsize)
{
	int result;

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return -1;

	char _fmldata[64];
	char *fmldata=_fmldata;
	char _tmpdata[64];
	char *tmpdata=_tmpdata;
	int size;

	result=fmmpValGet("pMainKey",0,fmldata,&size);
	if(result==-1)
		return -1;
	result=fpkgHexDec(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
		return -1;
	result=fmmpValSet("pkey",0,fmldata,size);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,fmldata,size);
	if(result==-1)
		return -1;

	memset(fmldata,0X66,8);
	size=8;
	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-ede3_pkey_pvec_0");
	if(result==-1)
		return -1;
	memcpy(dstdata,fmldata,8);
	if(dstsize!=NULL)
		*dstsize=8;

	return 0;
}

/*========================================*\
    功能 : ETC MAC码编码
    参数 : (输入)来源数据
           (输入)来源长度
           (输出)目的数据
           (输出)目的长度
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkgMacEnc(char *srcdata,int srcsize,char *dstdata,int *dstsize)
{
	int result;

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return -1;

	char _fmldata[64];
	char *fmldata=_fmldata;
	char _tmpdata[64];
	char *tmpdata=_tmpdata;
	int size;

	if(srcsize%8!=0)
	{
		memset(srcdata+srcsize,0X00,8-srcsize%8);
		srcsize+=8-srcsize%8;
	}
	memset(dstdata,0X00,8);
	int i;
	int j;
	for(i=0;8*i<srcsize;i++)
		for(j=0;j<8;j++)
			dstdata[j]^=srcdata[8*i+j];

	result=fmmpValGet("pWorkKey",0,fmldata,&size);
	if(result==-1)
		return -1;
	result=fpkgHexDec(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
		return -1;
	result=fmmpValSet("pkey",0,fmldata,size);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,fmldata,size);
	if(result==-1)
		return -1;

	memcpy(fmldata,dstdata,8);
	size=8;
	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-ede3_pkey_pvec_0");
	if(result==-1)
		return -1;
	memcpy(dstdata,fmldata,8);
	if(dstsize!=NULL)
		*dstsize=8;

	return 0;
}

/*========================================*\
    功能 : 5110报文编码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)无
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkg5110Enc(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return -1;

	char __fmldata[64];
	char *_fmldata=__fmldata;
	char __tmpdata[64];
	char *_tmpdata=__tmpdata;
	int _size;

	char pin[8+1];
	result=fpkgPinEnc(NULL,0,pin,NULL);
	if(result==-1)
		return -1;
	memcpy(*fmldata,pin,8);

	result=fmmpValGet("pSrvTrnCode",0,_fmldata,&_size);
	if(result==-1)
		return -1;
	fpkgHexDec(&_fmldata,&_tmpdata,&_size,"upper");
	memcpy(*fmldata+8,_fmldata,_size);

	*(long*)(*fmldata+10)=0;
	*(long*)(*fmldata+10)|=1L<<(12-1);
	*(long*)(*fmldata+10)|=1L<<(13-1);
	*(long*)(*fmldata+10)|=1L<<(41-1);
	*(long*)(*fmldata+10)|=1L<<(63-1);
	*(long*)(*fmldata+10)|=1L<<(64-1);

	result=fmmpValGet("pTrnTime",0,_fmldata,&_size);
	if(result==-1)
		return -1;
	fpkgHexDec(&_fmldata,&_tmpdata,&_size,"upper");
	memcpy(*fmldata+18,_fmldata,_size);

	result=fmmpValGet("pTrnDate",0,_fmldata,&_size);
	if(result==-1)
		return -1;
	fpkgHexDec(&_fmldata,&_tmpdata,&_size,"upper");
	memcpy(*fmldata+21,_fmldata,_size);

	char termcode[14+1];
	result=fmmpValGet("pTermCode",0,termcode,0);
	if(result==-1)
		return -1;
	memcpy(*fmldata+25,termcode,14);

	memcpy(*fmldata+39,"013",3);

	char usercode[13+1];
	result=fmmpValGet("pUserCode",0,usercode,0);
	if(result==-1)
		return -1;
	memcpy(*fmldata+42,usercode,13);

	char mac[8+1];
	result=fpkgMacEnc(*fmldata+8,47,mac,NULL);
	if(result==-1)
		return -1;
	memcpy(*fmldata+55,mac,8);

	*size=63;

	return 0;
}

/*========================================*\
    功能 : 5110报文解码
    参数 : (出入)正式报文数据
           (出入)临时报文数据
           (出入)报文长度
           (输入)无
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fpkg5110Dec(char **fmldata,char **tmpdata,int *size,char *argument)
{
	int result;

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return -1;

	char __fmldata[64];
	char *_fmldata=__fmldata;
	char __tmpdata[64];
	char *_tmpdata=__tmpdata;
	int _size;

	char retcode1[2+1];
	memcpy(retcode1,*fmldata+25,2);
	retcode1[2]='\0';
	result=fmmpValSet("pSrvRetCode1",0,retcode1,0);
	if(result==-1)
		return -1;

	char retcode2[4+1];
	memcpy(retcode2,*fmldata+44,4);
	retcode2[4]='\0';
	result=fmmpValSet("pSrvRetCode2",0,retcode2,0);
	if(result==-1)
		return -1;

	memcpy(_fmldata,*fmldata+65,16);
	_size=16;
	result=fpkgHexEnc(&_fmldata,&_tmpdata,&_size,"upper");
	if(result==-1)
		return -1;
	/*
	if(*(*fmldata+64)=='1')
		result=miniSetStr(bsncode,"etc","WorkKey",_fmldata,_size);
	else
	if(*(*fmldata+64)=='2')
		result=miniSetStr(bsncode,"etc","MainKey",_fmldata,_size);
	if(result==-1)
		return -1;
	*/

	return 0;
}
