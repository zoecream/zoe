/*========================================*\
    文件 : dbs.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>

#include <log.h>
#include <dbs.h>

//一次性执行.
pthread_once_t vdbsOnce=PTHREAD_ONCE_INIT;
//本地化存储.
pthread_key_t vdbsEnv;
pthread_key_t vdbsDbc;
pthread_key_t vdbsStm;

/*========================================*\
    功能 : 连接数据库
    参数 : (输入)业务代码
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsInit(char *bsncode)
{
	int result;

	void create(void)
	{
		pthread_key_create(&vdbsEnv,NULL);
		pthread_key_create(&vdbsDbc,NULL);
		pthread_key_create(&vdbsStm,NULL);
	}
	result=pthread_once(&vdbsOnce,create);
	if(result!=0)
	{
		mlogError("pthread_once",result,strerror(result),"[]");
		return -1;
	}
	void *env;
	void *dbc;
	void *stm;

	int status;
	char info[256];

	result=SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&env);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_ENV,env,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLAllocHandle",status,info,"[]");
		return -1;
	}
	result=SQLSetEnvAttr(env,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_ENV,env,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetEnvAttr",status,info,"[]");
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}

	result=SQLAllocHandle(SQL_HANDLE_DBC,env,&dbc);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_ENV,env,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLAllocHandle",status,info,"[]");
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}
	SQLSetConnectAttr(dbc,SQL_LOGIN_TIMEOUT,(SQLPOINTER)4,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetConnectAttr",status,info,"[]");
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}
	result=SQLConnect(dbc,(SQLCHAR*)bsncode,SQL_NTS,(SQLCHAR*)NULL,SQL_NTS,(SQLCHAR*)NULL,SQL_NTS);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLConnect",status,info,"[%s]",bsncode);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}
	result=SQLSetConnectAttr(dbc,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER)SQL_TXN_READ_COMMITTED,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetConnectAttr",status,info,"[]");
		SQLDisconnect(dbc);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}

	result=SQLAllocHandle(SQL_HANDLE_STMT,dbc,&stm);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLAllocHandle",status,info,"[]");
		SQLDisconnect(dbc);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}

	result=pthread_setspecific(vdbsEnv,env);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		SQLFreeHandle(SQL_HANDLE_STMT,stm);
		SQLDisconnect(dbc);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}
	result=pthread_setspecific(vdbsDbc,dbc);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		SQLFreeHandle(SQL_HANDLE_STMT,stm);
		SQLDisconnect(dbc);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}
	result=pthread_setspecific(vdbsStm,stm);
	if(result!=0)
	{
		mlogError("pthread_setspecific",result,strerror(result),"[]");
		SQLFreeHandle(SQL_HANDLE_STMT,stm);
		SQLDisconnect(dbc);
		SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		SQLFreeHandle(SQL_HANDLE_ENV,env);
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 断开数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsFree(void)
{
	int result;

	int status;
	char info[256];

	void *stm;
	stm=pthread_getspecific(vdbsStm);
	if(stm!=NULL)
	{
		result=SQLFreeHandle(SQL_HANDLE_STMT,stm);
		if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
			info[5]='-';
			mlogError("SQLFreeHandle",status,info,"[]");
			return -1;
		}
		result=pthread_setspecific(vdbsStm,NULL);
		if(result!=0)
		{
			mlogError("pthread_setspecific",result,strerror(result),"[]");
			return -1;
		}
	}
	void *dbc;
	dbc=pthread_getspecific(vdbsDbc);
	if(dbc!=NULL)
	{
		result=SQLDisconnect(dbc);
		if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
			info[5]='-';
			mlogError("SQLDisconnect",status,info,"[]");
			return -1;
		}
		result=SQLFreeHandle(SQL_HANDLE_DBC,dbc);
		if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
			info[5]='-';
			mlogError("SQLFreeHandle",status,info,"[]");
			return -1;
		}
		result=pthread_setspecific(vdbsDbc,NULL);
		if(result!=0)
		{
			mlogError("pthread_setspecific",result,strerror(result),"[]");
			return -1;
		}
	}
	void *env;
	env=pthread_getspecific(vdbsEnv);
	if(env!=NULL)
	{
		result=SQLFreeHandle(SQL_HANDLE_ENV,env);
		if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
		{
			SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
			info[5]='-';
			mlogError("SQLFreeHandle",status,info,"[]");
			return -1;
		}
		result=pthread_setspecific(vdbsEnv,NULL);
		if(result!=0)
		{
			mlogError("pthread_setspecific",result,strerror(result),"[]");
			return -1;
		}
	}
	return;
}

/*========================================*\
    功能 : 数据库管理
    参数 : (输入)SQL语句格式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsManage(char *format,...)
{
	int result;

	int status;
	char info[256];

	va_list list;
	va_start(list,format);
	char statment[1024];
	vsprintf(statment,format,list);
	va_end(list);

	void *stm;
	stm=pthread_getspecific(vdbsStm);

	result=SQLExecDirect(stm,statment,SQL_NTS);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO&&result!=SQL_NO_DATA)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLExecDirect",status,info,"[%s]",statment);
		return -1;
	}

	long affect;
	result=SQLRowCount(stm,&affect);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLRowCount",status,info,"[]");
		return -1;
	}

	//SQLFreeStmt(stm,SQL_CLOSE);
	//SQLFreeStmt(stm,SQL_UNBIND);

	if(affect==0)
		return -100;
	else
		return affect;
}

/*========================================*\
    功能 : 数据库查询预备
    参数 : (出入)语句句柄
           (输入)语句格式
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsSelectInit(void **stm,char *format,...)
{
	int result;

	int status;
	char info[256];

	va_list list;
	va_start(list,format);
	char statment[1024];
	vsprintf(statment,format,list);
	va_end(list);

	void *dbc;
	dbc=pthread_getspecific(vdbsDbc);

	result=SQLAllocHandle(SQL_HANDLE_STMT,dbc,stm);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLAllocHandle",status,info,"[]");
		return -1;
	}

	result=SQLExecDirect(*stm,statment,SQL_NTS);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,*stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLExecDirect",status,info,"[%s]",statment);
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库查询绑定
    参数 : (输入)语句句柄
           (输入)数据序号
           (输入)数据类型
           (输入)数据空间
           (输入)数据尺寸
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsSelectBind(void *stm,int index,int type,void *data,int size)
{
	int result;

	int status;
	char info[256];

	result=SQLBindCol(stm,index,type,data,size,NULL);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLBindCol",status,info,"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库查询提取
    参数 : (输入)语句句柄
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsSelectGain(void *stm)
{
	int result;

	int status;
	char info[256];

	result=SQLFetch(stm);
	if(result==SQL_NO_DATA)
	{
		return -100;
	}
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLFetch",status,info,"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库查询结束
    参数 : (输入)语句句柄
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsSelectFree(void *stm)
{
	int result;

	int status;
	char info[256];

	result=SQLFreeHandle(SQL_HANDLE_STMT,stm);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_STMT,stm,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLFreeHandle",status,info,"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库事务开始
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsTran(void)
{
	int result;

	int status;
	char info[256];

	void *dbc;
	dbc=pthread_getspecific(vdbsDbc);

	result=SQLSetConnectAttr(dbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_OFF,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetConnectAttr",status,info,"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库事务提交
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsComm(void)
{
	int result;

	int status;
	char info[256];

	void *dbc;
	dbc=pthread_getspecific(vdbsDbc);

	result=SQLEndTran(SQL_HANDLE_DBC,dbc,SQL_COMMIT);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLEndTran",status,info,"[]");
		return -1;
	}
	result=SQLSetConnectAttr(dbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetConnectAttr",status,info,"[]");
		return -1;
	}

	return 0;
}

/*========================================*\
    功能 : 数据库事务回滚
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsRoll(void)
{
	int result;

	int status;
	char info[256];

	void *dbc;
	dbc=pthread_getspecific(vdbsDbc);

	result=SQLEndTran(SQL_HANDLE_DBC,dbc,SQL_ROLLBACK);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLEndTran",status,info,"[]");
		return -1;
	}
	result=SQLSetConnectAttr(dbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,0);
	if(result!=SQL_SUCCESS&&result!=SQL_SUCCESS_WITH_INFO)
	{
		SQLGetDiagRec(SQL_HANDLE_DBC,dbc,1,info,&status,info+6,sizeof(info)-6,NULL);
		info[5]='-';
		mlogError("SQLSetConnectAttr",status,info,"[]");
		return -1;
	}

	return 0;
}
