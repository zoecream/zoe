/*========================================*\
    文件 : jsn.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <jsn.h>

int main(void)
{
	int result;

	char data[1024];
	int size;

	struct tjsnItem *item;

	//测试对象节点打包.
	item=NULL;
	result=fjsnCreate(&item,"/a","1",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,"/b","2",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,"/c/d","4",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,"/c/e","5.5555",6);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,"/f:0","7",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,"/f:1","8",1);
	if(result!=0)
		return -1;

	result=fjsnExport(item,data,&size);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fjsnFree(&item);

	printf("========================================\n");
	
	//测试对象节点解包.
	printf("%s\n",data);
	result=fjsnImport(&item,data,size);
	if(result!=0)
		return -1;

	result=fjsnSelect(item,"/a",data,&size);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",size,data);
	result=fjsnSelect(item,"/b",data,&size);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",size,data);
	result=fjsnSelect(item,"/c/d",data,&size);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",size,data);
	result=fjsnSelect(item,"/c/e",data,&size);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",size,data);
	result=fjsnSelect(item,"/f:0",data,&size);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",size,data);
	result=fjsnSelect(item,"/f:1",data,&size);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",size,data);

	fjsnFree(&item);

	printf("========================================\n");

	//测试数组节点打包.
	item=NULL;
	result=fjsnCreate(&item,":0/a","1",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,":0/b","2",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,":1/a","3",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,":1/b","4",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,":2/a","5",1);
	if(result!=0)
		return -1;
	result=fjsnCreate(&item,":2/b","6",1);
	if(result!=0)
		return -1;

	result=fjsnExport(item,data,&size);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fjsnFree(&item);

	printf("========================================\n");
	
	//测试数组节点解包.
	printf("%s\n",data);
	result=fjsnImport(&item,data,size);
	if(result!=0)
		return -1;

	result=fjsnSelect(item,":0/a",data,&size);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",size,data);
	result=fjsnSelect(item,":0/b",data,&size);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",size,data);
	result=fjsnSelect(item,":1/a",data,&size);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",size,data);
	result=fjsnSelect(item,":1/b",data,&size);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",size,data);
	result=fjsnSelect(item,":2/a",data,&size);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",size,data);
	result=fjsnSelect(item,":2/b",data,&size);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",size,data);

	fjsnFree(&item);

	printf("========================================\n");
	
	return 0;
}
