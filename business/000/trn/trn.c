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
}

/*========================================*\
    功能 : 交易出口函数
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int ftrnFree(void)
{
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

	/*
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
	*/

	result=fdbsManage("insert into Journal values('%s','%s','%s','%s','%s','%s','%s','%s','%s')",
		bsncode,clilnkcode,clitrncode,trndate,trntime,trnjour,cliretstat,cliretcode,cliretinfo);
	if(result==-1)
		return -1;

	/*
	void *stm;
	result=fdbsSelectInit(&stm,"select BsnCode,CliLnkCode,CliTrnCode,TrnDate,TrnTime,TrnJour,RetStat,RetCode,RetInfo from Journal where TrnJour='%s'",trnjour);
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
	*/

	return 0;
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
		case -3:
		flogDepend("I'm error flow![-3]");
		case -2:
		flogDepend("I'm error flow![-2]");
		case -1:
		flogDepend("I'm error flow![-1]");
		break;
	}
	return;
}

/*========================================*\
    功能 : 批量扣款生产者
    参数 : 空
    返回 : (成功)0
           (失败)<0
\*========================================*/
int ftrnP01(void)
{
	int result;

	int error;
	error=-1;
	//raise(SIGSEGV);

	char *bsncode;
	result=fmmpRefGet("pBsnCode",0,&bsncode,0);
	if(result==-1)
		return error;

	error=-2;
	char *clilnkcode;
	result=fmmpRefGet("pCliLnkCode",0,&clilnkcode,0);
	if(result==-1)
		return error;
	flogDepend("CliLnkCode[%s]",clilnkcode);
	char *clitrncode;
	result=fmmpRefGet("pCliTrnCode",0,&clitrncode,0);
	if(result==-1)
		return error;
	flogDepend("CliTrnCode[%s]",clitrncode);

	char *host;
	result=fmmpRefGet("pCliHost",0,&host,0);
	if(result==-1)
		return error;
	flogDepend("Host[%s]",host);
	char *port;
	result=fmmpRefGet("pCliPort",0,&port,0);
	if(result==-1)
		return error;
	flogDepend("Port[%s]",port);

	char *a;
	result=fmmpRefSet("pa",0,&a,8);
	if(result==-1)
		return error;
	result=miniGetStr(bsncode,"001","a",a,0);
	if(result==-1)
		return error;
	flogDepend("a[%s]",a);
	char *b;
	result=fmmpRefSet("pb",0,&b,8);
	if(result==-1)
		return error;
	result=miniGetStr(bsncode,"001","b",b,0);
	if(result==-1)
		return error;
	flogDepend("b[%s]",b);

	error=-3;
	char path[64];
	sprintf(path,"%s/%s/1.txt",getenv("BUSINESS"),bsncode);
	FILE *fp;
	fp=fopen(path,"r");
	if(fp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",path);
		return error;
	}

	int product(void *fp)
	{
		int result;

		char line[128];
		fgets(line,sizeof(line),(FILE*)fp);
		if(ferror((FILE*)fp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			return -1;
		}
		if(feof((FILE*)fp))
			return -100;

		char *position1;
		char *position2;

		position1=line;
		position2=strchr(position1,'|');
		char a[16];
		strncpy(a,position1,position2-position1);
		a[position2-position1]='\0';
		result=fmmpValSet("pa",0,a,0);
		if(result==-1)
			return -1;

		position1=position2+1;
		position2=strchr(position1,'|');
		char b[16];
		strncpy(b,position1,position2-position1);
		b[position2-position1]='\0';
		result=fmmpValSet("pb",0,b,0);
		if(result==-1)
			return -1;

		flogDepend("a[%s]b[%s]",a,b);

		return 0;
	}

	result=fzoeBtcTrn("C01",product,fp);
	if(result==-1)
		return error;

	fclose(fp);

	return error;
}

/*========================================*\
    功能 : 批量扣款消费者
    参数 : 空
    返回 : (成功)0
           (失败)<0
\*========================================*/
int ftrnC01(void)
{
	int result;

	int error;
	error=-1;

	char *date;
	result=fmmpRefGet("pTrnDate",0,&date,0);
	if(result==-1)
		return error;
	flogDepend("Date[%s]",date);
	char *time;
	result=fmmpRefGet("pTrnTime",0,&time,0);
	if(result==-1)
		return error;
	flogDepend("Time[%s]",time);

	result=fmmpValSet("pekey",0,"65537",0);
	if(result==-1)
		return error;
	result=fmmpValSet("pnkey",0,"18963013433694059164783079915225901655884622091074455623574499651862890701825200425011382648260744403439001417193688265729378093626577075796360717783650574621923665542765757623427651370039267279780525967220566585222487722958100608921326562811541155532567087759322522936341472337052905491942526925060052097265465966812987107117709023325072545173483672712257692750081212398036439582803854982962650736975340774012499668990310100509762249890359881837746510073123517825535020610978031951578715699448412921461825061840155906764672123146449509537860959027200694776575503258977635017886329962721585174129885157203241757058779",0);
	if(result==-1)
		return error;
	result=fmmpValSet("pdkey",0,"9210676795215925940506239092442872186107805250684767515104982836686535365760573302549320813445231643536811459371318212900047731287181512203604872802356789548688299485635457520908200134836427906123466024559389136275612866641191046790181543413619540618130854648498317154753649360960016693963112262416255067918796430747448027433165632779289821088003692693886039972523188537376214437274956683317270579204452466431807523390867599288181809554521377439432147976827371047011202662824985525008447220263052744127327523618242946751794792592767875584842955021309994048910678140937773334205520913732139185573579782780497533715773",0);
	if(result==-1)
		return error;

	result=fzoeSrvTrn("002","A01");
	if(result==-1)
		return error;

	char *srvlnkcode;
	result=fmmpRefGet("pSrvLnkCode",0,&srvlnkcode,0);
	if(result==-1)
		return error;
	flogDepend("SrvLnkCode[%s]",srvlnkcode);
	char *srvtrncode;
	result=fmmpRefGet("pSrvTrnCode",0,&srvtrncode,0);
	if(result==-1)
		return error;
	flogDepend("SrvTrnCode[%s]",srvtrncode);

	char *a;
	result=fmmpRefGet("pa",0,&a,0);
	if(result==-1)
		return error;
	flogDepend("a[%s]",a);
	char *b;
	result=fmmpRefGet("pb",0,&b,0);
	if(result==-1)
		return error;
	flogDepend("b[%s]",b);

	error=0;
	return error;
}
