/*========================================*\
    文件 : ini.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <ini.h>

int main(void)
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	int size;
	char a[1024];
	result=miniGetStr("000","001","a",a,&size);
	if(result==-1)
	{
		fprintf(stderr,"miniGetStr failed!\n");
		return -1;
	}
	printf("a:[%d][%s]\n",size,a);

	char b[1024];
	result=miniGetStr("000","002","b",b,&size);
	if(result==-1)
	{
		fprintf(stderr,"miniGetStr failed!\n");
		return -1;
	}
	printf("b:[%d][%s]\n",size,b);
	if(isupper(b[0]))
		b[0]=tolower(b[0]);
	else
	if(islower(b[0]))
		b[0]=toupper(b[0]);
	result=miniSetStr("000","002","b",b,size);
	if(result==-1)
	{
		fprintf(stderr,"miniSetStr failed!\n");
		return -1;
	}

	char c1;
	c1=8;
	result=miniSetChr("000","003","c",&c1,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c1=0;
	result=miniGetChr("000","003","c",&c1,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%d\n",c1);

	short c2;
	c2=8;
	result=miniSetSht("000","003","c",&c2,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c2=0;
	result=miniGetSht("000","003","c",&c2,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%d\n",c2);

	int c3;
	c3=8;
	result=miniSetInt("000","003","c",&c3,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c3=0;
	result=miniGetInt("000","003","c",&c3,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%d\n",c3);

	long c4;
	c4=8;
	result=miniSetLng("000","003","c",&c4,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c4=0;
	result=miniGetLng("000","003","c",&c4,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%d\n",c4);

	float c5;
	c5=8.0;
	result=miniSetFlt("000","003","c",&c5,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c5=0.0;
	result=miniGetFlt("000","003","c",&c5,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%.2f\n",c5);

	double c6;
	c6=8.0;
	result=miniSetDbl("000","003","c",&c6,0);
	if(result==-1)
	{
		fprintf(stderr,"miniSetInt failed!\n");
		return -1;
	}
	c6=0.0;
	result=miniGetDbl("000","003","c",&c6,0);
	if(result==-1)
	{
		fprintf(stderr,"miniGetInt failed!\n");
		return -1;
	}
	printf("%.2f\n",c6);

	return 0;
}
