/*========================================*\
    文件 : dbs.c
    作者 : 陈乐群
\*========================================*/

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <getopt.h>
#include <unistd.h>
#include <dirent.h>

#include <dbs.h>
#include <log.h>

char *vdbsTraverseSQL[5]=
{
	"",
	"",
	"",
	"select table_name from information_schema.tables where table_schema like '%%%s%%'",
	""
};

/*========================================*\
    功能 : 启动数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsServiceBoot(void);
/*========================================*\
    功能 : 停止数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsServiceShut(void);
/*========================================*\
    功能 : 显示数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsServiceList(void);

/*========================================*\
    功能 : 创建表格
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsTableCreate(void);
/*========================================*\
    功能 : 删除表格
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsTableRemove(void);

/*========================================*\
    功能 : 删除数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataDelete(void);
/*========================================*\
    功能 : 导入数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataImport(void);
/*========================================*\
    功能 : 导出数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataExport(void);

/*========================================*\
    功能 : 检查参数格式
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsExam(void);
/*========================================*\
    功能 : 显示帮助信息
    参数 : 空
    返回 : 空
\*========================================*/
void fdbsHelp(void);

//业务代码.
char vdbsBsnCode[3+1];
//表格名称.
char vdbsTable[32];

int main(int argc,char *argv[])
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	struct option options[]=
	{
		{"create",required_argument,NULL,'c'},
		{"remove",required_argument,NULL,'r'},
		{"delete",required_argument,NULL,'d'},
		{"import",required_argument,NULL,'i'},
		{"export",required_argument,NULL,'e'},
		{"help"  ,no_argument      ,NULL,'h'},
		{0,0,0,0}
	};

	int option;
	option=getopt_long(argc,argv,":bslc:r:d:i:e:h",options,NULL);

	switch(option)
	{
		case 'c':
		result=fdbsTableCreate();
		if(result==-1)
			return -1;
		break;

		case 'r':
		result=fdbsTableRemove();
		if(result==-1)
			return -1;
		break;

		case 'd':
		result=fdbsDataDelete();
		if(result==-1)
			return -1;
		break;

		case 'i':
		result=fdbsDataImport();
		if(result==-1)
			return -1;
		break;

		case 'e':
		result=fdbsDataExport();
		if(result==-1)
			return -1;
		break;

		case 'h':
		case '?':
		case ':':
		case -1:
		fdbsHelp();
		break;
	}

	return 0;
}

/*========================================*\
    功能 : 启动数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
//int fdbsServiceBoot(void)
//{
	/*
	if(strcmp(getenv("DATABASE"),"oradsn")==0)
	{
		system("lsnrctl start");
		system("echo -e \"startup\" | sqlplus / as SYSDBA");
	}
	else
	if(strcmp(getenv("DATABASE"),"sybdsn")==0)
	{
		system("startserver -f /home/syb/setup/ASE-15_0/install/RUN_syb");
		system("startserver -f /home/syb/setup/ASE-15_0/install/RUN_sybback");
	}
	else
	if(strcmp(getenv("DATABASE"),"db2dsn")==0)
	{
		system("db2start");
		system("db2 connect to db2data");
	}
	else
	if(strcmp(getenv("DATABASE"),"mysdsn")==0)
	{
	}
	*/
	//return 0;
//}

/*========================================*\
    功能 : 停止数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
//int fdbsServiceShut(void)
//{
	/*
	if(strcmp(getenv("DATABASE"),"oradsn")==0)
	{
		system("echo -e \"shutdown\" | sqlplus / as SYSDBA");
		system("lsnrctl stop");
	}
	else
	if(strcmp(getenv("DATABASE"),"sybdsn")==0)
	{
		system("echo -e \"shutdown SYB_BACKUP\\ngo\" | isql64 -Usa -Psybpass");
		system("echo -e \"shutdown\\ngo\" | isql64 -Usa -Psybpass");
	}
	else
	if(strcmp(getenv("DATABASE"),"db2dsn")==0)
	{
		system("db2stop");
	}
	else
	if(strcmp(getenv("DATABASE"),"mysdsn")==0)
	{
	}
	*/
	//return 0;
