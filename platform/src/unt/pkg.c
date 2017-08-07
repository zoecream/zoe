/*========================================*\
    文件 : pkg.c
    作者 : 陈乐群
\*========================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pkg.h>
#include <mmp.h>

int main(int argc,char *argv[])
{
	int result;

	result=flogMove(2,"/dev/null");
	if(result==-1)
		return -1;

	result=fmmpInit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpInit failed.\n");
		return -1;
	}
	result=fmmpRnit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRnit failed.\n");
		return -1;
	}

	int i;

	char _fmldata[1024*8];
	char *fmldata=_fmldata;
	char _tmpdata[1024*8];
	char *tmpdata=_tmpdata;
	int size;

	//测试字符编码转换.
	strcpy(fmldata,"中文");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgSetEnc(&fmldata,&tmpdata,&size,"utf-8_gbk_8192");
	if(result==-1)
	{
		fprintf(stderr,"fpkgSetEnc failed!\n");
		return -1;
	}

	printf("enc:[%4d][%s]\n",size,fmldata);

	result=fpkgSetEnc(&fmldata,&tmpdata,&size,"gbk_utf-8_8192");
	if(result==-1)
	{
		fprintf(stderr,"fpkgSetEnc failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	//测试URL转换.
	strcpy(fmldata,"abcd.-_*ABCD!@#$1234 中文");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgUrlEnc(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
	{
		fprintf(stderr,"fpkgUrlEnc failed!\n");
		return -1;
	}

	printf("enc:[%4d][%s]\n",size,fmldata);

	result=fpkgUrlDec(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
	{
		fprintf(stderr,"fpkgUrlDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	//测试十六进制转换.
	strcpy(fmldata,"lmnopqrs");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
	{
		fprintf(stderr,"fpkgHexEnc failed!\n");
		return -1;
	}

	printf("enc:[%4d][%s]\n",size,fmldata);

	size--;
	result=fpkgHexDec(&fmldata,&tmpdata,&size,"upper");
	if(result==-1)
	{
		fprintf(stderr,"fpkgHexDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	//测试BASE64算法.
	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgB64Enc(&fmldata,&tmpdata,&size,NULL);
	if(result==-1)
	{
		fprintf(stderr,"fpkgB64Enc failed!\n");
		return -1;
	}

	printf("enc:[%4d][%s]\n",size,fmldata);

	result=fpkgB64Dec(&fmldata,&tmpdata,&size,NULL);
	if(result==-1)
	{
		fprintf(stderr,"fpkgB64Dec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	//测试信息摘要算法.
	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgDigEnc(&fmldata,&tmpdata,&size,"md5");
	if(result==-1)
	{
		fprintf(stderr,"fpkgDigEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgDigEnc(&fmldata,&tmpdata,&size,"sha224");
	if(result==-1)
	{
		fprintf(stderr,"fpkgDigEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgDigEnc(&fmldata,&tmpdata,&size,"sha256");
	if(result==-1)
	{
		fprintf(stderr,"fpkgDigEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgDigEnc(&fmldata,&tmpdata,&size,"sha384");
	if(result==-1)
	{
		fprintf(stderr,"fpkgDigEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgDigEnc(&fmldata,&tmpdata,&size,"sha512");
	if(result==-1)
	{
		fprintf(stderr,"fpkgDigEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);

	printf("========================================\n");

	//测试对称加密算法.
	result=fmmpValSet("pkey",0,"ABCDEFGH",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefgh",0);
	if(result==-1)
		return -1;

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-ecb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"des-ecb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"des-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-ofb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"des-ofb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	//========================================//

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"des-cfb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"des-cfb_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	result=fmmpValSet("pkey",0,"ABCDEFGHIJKLMNOP",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefghijklmnop",0);
	if(result==-1)
		return -1;

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"aes-128-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"aes-128-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	//========================================//

	result=fmmpValSet("pkey",0,"ABCDEFGHIJKLMNOPQRSTUVWX",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefghijklmnop",0);
	if(result==-1)
		return -1;

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"aes-192-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"aes-192-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	//========================================//

	result=fmmpValSet("pkey",0,"ABCDEFGHIJKLMNOPQRSTUVWXYZ123456",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefghijklmnop",0);
	if(result==-1)
		return -1;

	strcpy(fmldata,"12345678");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgCipEnc(&fmldata,&tmpdata,&size,"aes-256-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgCipDec(&fmldata,&tmpdata,&size,"aes-256-cbc_pkey_pvec_1");
	if(result==-1)
	{
		fprintf(stderr,"fpkgCipDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	//测试RSA算法加密.
	result=fmmpValSet("pekey",0,"65537",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pnkey",0,"18963013433694059164783079915225901655884622091074455623574499651862890701825200425011382648260744403439001417193688265729378093626577075796360717783650574621923665542765757623427651370039267279780525967220566585222487722958100608921326562811541155532567087759322522936341472337052905491942526925060052097265465966812987107117709023325072545173483672712257692750081212398036439582803854982962650736975340774012499668990310100509762249890359881837746510073123517825535020610978031951578715699448412921461825061840155906764672123146449509537860959027200694776575503258977635017886329962721585174129885157203241757058779",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pdkey",0,"9210676795215925940506239092442872186107805250684767515104982836686535365760573302549320813445231643536811459371318212900047731287181512203604872802356789548688299485635457520908200134836427906123466024559389136275612866641191046790181543413619540618130854648498317154753649360960016693963112262416255067918796430747448027433165632779289821088003692693886039972523188537376214437274956683317270579204452466431807523390867599288181809554521377439432147976827371047011202662824985525008447220263052744127327523618242946751794792592767875584842955021309994048910678140937773334205520913732139185573579782780497533715773",0);
	if(result==-1)
		return -1;

	strcpy(fmldata,"11111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111");
	size=strlen(fmldata);
	printf("src:[%4d][%s]\n",size,fmldata);

	result=fpkgRsaEnc(&fmldata,&tmpdata,&size,"pekey_pnkey_pdkey_1_1");
	if(result==-1)
	{
		printf("fpkgRsaEnc failed!\n");
		return -1;
	}

	fpkgHexEnc(&fmldata,&tmpdata,&size,"upper");
	printf("enc:[%4d][%s]\n",size/2,fmldata);
	mpkgSwap(fmldata,tmpdata);
	size/=2;

	result=fpkgRsaDec(&fmldata,&tmpdata,&size,"pekey_pnkey_pdkey_1_1");
	if(result==-1)
	{
		printf("fpkgRsaDec failed!\n");
		return -1;
	}

	printf("dec:[%4d][%s]\n",size,fmldata);

	printf("========================================\n");

	result=fpkgRuleInit("000");
	if(result==-1)
	{
		fprintf(stderr,"fpkgRuleInit failed.\n");
		return -1;
	}

	result=fmmpRnit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRnit failed.\n");
		return -1;
	}

	result=fmmpValSet("pkey",0,"ABCDEFGH",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefgh",0);
	if(result==-1)
		return -1;

	result=fmmpValSet("pekey",0,"65537",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pnkey",0,"18963013433694059164783079915225901655884622091074455623574499651862890701825200425011382648260744403439001417193688265729378093626577075796360717783650574621923665542765757623427651370039267279780525967220566585222487722958100608921326562811541155532567087759322522936341472337052905491942526925060052097265465966812987107117709023325072545173483672712257692750081212398036439582803854982962650736975340774012499668990310100509762249890359881837746510073123517825535020610978031951578715699448412921461825061840155906764672123146449509537860959027200694776575503258977635017886329962721585174129885157203241757058779",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pdkey",0,"9210676795215925940506239092442872186107805250684767515104982836686535365760573302549320813445231643536811459371318212900047731287181512203604872802356789548688299485635457520908200134836427906123466024559389136275612866641191046790181543413619540618130854648498317154753649360960016693963112262416255067918796430747448027433165632779289821088003692693886039972523188537376214437274956683317270579204452466431807523390867599288181809554521377439432147976827371047011202662824985525008447220263052744127327523618242946751794792592767875584842955021309994048910678140937773334205520913732139185573579782780497533715773",0);
	if(result==-1)
		return -1;

	{
		int immp;
		immp=1;
		result=fmmpValSet("immp1",0,&immp,0);
		if(result==-1)
			return -1;
		immp=2;
		result=fmmpValSet("immp2",0,&immp,0);
		if(result==-1)
			return -1;
		immp=3;
		result=fmmpValSet("immp3",0,&immp,0);
		if(result==-1)
			return -1;

		long lmmp;
		lmmp=1001;
		result=fmmpValSet("lmmp1",0,&lmmp,0);
		if(result==-1)
			return -1;
		lmmp=1002;
		result=fmmpValSet("lmmp2",0,&lmmp,0);
		if(result==-1)
			return -1;
		lmmp=1003;
		result=fmmpValSet("lmmp3",0,&lmmp,0);
		if(result==-1)
			return -1;

		double dmmp;
		dmmp=1.0;
		result=fmmpValSet("dmmp1",0,&dmmp,0);
		if(result==-1)
			return -1;
		dmmp=2.0;
		result=fmmpValSet("dmmp2",0,&dmmp,0);
		if(result==-1)
			return -1;
		dmmp=3.0;
		result=fmmpValSet("dmmp3",0,&dmmp,0);
		if(result==-1)
			return -1;

		char pmmp[4];
		result=fmmpValSet("pmmp1",0,"a",0);
		if(result==-1)
			return -1;
		result=fmmpValSet("pmmp2",0,"b",0);
		if(result==-1)
			return -1;
		result=fmmpValSet("pmmp3",0,"c",0);
		if(result==-1)
			return -1;

		int count;
		count=4;
		result=fmmpValSet("icnt",0,&count,0);
		if(result==-1)
			return -1;

		for(i=0;i<count;i++)
		{
			immp=i+1;
			result=fmmpValSet("_immp4",i,&immp,0);
			if(result==-1)
				return -1;
			lmmp=(i+1)*1000;
			result=fmmpValSet("_lmmp4",i,&lmmp,0);
			if(result==-1)
				return -1;
			dmmp=(i+1)*1.1;
			result=fmmpValSet("_dmmp4",i,&dmmp,0);
			if(result==-1)
				return -1;
			sprintf(pmmp,"p%d",i+1);
			result=fmmpValSet("_pmmp4",i,pmmp,0);
			if(result==-1)
				return -1;
		}
	}

	result=fpkgEnc("001","T02",&fmldata,&tmpdata,&size);
	if(result==-1)
	{
		fprintf(stderr,"fpkgEnc failed.\n");
		return -1;
	}

	/*
	printf("========================================\n");
	*/
	printf("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	result=fmmpRnit(4096);
	if(result==-1)
	{
		fprintf(stderr,"fmmpRnit failed.\n");
		return -1;
	}

	result=fmmpValSet("pkey",0,"ABCDEFGH",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pvec",0,"abcdefgh",0);
	if(result==-1)
		return -1;

	result=fmmpValSet("pekey",0,"65537",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pnkey",0,"18963013433694059164783079915225901655884622091074455623574499651862890701825200425011382648260744403439001417193688265729378093626577075796360717783650574621923665542765757623427651370039267279780525967220566585222487722958100608921326562811541155532567087759322522936341472337052905491942526925060052097265465966812987107117709023325072545173483672712257692750081212398036439582803854982962650736975340774012499668990310100509762249890359881837746510073123517825535020610978031951578715699448412921461825061840155906764672123146449509537860959027200694776575503258977635017886329962721585174129885157203241757058779",0);
	if(result==-1)
		return -1;
	result=fmmpValSet("pdkey",0,"9210676795215925940506239092442872186107805250684767515104982836686535365760573302549320813445231643536811459371318212900047731287181512203604872802356789548688299485635457520908200134836427906123466024559389136275612866641191046790181543413619540618130854648498317154753649360960016693963112262416255067918796430747448027433165632779289821088003692693886039972523188537376214437274956683317270579204452466431807523390867599288181809554521377439432147976827371047011202662824985525008447220263052744127327523618242946751794792592767875584842955021309994048910678140937773334205520913732139185573579782780497533715773",0);
	if(result==-1)
		return -1;

	result=fpkgDec("001","T02",&fmldata,&tmpdata,size);
	if(result==-1)
	{
		fprintf(stderr,"fpkgDec failed.\n");
		return -1;
	}

	{
		int *immp;
		result=fmmpRefGet("immp1",0,&immp,0);
		if(result==-1)
			return -1;
		printf("immp1:[%d]\n",*immp);
		result=fmmpRefGet("immp2",0,&immp,0);
		if(result==-1)
			return -1;
		printf("immp2:[%d]\n",*immp);
		result=fmmpRefGet("immp3",0,&immp,0);
		if(result==-1)
			return -1;
		printf("immp3:[%d]\n",*immp);

		long *lmmp;
		result=fmmpRefGet("lmmp1",0,&lmmp,0);
		if(result==-1)
			return -1;
		printf("lmmp1:[%ld]\n",*lmmp);
		result=fmmpRefGet("lmmp2",0,&lmmp,0);
		if(result==-1)
			return -1;
		printf("lmmp2:[%ld]\n",*lmmp);
		result=fmmpRefGet("lmmp3",0,&lmmp,0);
		if(result==-1)
			return -1;
		printf("lmmp3:[%ld]\n",*lmmp);

		double *dmmp;
		result=fmmpRefGet("dmmp1",0,&dmmp,0);
		if(result==-1)
			return -1;
		printf("dmmp1:[%.2f]\n",*dmmp);
		result=fmmpRefGet("dmmp2",0,&dmmp,0);
		if(result==-1)
			return -1;
		printf("dmmp2:[%.2f]\n",*dmmp);
		result=fmmpRefGet("dmmp3",0,&dmmp,0);
		if(result==-1)
			return -1;
		printf("dmmp3:[%.2f]\n",*dmmp);

		char *pmmp;
		result=fmmpRefGet("pmmp1",0,&pmmp,0);
		if(result==-1)
			return -1;
		printf("pmmp1:[%s]\n",pmmp);
		result=fmmpRefGet("pmmp2",0,&pmmp,0);
		if(result==-1)
			return -1;
		printf("pmmp2:[%s]\n",pmmp);
		result=fmmpRefGet("pmmp3",0,&pmmp,0);
		if(result==-1)
			return -1;
		printf("pmmp3:[%s]\n",pmmp);

		int *count;
		result=fmmpRefGet("icnt",0,&count,0);
		if(result==-1)
			return -1;
		printf("count:[%d]\n",*count);

		for(i=0;i<*count;i++)
		{
			result=fmmpRefGet("_immp4",i,&immp,0);
			if(result==-1)
				return -1;
			printf("_immp4_%d:[%d]\n",i,*immp);
			result=fmmpRefGet("_lmmp4",i,&lmmp,0);
			if(result==-1)
				return -1;
			printf("_lmmp4_%d:[%ld]\n",i,*lmmp);
			result=fmmpRefGet("_dmmp4",i,&dmmp,0);
			if(result==-1)
				return -1;
			printf("_dmmp4_%d:[%.2f]\n",i,*dmmp);
			result=fmmpRefGet("_pmmp4",i,&pmmp,0);
			if(result==-1)
				return -1;
			printf("_pmmp4_%d:[%s]\n",i,pmmp);
		}
	}

	/*
	printf("========================================\n");
	*/

	fmmpFree();

	return 0;
}
