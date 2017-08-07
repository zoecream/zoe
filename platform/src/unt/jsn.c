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

	char pd[1024];
	int pl;
	char *move;

	//测试对象头节点打包.
	result=fjsnObjCreate(&item1);
	if(result!=0)
		return -1;

	result=fjsnStrCreate(&item2,"1",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item1,item2,"a",1);

	result=fjsnStrCreate(&item2,"2",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item1,item2,"b",1);

	result=fjsnObjCreate(&item2);
	if(result!=0)
		return -1;
	fjsnObjInsert(item1,item2,"c",1);

	result=fjsnNumCreate(&item3,"4",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"d",1);

	result=fjsnNumCreate(&item3,"5.5555",6);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"e",1);

	result=fjsnArrCreate(&item2);
	if(result!=0)
		return -1;
	fjsnObjInsert(item1,item2,"f",1);

	result=fjsnStrCreate(&item3,"7",1);
	if(result!=0)
		return -1;
	fjsnArrInsert(item2,item3);

	result=fjsnStrCreate(&item3,"8",1);
	if(result!=0)
		return -1;
	fjsnArrInsert(item2,item3);

	move=pd;
	result=fjsnExport(item1,&move,&pl);
	if(result!=0)
		return -1;
	printf("%.*s\n",pl,pd);

	fjsnFree(item1);

	printf("========================================\n");
	
	//测试对象头节点解包.
	result=fjsnObjCreate(&item1);
	if(result!=0)
		return -1;

	printf("%.*s\n",pl,pd);

	move=pd;
	result=fjsnImport(item1,&move,pl);
	if(result!=0)
		return -1;

	result=fjsnObjSelect(item1,&item2,"a",1);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item2->vall,item2->vald);

	result=fjsnObjSelect(item1,&item2,"b",1);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item2->vall,item2->vald);

	result=fjsnObjSelect(item1,&item2,"c",1);
	if(result!=0)
		return -1;

	result=fjsnObjSelect(item2,&item3,"d",1);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",item3->vall,item3->vald);

	result=fjsnObjSelect(item2,&item3,"e",1);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",item3->vall,item3->vald);

	result=fjsnObjSelect(item1,&item2,"f",1);
	if(result!=0)
		return -1;

	result=fjsnArrSelect(item2,&item3,0);
	if(result!=0)
		return -1;
	printf("g:%.*s\n",item3->vall,item3->vald);

	result=fjsnArrSelect(item2,&item3,1);
	if(result!=0)
		return -1;
	printf("h:%.*s\n",item3->vall,item3->vald);

	fjsnFree(item1);

	printf("========================================\n");

	//测试数组头节点打包.
	result=fjsnArrCreate(&item1);
	if(result!=0)
		return -1;

	result=fjsnObjCreate(&item2);
	if(result!=0)
		return -1;
	fjsnArrInsert(item1,item2);

	result=fjsnStrCreate(&item3,"1",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"a",1);

	result=fjsnStrCreate(&item3,"2",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"b",1);

	result=fjsnObjCreate(&item2);
	if(result!=0)
		return -1;
	fjsnArrInsert(item1,item2);

	result=fjsnStrCreate(&item3,"3",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"a",1);

	result=fjsnStrCreate(&item3,"4",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"b",1);

	result=fjsnObjCreate(&item2);
	if(result!=0)
		return -1;
	fjsnArrInsert(item1,item2);

	result=fjsnStrCreate(&item3,"5",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"a",1);

	result=fjsnStrCreate(&item3,"6",1);
	if(result!=0)
		return -1;
	fjsnObjInsert(item2,item3,"b",1);

	move=pd;
	result=fjsnExport(item1,&move,&pl);
	if(result!=0)
		return -1;
	printf("%.*s\n",pl,pd);

	fjsnFree(item1);

	printf("========================================\n");
	
	//测试数组头节点解包.
	result=fjsnObjCreate(&item1);
	if(result!=0)
		return -1;

	printf("%.*s\n",pl,pd);

	move=pd;
	result=fjsnImport(item1,&move,pl);
	if(result!=0)
		return -1;

	result=fjsnArrSelect(item1,&item2,0);
	if(result!=0)
		return -1;

	result=fjsnObjSelect(item2,&item3,"a",1);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item3->vall,item3->vald);

	result=fjsnObjSelect(item2,&item3,"b",1);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item3->vall,item3->vald);

	result=fjsnArrSelect(item1,&item2,1);
	if(result!=0)
		return -1;

	result=fjsnObjSelect(item2,&item3,"a",1);
	if(result!=0)
		return -1;
	printf("c:%.*s\n",item3->vall,item3->vald);

	result=fjsnObjSelect(item2,&item3,"b",1);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",item3->vall,item3->vald);

	result=fjsnArrSelect(item1,&item2,2);
	if(result!=0)
		return -1;

	result=fjsnObjSelect(item2,&item3,"a",1);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",item3->vall,item3->vald);

	result=fjsnObjSelect(item2,&item3,"b",1);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",item3->vall,item3->vald);

	fjsnFree(item1);

	printf("========================================\n");
	
	return 0;
}