//}

/*========================================*\
    功能 : 显示数据库
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
//int fdbsServiceList(void)
//{
	/*
	if(strcmp(getenv("DATABASE"),"oradsn")==0)
	{
		system("lsnrctl status");
		system("echo -e \"select status from v\\$instance;\" | sqlplus / as SYSDBA");
	}
	else
	if(strcmp(getenv("DATABASE"),"sybdsn")==0)
	{
		system("showserver");
	}
	else
	if(strcmp(getenv("DATABASE"),"db2dsn")==0)
	{
		system("db2gcf -s");
	}
	else
	if(strcmp(getenv("DATABASE"),"mysdsn")==0)
	{
	}
	*/
	//return 0;
//}

/*========================================*\
    功能 : 创建表格
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsTableCreate(void)
{
	int result;

	result=fdbsExam();
	if(result==-1)
		return -1;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return -1;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(vdbsBsnCode,"all")!=0&&strcmp(vdbsBsnCode,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vdbsBsnCode,bsndirent[i]->d_name);
		result=fdbsInit(vdbsBsnCode);
		if(result==-1)
			return -1;

		char sqlpath[64];
		sprintf(sqlpath,"%s/%s/dbs/dbs.sql",getenv("BUSINESS"),vdbsBsnCode);
		FILE *sqlfp;
		sqlfp=fopen(sqlpath,"r");
		if(sqlfp==NULL)
		{
			mlogError("fopen",errno,strerror(errno),"[%s]",sqlpath);
			fdbsFree();
			return -1;
		}

		char text[2048];
		int i=0;
		int matchcount=0;
		int notepos;

		while(1)
		{
			text[i]=fgetc(sqlfp);
			if(ferror(sqlfp))
			{
				mlogError("fgets",errno,strerror(errno),"[]");
				return -1;
			}
			if(feof(sqlfp))
				break;

			switch(text[i])
			{
				case '*':
				if(text[i-1]=='/')
					notepos=i-1;
				break;
				case '/':
				if(text[i-1]=='*')
					i=notepos-1;
				break;

				case '(':
				matchcount++;
				break;
				case ')':
				matchcount--;
				break;

				case ' ':
				case '\t':
				case '\v':
				case '\n':
				case '\r':
				case '\f':
				if(i==0||text[i-1]==' '||text[i-1]=='('||text[i-1]==')')
					continue;
				else
					text[i]=' ';
			}

			if(text[i++]!=')'||matchcount!=0)
				continue;

			text[i]='\0';
			i=0;

			char *position1;
			char *position2;
			position1=strcasestr(text,"create table ")+13;
			position2=strchr(position1,' ');
			char table[32];
			strncpy(table,position1,position2-position1);
			table[position2-position1]='\0';

			if(strcasecmp(vdbsTable,"all")!=0&&strcasestr(table,vdbsTable)==NULL)
				continue;
			printf("创建表格[%s]\n",table);
			fdbsManage(text);
		}

		fclose(sqlfp);
		fdbsFree();
	}

	return 0;
}

/*========================================*\
    功能 : 删除表格
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsTableRemove(void)
{
	int result;

	result=fdbsExam();
	if(result==-1)
		return -1;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return -1;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(vdbsBsnCode,"all")!=0&&strcmp(vdbsBsnCode,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vdbsBsnCode,bsndirent[i]->d_name);
		result=fdbsInit(vdbsBsnCode);
		if(result==-1)
			return -1;

		char type[3+1];
		bzero(type,sizeof(type));

		char inipath[64];
		sprintf(inipath,"%s/%s/dbs/dbs.ini",getenv("BUSINESS"),vdbsBsnCode);
		FILE *inifp;
		inifp=fopen(inipath,"r");
		if(inifp==NULL)
		{
			mlogError("fopen",errno,strerror(errno),"[%s]",inipath);
			return -1;
		}

		while(1)
		{
			char iniline[128];
			fgets(iniline,sizeof(iniline),inifp);
			if(ferror(inifp))
			{
				mlogError("fgets",errno,strerror(errno),"[]");
				return -1;
			}
			if(feof(inifp))
				break;

			if(strncasecmp(iniline,"type",4)==0)
				strncpy(type,iniline+5,3);
		}

		fclose(inifp);

		void *trastm;
		if(strcmp(type,"ora")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[0]);
		else
		if(strcmp(type,"syb")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[1]);
		else
		if(strcmp(type,"db2")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[2]);
		else
		if(strcmp(type,"mys")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[3],vdbsBsnCode);
		else
		if(strcmp(type,"mss")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[4]);
		else
			continue;
		if(result==-1)
			return -1;

		char table[64];
		mdbsSelectBindStr(trastm,1,table,sizeof(table));

		while(1)
		{
			result=fdbsSelectGain(trastm);
			if(result==-1)
				return -1;
			if(result==-100)
				break;

			if(strcasecmp(vdbsTable,"all")!=0&&strcasestr(table,vdbsTable)==NULL)
				continue;
			printf("删除表格[%s]\n",table);
			fdbsManage("drop table %s",table);
		}

		fdbsSelectFree(trastm);
		fdbsFree();
	}

	return 0;
}

/*========================================*\
    功能 : 删除数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataDelete(void)
{
	int result;

	result=fdbsExam();
	if(result==-1)
		return -1;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return -1;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(vdbsBsnCode,"all")!=0&&strcmp(vdbsBsnCode,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vdbsBsnCode,bsndirent[i]->d_name);
		result=fdbsInit(vdbsBsnCode);
		if(result==-1)
			return -1;

		char type[3+1];
		bzero(type,sizeof(type));

		char inipath[64];
		sprintf(inipath,"%s/%s/dbs/dbs.ini",getenv("BUSINESS"),vdbsBsnCode);
		FILE *inifp;
		inifp=fopen(inipath,"r");
		if(inifp==NULL)
		{
			mlogError("fopen",errno,strerror(errno),"[%s]",inipath);
			return -1;
		}

		while(1)
		{
			char iniline[128];
			fgets(iniline,sizeof(iniline),inifp);
			if(ferror(inifp))
			{
				mlogError("fgets",errno,strerror(errno),"[]");
				return -1;
			}
			if(feof(inifp))
				break;

			if(strncasecmp(iniline,"type",4)==0)
				strncpy(type,iniline+5,3);
		}

		fclose(inifp);

		void *trastm;
		if(strcmp(type,"ora")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[0]);
		else
		if(strcmp(type,"syb")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[1]);
		else
		if(strcmp(type,"db2")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[2]);
		else
		if(strcmp(type,"mys")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[3],vdbsBsnCode);
		else
		if(strcmp(type,"mss")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[4]);
		else
			continue;
		if(result==-1)
			return -1;

		char table[64];
		mdbsSelectBindStr(trastm,1,table,sizeof(table));

		while(1)
		{
			result=fdbsSelectGain(trastm);
			if(result==-1)
				return -1;
			if(result==-100)
				break;

			if(strcasecmp(vdbsTable,"all")!=0&&strcasestr(table,vdbsTable)==NULL)
				continue;
			printf("删除数据[%s]\n",table);
			fdbsManage("delete from %s",table);
		}

		fdbsSelectFree(trastm);
		fdbsFree();
	}

	return 0;
}

/*========================================*\
    功能 : 导入数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataImport(void)
{
	int result;

	result=fdbsExam();
	if(result==-1)
		return -1;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return -1;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(vdbsBsnCode,"all")!=0&&strcmp(vdbsBsnCode,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vdbsBsnCode,bsndirent[i]->d_name);
		result=fdbsInit(vdbsBsnCode);
		if(result==-1)
			return -1;

		char type[3+1];
		bzero(type,sizeof(type));

		char inipath[64];
		sprintf(inipath,"%s/%s/dbs/dbs.ini",getenv("BUSINESS"),vdbsBsnCode);
		FILE *inifp;
		inifp=fopen(inipath,"r");
		if(inifp==NULL)
		{
			mlogError("fopen",errno,strerror(errno),"[%s]",inipath);
			return -1;
		}

		while(1)
		{
			char iniline[128];
			fgets(iniline,sizeof(iniline),inifp);
			if(ferror(inifp))
			{
				mlogError("fgets",errno,strerror(errno),"[]");
				return -1;
			}
			if(feof(inifp))
				break;

			if(strncasecmp(iniline,"type",4)==0)
				strncpy(type,iniline+5,3);
		}

		fclose(inifp);

		void *trastm;
		if(strcmp(type,"ora")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[0]);
		else
		if(strcmp(type,"syb")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[1]);
		else
		if(strcmp(type,"db2")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[2]);
		else
		if(strcmp(type,"mys")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[3],vdbsBsnCode);
		else
		if(strcmp(type,"mss")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[4]);
		else
			continue;
		if(result==-1)
			return -1;

		char table[64];
		mdbsSelectBindStr(trastm,1,table,sizeof(table));

		while(1)
		{
			result=fdbsSelectGain(trastm);
			if(result==-1)
				return -1;
			if(result==-100)
				break;

			if(strcasecmp(vdbsTable,"all")!=0&&strcasestr(table,vdbsTable)==NULL)
				continue;
			printf("导入数据[%s]\n",table);

			int info[100];
			int count=0;

			char sqlpath[64];
			sprintf(sqlpath,"%s/%s/dbs/dbs.sql",getenv("BUSINESS"),vdbsBsnCode);
			FILE *sqlfp;
			sqlfp=fopen(sqlpath,"r");
			if(sqlfp==NULL)
			{
				mlogError("fopen",errno,strerror(errno),"[%s]",sqlpath);
				return -1;
			}

			while(1)
			{
				char sqlline[2048];
				fgets(sqlline,sizeof(sqlline),sqlfp);
				if(ferror(sqlfp))
				{
					mlogError("fgets",errno,strerror(errno),"[]");
					return -1;
				}
				if(feof(sqlfp))
					break;

				if(strcasestr(sqlline,"create table")==NULL)
					continue;
				char *p;
				if((p=strstr(sqlline,table))==NULL||!isspace(p[strlen(table)]))
					continue;

				while(1)
				{
					fgets(sqlline,sizeof(sqlline),sqlfp);
					if(ferror(sqlfp))
					{
						mlogError("fgets",errno,strerror(errno),"[]");
						return -1;
					}
					if(feof(sqlfp))
						break;

					if(sqlline[0]=='(')
						continue;
					if(sqlline[0]==')')
						break;

					if(strcasestr(sqlline,"integer")!=NULL)
						info[count++]=0;
					else
					if(strcasestr(sqlline,"decimal")!=NULL)
						info[count++]=0;
					else
					if(strcasestr(sqlline,"char")!=NULL)
						info[count++]=1;
				}
				break;
			}

			fclose(sqlfp);

			char txtpath[64];
			sprintf(txtpath,"%s/%s/dbs/%s.txt",getenv("BUSINESS"),vdbsBsnCode,table);
			FILE *txtfp;
			txtfp=fopen(txtpath,"r");
			if(txtfp==NULL)
			{
				mlogError("fopen",errno,strerror(errno),"[%s]",txtpath);
				return -1;
			}

			while(1)
			{
				char txtline[2048];
				fgets(txtline,sizeof(txtline),txtfp);
				if(ferror(txtfp))
				{
					mlogError("fgets",errno,strerror(errno),"[]");
					return -1;
				}
				if(feof(txtfp))
					break;

				char command[2048];
				sprintf(command,"insert into %s values (",table);
				char *current;
				current=command+strlen(command);

				char *position1;
				char *position2;
				position1=txtline;

				int j;
				for(j=0;j<count;j++)
				{
					position2=strstr(position1,"|*|");
					while(position2>position1)
					{
						if(*(position2-1)==' ')
							position2--;
						else
							break;
					}
					if(info[j]==1)
						*current++='\'';
					strncpy(current,position1,position2-position1);
					current+=position2-position1;
					if(info[j]==1)
						*current++='\'';
					*current++=',';
					position2=strstr(position1,"|*|");
					position1=position2+3;
				}

				strcpy(current-1,")");
				result=fdbsManage(command);
				if(result==-1)
					break;
			}

			fclose(txtfp);
		}

		fdbsSelectFree(trastm);
		fdbsFree();
	}

	return 0;
}

/*========================================*\
    功能 : 导出数据
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsDataExport(void)
{
	int result;

	result=fdbsExam();
	if(result==-1)
		return -1;

	char bsnpath[64];
	strcpy(bsnpath,getenv("BUSINESS"));
	struct dirent **bsndirent;
	int count;
	count=scandir(bsnpath,&bsndirent,NULL,alphasort);
	if(count==-1)
	{
		mlogError("scandir",errno,strerror(errno),"[%s]",bsnpath);
		return -1;
	}
	int i;
	for(i=0;i<count;i++)
	{
		if(bsndirent[i]->d_type!=DT_DIR)
			continue;
		if(bsndirent[i]->d_name[0]=='.')
			continue;
		if(strcasecmp(vdbsBsnCode,"all")!=0&&strcmp(vdbsBsnCode,bsndirent[i]->d_name)!=0)
			continue;

		strcpy(vdbsBsnCode,bsndirent[i]->d_name);
		result=fdbsInit(vdbsBsnCode);
		if(result==-1)
			return -1;

		char type[3+1];
		bzero(type,sizeof(type));

		char inipath[64];
		sprintf(inipath,"%s/%s/dbs/dbs.ini",getenv("BUSINESS"),vdbsBsnCode);
		FILE *inifp;
		inifp=fopen(inipath,"r");
		if(inifp==NULL)
		{
			mlogError("fopen",errno,strerror(errno),"[%s]",inipath);
			return -1;
		}

		while(1)
		{
			char iniline[128];
			fgets(iniline,sizeof(iniline),inifp);
			if(ferror(inifp))
			{
				mlogError("fgets",errno,strerror(errno),"[]");
				return -1;
			}
			if(feof(inifp))
				break;

			if(strncasecmp(iniline,"type",4)==0)
				strncpy(type,iniline+5,3);
		}

		fclose(inifp);

		void *trastm;
		if(strcmp(type,"ora")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[0]);
		else
		if(strcmp(type,"syb")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[1]);
		else
		if(strcmp(type,"db2")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[2]);
		else
		if(strcmp(type,"mys")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[3],vdbsBsnCode);
		else
		if(strcmp(type,"mss")==0)
			result=fdbsSelectInit(&trastm,vdbsTraverseSQL[4]);
		else
			continue;
		if(result==-1)
			return -1;

		char table[64];
		mdbsSelectBindStr(trastm,1,table,sizeof(table));

		while(1)
		{
			result=fdbsSelectGain(trastm);
			if(result==-1)
				return -1;
			if(result==-100)
				break;

			if(strcasecmp(vdbsTable,"all")!=0&&strcasestr(table,vdbsTable)==NULL)
				continue;
			printf("导出数据[%s]\n",table);

			char sqlpath[64];
			sprintf(sqlpath,"%s/%s/dbs/dbs.sql",getenv("BUSINESS"),vdbsBsnCode);
			FILE *sqlfp;
			sqlfp=fopen(sqlpath,"r");
			if(sqlfp==NULL)
			{
				mlogError("fopen",0,"0","[%s]",sqlpath);
				return -1;
			}

			int info[100];
			int count=0;

			while(1)
			{
				char sqlline[2048];
				fgets(sqlline,sizeof(sqlline),sqlfp);
				if(ferror(sqlfp))
				{
					mlogError("fgets",errno,strerror(errno),"[]");
					return -1;
				}
				if(feof(sqlfp))
					break;

				if(strcasestr(sqlline,"create table")==NULL)
					continue;
				char *p;
				if((p=strstr(sqlline,table))==NULL||!isspace(p[strlen(table)]))
					continue;

				while(1)
				{
					fgets(sqlline,sizeof(sqlline),sqlfp);
					if(ferror(sqlfp))
					{
						mlogError("fgets",errno,strerror(errno),"[]");
						return -1;
					}
					if(feof(sqlfp))
						break;

					if(sqlline[0]=='(')
						continue;
					if(sqlline[0]==')')
						break;

					if(strcasestr(sqlline,"integer")!=NULL)
						info[count++]=-1;
					else
					if(strcasestr(sqlline,"decimal")!=NULL)
						info[count++]=-2;
					else
					if(strcasestr(sqlline,"char")!=NULL)
						info[count++]=atoi(strcasestr(sqlline,"char")+5);
				}
				break;
			}

			fclose(sqlfp);

			void *expstm;
			result=fdbsSelectInit(&expstm,"select * from %s",table);
			if(result==-1)
				return -1;

			char txtpath[64];
			sprintf(txtpath,"%s/%s/dbs/%s.txt",getenv("BUSINESS"),vdbsBsnCode,table);
			FILE *txtfp;
			txtfp=fopen(txtpath,"w");
			if(txtfp==NULL)
			{
				mlogError("fopen",errno,strerror(errno),"[%s]",txtpath);
				return -1;
			}

			void **fields=(void**)malloc(sizeof(char*)*count);
			if(fields==NULL)
			{
				mlogError("malloc",0,"0","[%d]",sizeof(char*)*count);
				return -1;
			}

			int j;
			for(j=0;j<count;j++)
			{
				if(info[j]==-1)
				{
					fields[j]=malloc(sizeof(int));
					mdbsSelectBindInt(expstm,j+1,fields[j],0);
				}
				else
				if(info[j]==-2)
				{
					fields[j]=malloc(sizeof(double));
					mdbsSelectBindDbl(expstm,j+1,fields[j],0);
				}
				else
				{
					fields[j]=malloc(info[j]+1);
					mdbsSelectBindStr(expstm,j+1,fields[j],info[j]+1);
				}

				if(fields[j]==NULL)
				{
					mlogError("malloc",0,"0","[]");
					return -1;
				}
			}

			while(1)
			{
				result=fdbsSelectGain(expstm);
				if(result==-1)
					break;
				if(result==-100)
					break;

				for(j=0;j<count;j++)
				{
					if(info[j]==-1)
						fprintf(txtfp,"%10d|*|",*(int*)fields[j]);
					else
					if(info[j]==-2)
						fprintf(txtfp,"%17.2f|*|",fields[j]);
					else
						fprintf(txtfp,"%-*s|*|",info[j],fields[j]);
				}
				fprintf(txtfp,"\n");
			}

			for(j=0;j<count;j++)
				free(fields[j]);
			free(fields);

			fclose(txtfp);

			fdbsSelectFree(expstm);
		}

		fdbsSelectFree(trastm);
		fdbsFree();
	}

	return 0;
}

/*========================================*\
    功能 : 检查参数格式
    参数 : 空
    返回 : (成功)0
           (失败)-1
\*========================================*/
int fdbsExam(void)
{
	char *position1;
	char *position2;

	position1=optarg;
	position2=strchr(position1,'_');
	if(position2==NULL)
	{
		fdbsHelp();
		return -1;
	}
	if(position2-position1!=3)
	{
		fdbsHelp();
		return -1;
	}
	strncpy(vdbsBsnCode,position1,3);

	position1=position2+1;
	if(strlen(position1)==0)
	{
		fdbsHelp();
		return -1;
	}
	strcpy(vdbsTable,position1);

	return 0;
}

/*========================================*\
    功能 : 显示帮助信息
    参数 : 空
    返回 : 空
\*========================================*/
void fdbsHelp(void)
{
	printf("\n");
	printf("-c|--create : 创建表格,参数:业务_表格.\n");
	printf("-r|--remove : 删除表格,参数:业务_表格.\n");
	printf("-d|--delete : 删除数据,参数:业务_表格.\n");
	printf("-i|--import : 导入数据,参数:业务_表格.\n");
	printf("-e|--export : 导出数据,参数:业务_表格.\n");
	printf("\n");
	return;
}
