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

	struct tjsnItem *item1;
	struct tjsnItem *item2;
	struct tjsnItem *item3;

	char data[1024];

	//测试对象节点打包.
	result=fjsnCreate(&item1,cjsnObj,NULL,0,NULL,0);
	if(result!=0)
		return -1;

	result=fjsnCreate(&item2,cjsnStr,"a",1,"1",1);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item2,cjsnStr,"b",1,"2",1);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item2,cjsnObj,"c",1,NULL,0);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item3,cjsnNum,"d",1,"4",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item3,cjsnNum,"e",1,"5.5555",6);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item2,cjsnArr,"f",1,NULL,0);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item3,cjsnStr,NULL,0,"7",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item3,cjsnStr,NULL,0,"8",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnExport(item1,data);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fjsnFree(item1);

	printf("========================================\n");
	
	//测试对象节点解包.
	printf("%s\n",data);
	result=fjsnImport(&item1,data);
	if(result!=0)
		return -1;

	result=fjsnSelect(item1,&item2,"a",1,0);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item2->valsize,item2->valdata);

	result=fjsnSelect(item1,&item2,"b",1,0);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item2->valsize,item2->valdata);

	result=fjsnSelect(item1,&item2,"c",1,0);
	if(result!=0)
		return -1;

	result=fjsnSelect(item2,&item3,"d",1,0);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item2,&item3,"e",1,0);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item1,&item2,"f",1,0);
	if(result!=0)
		return -1;

	result=fjsnSelect(item2,&item3,NULL,0,0);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item2,&item3,NULL,0,1);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",item3->valsize,item3->valdata);

	fjsnFree(item1);

	printf("========================================\n");

	//测试数组节点打包.
	result=fjsnCreate(&item1,cjsnArr,NULL,0,NULL,0);
	if(result!=0)
		return -1;

	result=fjsnCreate(&item2,cjsnObj,NULL,0,NULL,0);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item3,cjsnStr,"a",1,"1",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item3,cjsnStr,"b",1,"2",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item2,cjsnObj,NULL,0,NULL,0);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item3,cjsnStr,"a",1,"3",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item3,cjsnStr,"b",1,"4",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item2,cjsnObj,NULL,0,NULL,0);
	if(result!=0)
		return -1;
	fjsnInsert(item1,item2);

	result=fjsnCreate(&item3,cjsnStr,"a",1,"5",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnCreate(&item3,cjsnStr,"b",1,"6",1);
	if(result!=0)
		return -1;
	fjsnInsert(item2,item3);

	result=fjsnExport(item1,data);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fjsnFree(item1);

	printf("========================================\n");
	
	//测试数组节点解包.
	printf("%s\n",data);
	result=fjsnImport(&item1,data);
	if(result!=0)
		return -1;

	result=fjsnSelect(item1,&item2,NULL,0,0);
	if(result!=0)
		return -1;

	result=fjsnSelect(item2,&item3,"a",1,0);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item2,&item3,"b",1,0);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item1,&item2,NULL,0,1);
	if(result!=0)
		return -1;

	result=fjsnSelect(item2,&item3,"a",1,0);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item2,&item3,"b",1,0);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item1,&item2,NULL,0,2);
	if(result!=0)
		return -1;

	result=fjsnSelect(item2,&item3,"a",1,0);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item3->valsize,item3->valdata);

	result=fjsnSelect(item2,&item3,"b",1,0);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item3->valsize,item3->valdata);

	fjsnFree(item1);

	printf("========================================\n");
	
	return 0;
}
