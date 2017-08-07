/*========================================*\
    文件 : mmp.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mmp.h>

#define cmmpPrintSize 16*12

int main(void)
{
	int result;
	int i;

	result=fmmpInit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpInit failed.\n");
		return -1;
	}
	/*
	printf("fmmpInit:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	printf("========================================\n");

	/*
	result=fmmpRnit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRnit failed.\n");
		return -1;
	}
	printf("fmmpRnit:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");

	char *p;

	//测试分配时使用空闲节点并拆分.
	short index1;
	result=fmmpHeapInit(0,8,&index1);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index1;
	strcpy(p,"AAAAAAA");
	printf("fmmpHeapInit index1:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	short index2;
	result=fmmpHeapInit(0,8,&index2);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index2;
	strcpy(p,"AAAAAAA");
	printf("fmmpHeapInit index2:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	short index3;
	result=fmmpHeapInit(0,8,&index3);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index3;
	strcpy(p,"AAAAAAA");
	printf("fmmpHeapInit index3:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	short index4;
	result=fmmpHeapInit(0,8,&index4);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index4;
	strcpy(p,"AAAAAAA");
	printf("fmmpHeapInit index4:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	short index5;
	result=fmmpHeapInit(0,8,&index5);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index5;
	strcpy(p,"AAAAAAA");
	printf("fmmpHeapInit index5:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");

	//测试重分合并下一空闲节点.
	result=fmmpHeapRnit(0,index5,24,&index5);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapRnit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index5;
	strcpy(p,"AAAAAAAAAAAAAAAAAAAAAAA");
	printf("fmmpHeapRnit index5:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");

	//测试重分调用分配函数.
	result=fmmpHeapRnit(0,index3,24,&index3);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapRnit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index3;
	strcpy(p,"AAAAAAAAAAAAAAAAAAAAAAA");
	printf("fmmpHeapRnit index3:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");

	//测试重分合并下一空闲节点后拆分.
	result=fmmpHeapRnit(0,index2,12,&index2);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapRnit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index2;
	strcpy(p,"AAAAAAAAAAA");
	printf("fmmpHeapRnit index2:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");

	//测试释放后合并上下空闲节点.
	fmmpHeapFree(0,index1);
	printf("fmmpHeapFree index1:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	fmmpHeapFree(0,index2);
	printf("fmmpHeapFree index2:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");
	*/

	/*
	printf("%d\n",vmmpHeapTail-vmmpHead);

	fmmpHeapFree(0,index3);
	printf("fmmpHeapFree index3:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("%d\n",vmmpHeapTail-vmmpHead);

	fmmpHeapFree(0,index4);
	printf("fmmpHeapFree index4:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("%d\n",vmmpHeapTail-vmmpHead);

	fmmpHeapFree(0,index5);
	printf("fmmpHeapFree index5:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("%d\n",vmmpHeapTail-vmmpHead);

	result=fmmpHeapInit(0,4089,&index1);
	if(result==-1)
	{
		fprintf(stderr,"fmmpHeapInit failed.\n");
		return -1;
	}
	p=vmmpHead[0]+index1;
	strcpy(p,"AAAAAAAAAAAAAAAAAAAAAAAA");
	printf("fmmpHeapInit index1:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');

	printf("========================================\n");
	*/

	result=fmmpRnit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRnit failed.\n");
		return -1;
	}
	/*
	printf("fmmpRnit:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	printf("========================================\n");

	int immp;
	immp=10;
	result=fmmpValSet("immp",0,&immp,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	immp=0;
	result=fmmpValGet("immp",0,&immp,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%d\n",immp);

	int *pi;
	result=fmmpRefSet("immp",0,&pi,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRefSet failed.\n");
		return -1;
	}
	*pi=20;
	/*
	printf("fmmpRefSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	pi=NULL;
	result=fmmpRefGet("immp",0,&pi,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRefGet failed.\n");
		return -1;
	}
	printf("fmmpRefGet:%d\n",*pi);

	printf("========================================\n");

	int length;

	char pmmp[16];
	strcpy(pmmp,"hello");
	result=fmmpValSet("pmmp",0,pmmp,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	bzero(pmmp,sizeof(pmmp));
	result=fmmpValGet("pmmp",0,pmmp,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%.*s\n",length,pmmp);

	char *pp;
	result=fmmpRefSet("pmmp",0,&pp,strlen("helloworld"));
	if(result==-1)
	{
		fprintf(stderr,"fmmpRefSet failed.\n");
		return -1;
	}
	strcpy(pp,"helloworld");
	/*
	printf("fmmpRefSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	pp=NULL;
	result=fmmpRefGet("pmmp",0,&pp,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRefGet failed.\n");
		return -1;
	}
	printf("fmmpRefGet:%.*s\n",length,pp);

	printf("========================================\n");

	char _parr[16];

	strcpy(_parr,"1");
	result=fmmpValSet("_parr",0,_parr,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	strcpy(_parr,"12");
	result=fmmpValSet("_parr",1,_parr,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	strcpy(_parr,"21");
	result=fmmpValSet("_parr",8,_parr,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	strcpy(_parr,"2");
	result=fmmpValSet("_parr",9,_parr,0);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValSet failed.\n");
		return -1;
	}
	/*
	printf("fmmpValSet:\n");
	for(i=0;i<cmmpPrintSize;i++)
		printf("%02X%c",(unsigned char)vmmpHead[0][i],(i%16==15)?'\n':' ');
	*/

	bzero(_parr,sizeof(_parr));
	result=fmmpValGet("_parr",0,_parr,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%.*s\n",length,_parr);

	bzero(_parr,sizeof(_parr));
	result=fmmpValGet("_parr",1,_parr,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%.*s\n",length,_parr);

	bzero(_parr,sizeof(_parr));
	result=fmmpValGet("_parr",8,_parr,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%.*s\n",length,_parr);

	bzero(_parr,sizeof(_parr));
	result=fmmpValGet("_parr",9,_parr,&length);
	if(result==-1)
	{
		fprintf(stderr,"fmmpValGet failed.\n");
		return -1;
	}
	printf("fmmpValGet:%.*s\n",length,_parr);

	printf("========================================\n");

	fmmpFree();

	return 0;
}
