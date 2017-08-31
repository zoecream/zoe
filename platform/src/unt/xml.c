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

	struct txmlItem *item1;
	struct txmlItem *item2;
	struct txmlItem *item3;

	char data[1024];

	//测试对象节点打包.
	result=fxmlCreate(&item1,cxmlNode,"root",4,NULL,0);
	if(result!=0)
		return -1;

	result=fxmlCreate(&item2,cxmlNode,"a",1,"1",1);
	if(result!=0)
		return -1;
	fxmlInsert(item1,item2,cxmlNode);

	result=fxmlCreate(&item2,cxmlNode,"b",1,"2",1);
	if(result!=0)
		return -1;
	fxmlInsert(item1,item2,cxmlNode);

	result=fxmlCreate(&item2,cxmlNode,"c",1,NULL,0);
	if(result!=0)
		return -1;
	fxmlInsert(item1,item2,cxmlNode);

	result=fxmlCreate(&item3,cxmlAttr,"d",1,"4",1);
	if(result!=0)
		return -1;
	fxmlInsert(item2,item3,cxmlAttr);

	result=fxmlCreate(&item3,cxmlAttr,"e",1,"5.5555",6);
	if(result!=0)
		return -1;
	fxmlInsert(item2,item3,cxmlAttr);

	result=fxmlCreate(&item2,cxmlNode,"f",1,"7",1);
	if(result!=0)
		return -1;
	fxmlInsert(item1,item2,cxmlNode);

	result=fxmlCreate(&item2,cxmlNode,"f",1,"8",1);
	if(result!=0)
		return -1;
	fxmlInsert(item1,item2,cxmlNode);

	result=fxmlExport(item1,data);
	if(result!=0)
		return -1;
	printf("%s\n",data);

	fxmlFree(item1);

	printf("========================================\n");
	
	//测试对象节点解包.
	printf("%s\n",data);
	result=fxmlImport(&item1,data);
	if(result!=0)
		return -1;

	result=fxmlSelect(item1,&item2,cxmlNode,"a",1,0);
	if(result!=0)
		return -1;
	printf("a:%.*s\n",item2->valsize,item2->valdata);

	result=fxmlSelect(item1,&item2,cxmlNode,"b",1,0);
	if(result!=0)
		return -1;
	printf("b:%.*s\n",item2->valsize,item2->valdata);

	result=fxmlSelect(item1,&item2,cxmlNode,"c",1,0);
	if(result!=0)
		return -1;

	result=fxmlSelect(item2,&item3,cxmlAttr,"d",1,0);
	if(result!=0)
		return -1;
	printf("d:%.*s\n",item3->valsize,item3->valdata);

	result=fxmlSelect(item2,&item3,cxmlAttr,"e",1,0);
	if(result!=0)
		return -1;
	printf("e:%.*s\n",item3->valsize,item3->valdata);

	result=fxmlSelect(item1,&item2,cxmlNode,"f",1,0);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",item2->valsize,item2->valdata);

	result=fxmlSelect(item1,&item2,cxmlNode,"f",1,1);
	if(result!=0)
		return -1;
	printf("f:%.*s\n",item2->valsize,item2->valdata);

	fxmlFree(item1);

	printf("========================================\n");

	return 0;
}
