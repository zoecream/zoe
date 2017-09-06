/*========================================*\
    文件 : xml.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <xml.h>

int main(void)
{
	int result;

	struct txmlItem *item;

	char data[1024];
	int size;

	//测试对象节点打包.
	item=NULL;
	result=fxmlCreate(&item,"/root/a","1",1);
	if(result!=0)
		return -1;
	result=fxmlCreate(&item,"/root/b","2",1);
	if(result!=0)
		return -1;
	result=fxmlCreate(&item,"/root/c#d","4",1);
	if(result!=0)
		return -1;
	result=fxmlCreate(&item,"/root/c#e","5.5555",6);
	if(result!=0)
		return -1;
	result=fxmlCreate(&item,"/root/f:0","7",1);
	if(result!=0)
		return -1;
	result=fxmlCreate(&item,"/root/f:1","8",1);
	if(result!=0)
		return -1;

	result=fxmlExport(item,data,&size);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fxmlFree(&item);

	printf("========================================\n");
	
	//测试对象节点解包.
	printf("%s\n",data);
	result=fxmlImport(&item,data,size);
	if(result!=0)
		return -1;

	result=fxmlSelect(item,"/root/a",data,&size);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",size,data);
	result=fxmlSelect(item,"/root/b",data,&size);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",size,data);
	result=fxmlSelect(item,"/root/c#d",data,&size);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",size,data);
	result=fxmlSelect(item,"/root/c#e",data,&size);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",size,data);
	result=fxmlSelect(item,"/root/f:0",data,&size);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",size,data);
	result=fxmlSelect(item,"/root/f:1",data,&size);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",size,data);

	fxmlFree(&item);

	printf("========================================\n");

	return 0;
}
