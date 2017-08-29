/*========================================*\
    文件 : trn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <log.h>
#include <ini.h>
#include <mmp.h>
#include <dbs.h>

/*========================================*\
    功能 : 交易入口函数
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int ftrnInit(void)
{
	/*
	int result;

	result=fdbsTran();
	if(result==-1)
		return -1;

	result=fdbsManage("update JournalNo set TrnJour=TrnJour+1");
	if(result==-1)
	{
		fdbsRoll();
		return -1;
	}
	if(result==-100)
	{
		result=fdbsManage("insert into JournalNo values (0)");
		if(result==-1)
		{
			fdbsRoll();
			return -1;
		}
	}

	void *stm;
	fdbsSelectInit(&stm,"select TrnJour from JournalNo");
	int temp;
	mdbsSelectBindInt(stm,1,&temp,0);
	result=fdbsSelectGain(stm);
	if(result==-1)
	{
		fdbsRoll();
		return -1;
	}
	fdbsSelectFree(stm);

	if(temp>9999999)
	{
		temp=0;
		result=fdbsManage("update JournalNo set TrnJour=0");
		if(result==-1)
		{
			fdbsRoll();
			return -1;
		}
	}
	fdbsComm();

	char trnjour[31+1];
	sprintf(trnjour,"%s%07d",flogDate(0),temp);

	result=fmmpValSet("pTrnJour",0,trnjour,0);
	if(result==-1)
		return -1;
	flogDepend("TrnJour[%s]",trnjour);

	return 0;
	*/
}

/*========================================*\
    功能 : 交易出口函数
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int ftrnFree(void)
{
	/*
	int result;

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return -1;
	char *clilnkcode;
	result=fmmpRefGet("pCliLnkCode",0,&clilnkcode,0);
	if(result==-1)
		return -1;
	char *clitrncode;
	result=fmmpRefGet("pCliTrnCode",0,&clitrncode,0);
	if(result==-1)
		return -1;
	char *trndate;
	result=fmmpRefGet("pTrnDate",0,&trndate,0);
	if(result==-1)
		return -1;
	char *trntime;
	result=fmmpRefGet("pTrnTime",0,&trntime,0);
	if(result==-1)
		return -1;
	char *trnjour;
	result=fmmpRefGet("pTrnJour",0,&trnjour,0);
	if(result==-1)
		return -1;
	char *cliretstat;
	result=fmmpRefGet("pCliRetStat",0,&cliretstat,0);
	if(result==-1)
		return -1;
	char *cliretcode;
	result=fmmpRefGet("pCliRetCode",0,&cliretcode,0);
	if(result==-1)
		return -1;
	char *cliretinfo;
	result=fmmpRefGet("pCliRetInfo",0,&cliretinfo,0);
	if(result==-1)
		return -1;

	/-*
	flogDepend("bsncode[%s]",bsncode);
	flogDepend("clilnkcode[%s]",clilnkcode);
	flogDepend("clitrncode[%s]",clitrncode);
	flogDepend("trndate[%s]",trndate);
	flogDepend("trntime[%s]",trntime);
	flogDepend("trnjour[%s]",trnjour);
	flogDepend("cliretstat[%s]",cliretstat);
	flogDepend("cliretcode[%s]",cliretcode);
	flogDepend("cliretinfo[%s]",cliretinfo);

	flogDepend("");
	*-/

	result=fdbsManage("insert into Journal values('%s','%s','%s','%s','%s','%s','%s','%s','%s')",
		bsncode,clilnkcode,clitrncode,trndate,trntime,trnjour,cliretstat,cliretcode,cliretinfo);
	if(result==-1)
		return -1;

	/-*
	void *stm;
	result=fdbsSelectInit(&stm,"select BsnCode,CliLnkCode,CliTrnCode,TrnDate,TrnTime,TrnJour,CliRetStat,CliRetCode,CliRetInfo from Journal where TrnJour='%s'",trnjour);
	if(result==-1)
		return -1;

	char _bsncode[3+1];
	char _clilnkcode[3+1];
	char _clitrncode[15+1];
	char _trndate[8+1];
	char _trntime[6+1];
	char _trnjour[31+1];
	char _cliretstat[1+1];
	char _cliretcode[8+1];
	char _cliretinfo[63+1];

	mdbsSelectBindStr(stm,1,_bsncode,sizeof(_bsncode));
	mdbsSelectBindStr(stm,2,_clilnkcode,sizeof(_clilnkcode));
	mdbsSelectBindStr(stm,3,_clitrncode,sizeof(_clitrncode));
	mdbsSelectBindStr(stm,4,_trndate,sizeof(_trndate));
	mdbsSelectBindStr(stm,5,_trntime,sizeof(_trntime));
	mdbsSelectBindStr(stm,6,_trnjour,sizeof(_trnjour));
	mdbsSelectBindStr(stm,7,_cliretstat,sizeof(_cliretstat));
	mdbsSelectBindStr(stm,8,_cliretcode,sizeof(_cliretcode));
	mdbsSelectBindStr(stm,9,_cliretinfo,sizeof(_cliretinfo));

	result=fdbsSelectGain(stm);
	if(result==-1)
		return -1;
	if(result==-100)
		return -1;

	flogDepend("bsncode[%s]",_bsncode);
	flogDepend("clilnkcode[%s]",_clilnkcode);
	flogDepend("clitrncode[%s]",_clitrncode);
	flogDepend("trndate[%s]",_trndate);
	flogDepend("trntime[%s]",_trntime);
	flogDepend("trnjour[%s]",_trnjour);
	flogDepend("cliretstat[%s]",_cliretstat);
	flogDepend("cliretcode[%s]",_cliretcode);
	flogDepend("cliretinfo[%s]",_cliretinfo);

	fdbsSelectFree(stm);
	*-/

	return 0;
	*/
}

/*========================================*\
    功能 : 错误处理函数
    参数 : (输入)错误标识
    返回 : 空
\*========================================*/
void ftrnError(int error)
{
	switch(error)
	{
		case -1:
		flogDepend("I'm error flow!");
		case -2:
		break;
	}
	return;
}

/*========================================*\
    功能 : 签到
    参数 : 空
    返回 : (成功)0
           (失败)<0
\*========================================*/
int ftrnM01(void)
{
	int result;

	int error;
	error=-1;

	result=fzoeSrvTrn("002","5110");
	if(result==-1)
		return error;

	char *srvretcode1;
	result=fmmpRefGet("pSrvRetCode1",0,&srvretcode1,0);
	if(result==-1)
		return error;
	flogDepend("SrvRetCode1[%s]",srvretcode1);
	char *srvretcode2;
	result=fmmpRefGet("pSrvRetCode2",0,&srvretcode2,0);
	if(result==-1)
		return error;
	flogDepend("SrvRetCode2[%s]",srvretcode2);

	error=0;
	return error;
}
