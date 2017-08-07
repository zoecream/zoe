/*========================================*\
    文件 : ini.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/stat.h>

#include <log.h>
#include <ini.h>

typedef char   CHR;
typedef short  SHT;
typedef int    INT;
typedef long   LNG;
typedef float  FLT;
typedef double DBL;

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
int finiGet(char *bsncode,char *sec,char *key,void *data,int *size,int type)
{
	int result;

	char path[64];
	sprintf(path,"%s/%s/ini/bsn.ini",getenv("BUSINESS"),bsncode);
	FILE *fp;
	fp=fopen(path,"r");
	if(fp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",path);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
				return -1;
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			position1=position2+1;
			position2=strchr(position1,'\n');
			if(position2==NULL)
			{
				fclose(fp);
				return -1;
			}
			*position2='\0';

			switch(type)
			{
				case 0: strcpy(data,position1); break;
				case 1: *(CHR*)data=atoi(position1); break;
				case 2: *(SHT*)data=atoi(position1); break;
				case 3: *(INT*)data=atoi(position1); break;
				case 4: *(LNG*)data=atol(position1); break;
				case 5: *(FLT*)data=atof(position1); break;
				case 6: *(DBL*)data=atof(position1); break;
			}
			if(size!=NULL)
				*size=strlen(position1);

			break;
		}
	}

	fclose(fp);
	return 0;
}

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
int finiSet(char *bsncode,char *sec,char *key,void *data,int size,int type)
{
	int result;

	char path[64];
	sprintf(path,"%s/%s/ini/bsn.ini",getenv("BUSINESS"),bsncode);
	FILE *fp;
	fp=fopen(path,"r+");
	if(fp==NULL)
	{
		mlogError("fopen",errno,strerror(errno),"[%s]",path);
		return -1;
	}

	int in;
	in=0;

	while(1)
	{
		long offset1;
		offset1=ftell(fp);
		if(offset1==-1)
		{
			mlogError("ftell",errno,strerror(errno),"[]");
			fclose(fp);
			return -1;
		}

		char line[1024];
		fgets(line,sizeof(line),fp);
		if(ferror(fp))
		{
			mlogError("fgets",errno,strerror(errno),"[]");
			fclose(fp);
			return -1;
		}
		if(feof(fp))
			break;

		if(line[0]=='#')
			continue;
		else
		if(line[0]=='\n')
			continue;
		else
		if(line[0]=='[')
		{
			line[strlen(line)-2]='\0';
			if(strcmp(line+1,sec)==0)
				in=1;
			else
			if(in==1)
				return -1;
		}
		else
		{
			if(in==0)
				continue;

			char *position1;
			char *position2;

			position1=line;
			position2=strchr(position1,'=');
			if(position2==NULL)
			{
				fclose(fp);
				return -1;
			}
			*position2='\0';
			if(strcmp(position1,key)!=0)
				continue;

			struct stat status;
			result=fstat(fileno(fp),&status);
			if(result==-1)
			{
				mlogError("fstat",errno,strerror(errno),"[]");
				fclose(fp);
				return -1;
			}

			long offset2;
			offset2=ftell(fp);
			if(offset2==-1)
			{
				mlogError("ftell",errno,strerror(errno),"[]");
				fclose(fp);
				return -1;
			}

			char *text;
			text=(char*)malloc(status.st_size-offset2);
			if(text==NULL)
			{
				mlogError("malloc",errno,strerror(errno),"[%d]",status.st_size-offset2);
				fclose(fp);
				return -1;
			}

			int remain;
			int record;
			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fread(text+record,1,remain,fp);
				if(ferror(fp))
				{
					mlogError("fread",errno,strerror(errno),"[]");
					fclose(fp);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=fseek(fp,offset1,SEEK_SET);
			if(result==-1)
			{
				mlogError("fseek",errno,strerror(errno),"[]");
				fclose(fp);
				return -1;
			}

			switch(type)
			{
				case 0: size=fprintf(fp,"%s=%.*s\n",key,size,data); break;
				case 1: size=fprintf(fp,"%s=%d\n",key,*(CHR*)data); break;
				case 2: size=fprintf(fp,"%s=%d\n",key,*(SHT*)data); break;
				case 3: size=fprintf(fp,"%s=%d\n",key,*(INT*)data); break;
				case 4: size=fprintf(fp,"%s=%d\n",key,*(LNG*)data); break;
				case 5: size=fprintf(fp,"%s=%.2f\n",key,*(FLT*)data); break;
				case 6: size=fprintf(fp,"%s=%.2f\n",key,*(DBL*)data); break;
			}

			remain=status.st_size-offset2;
			record=0;
			while(remain>0)
			{
				result=fwrite(text+record,1,remain,fp);
				if(ferror(fp))
				{
					mlogError("fwrite",errno,strerror(errno),"[]");
					fclose(fp);
					return -1;
				}
				remain-=result;
				record+=result;
			}

			result=ftruncate(fileno(fp),status.st_size-(offset2-offset1)+size);
			if(result==-1)
			{
				mlogError("ftruncate",errno,strerror(errno),"[%d]",status.st_size-(offset2-offset1)+size);
				fclose(fp);
				return -1;
			}

			free(text);

			break;
		}
	}

	fclose(fp);
	return 0;
}
