/*========================================*\
    文件 : dbs.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <dbs.h>
#include <log.h>

int main(int argc,char *argv[])
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	result=fdbsInit("000");
	if(result==-1)
	{
		fprintf(stderr,"fdbsInit failed!\n");
		return -1;
	}

	//测试插入.
	result=fdbsManage("insert into test values (1,1000,1.1,'a')");
	if(result==-1)
		return -1;
	printf("%d\n",result);
	result=fdbsManage("insert into test values (2,2000,2.2,'b')");
	if(result==-1)
		return -1;
	printf("%d\n",result);

	//测试删除.
	result=fdbsManage("delete from test where s=9");
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");
	else
		printf("%d\n",result);

	result=fdbsManage("delete from test where s=2");
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");
	else
		printf("%d\n",result);

	//测试更新.
	result=fdbsManage("update test set c='A' where s=9");
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");
	else
		printf("%d\n",result);

	result=fdbsManage("update test set c='A' where s=1");
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");
	else
		printf("%d\n",result);

	//测试单行查询.
	void *stm;
	result=fdbsSelectInit(&stm,"select * from test where s=1");
	if(result==-1)
		return -1;

	short s1;
	int i1;
	double d1;
	char c1[20+1];

	mdbsSelectBindSht(stm,1,&s1,0);
	mdbsSelectBindInt(stm,2,&i1,0);
	mdbsSelectBindDbl(stm,3,&d1,0);
	mdbsSelectBindStr(stm,4,c1,sizeof(c1));

	result=fdbsSelectGain(stm);
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");
	printf("result:[%hd][%d][%f][%s]\n",s1,i1,d1,c1);

	result=fdbsSelectGain(stm);
	if(result==-1)
		return -1;
	if(result==-100)
		printf("none data\n");

	fdbsSelectFree(stm);

	//测试聚合函数.
	result=fdbsSelectInit(&stm,"select max(s),min(s),count(*),sum(d),avg(d) from test");
	if(result==-1)
		return -1;

	short max;
	short min;
	int cnt;
	double sum;
	double avg;

	mdbsSelectBindSht(stm,1,&max,0);
	mdbsSelectBindSht(stm,2,&min,0);
	mdbsSelectBindInt(stm,3,&cnt,0);
	mdbsSelectBindDbl(stm,4,&sum,0);
	mdbsSelectBindDbl(stm,5,&avg,0);

	result=fdbsSelectGain(stm);
	if(result==-1)
		return -1;

	printf("max:%hd\n",max);
	printf("min:%hd\n",min);
	printf("cnt:%d\n",cnt);
	printf("sum:%f\n",sum);
	printf("avg:%f\n",avg);
	
	fdbsSelectFree(stm);

	//测试事务开始/提交/回滚.
	result=fdbsTran();
	if(result==-1)
		return -1;
	result=fdbsManage("insert into test values (3,3000,3.3,'c')");
	if(result==-1)
		return -1;
	result=fdbsRoll();
	if(result==-1)
		return -1;

	result=fdbsTran();
	if(result==-1)
		return -1;
	result=fdbsManage("insert into test values (4,4000,4.4,'d')");
	if(result==-1)
		return -1;
	result=fdbsComm();
	if(result==-1)
		return -1;

	//测试多行查询.
	result=fdbsSelectInit(&stm,"select * from test");
	if(result==-1)
		return -1;

	short s2;
	int i2;
	double d2;
	char c2[20+1];

	mdbsSelectBindSht(stm,1,&s2,0);
	mdbsSelectBindInt(stm,2,&i2,0);
	mdbsSelectBindDbl(stm,3,&d2,0);
	mdbsSelectBindStr(stm,4,c2,sizeof(c2));

	while(1)
	{
		result=fdbsSelectGain(stm);
		if(result==-1)
			return -1;
		if(result==-100)
			break;
		printf("result:[%hd][%d][%f][%s]\n",s2,i2,d2,c2);
	}

	fdbsSelectFree(stm);

	fdbsFree();

	return 0;
}
