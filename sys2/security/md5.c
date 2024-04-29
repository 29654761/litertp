/**
 * @file md5.c
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include <stdio.h>
#include <memory.h>
#include <assert.h>
#include "md5.h"


#define F(x,y,z)	((x&y)|((~x)&z))
#define G(x,y,z)	((x&z)|(y&(~z)))
#define H(x,y,z)	(x^y^z)
#define I(x,y,z)	(y^(x|(~z)))

#define ROTATE_LEFT(x, n) ((x<<n)|(x>>(32-n)))

static unsigned char add_data[64] = {0x80}; 


#define FF(a,b,c,d,mi,s,ti)	a=a+F(b,c,d)+mi+ti;a=b+ROTATE_LEFT(a,s)
#define GG(a,b,c,d,mi,s,ti) a=a+G(b,c,d)+mi+ti;a=b+ROTATE_LEFT(a,s)
#define HH(a,b,c,d,mi,s,ti) a=a+H(b,c,d)+mi+ti;a=b+ROTATE_LEFT(a,s)
#define II(a,b,c,d,mi,s,ti) a=a+I(b,c,d)+mi+ti;a=b+ROTATE_LEFT(a,s)



unsigned long long md5_calc_add_size(unsigned long long nSize)
{
	unsigned long long nRemainder=nSize%64,nMd5SizeNeed=0;

	if(nRemainder<56)
	{
		nMd5SizeNeed=56-nRemainder;
	}
	else if(nRemainder>56)
	{
		if(nRemainder==0)
			return 56;
		else
			nMd5SizeNeed=120-nRemainder;
	}
	else
	{
		return 0;
	}

	return nMd5SizeNeed;
}

int md5_fill(MD5_CYCLE* mc,const unsigned char* pData,unsigned long long nSize,unsigned long long* nOffset)
{
	unsigned long long nAddSize=0,nFillSize=0;
	int iLeaveSize=0;
	unsigned long long nBitSize=0;

	pData+=*nOffset;
	if(*nOffset<nSize)
	{
		nFillSize=nSize-(*nOffset);
		if(nFillSize>=64)
		{
			memcpy((unsigned char*)mc,pData,64);
			*nOffset+=64;
			mc->nOddOffset=0;
			return 1;
		}
		else
		{
			nAddSize=md5_calc_add_size(nFillSize);
			memcpy((unsigned char*)mc,pData,(size_t)nFillSize);
			*nOffset+=nFillSize;

			iLeaveSize=64-(int)nFillSize;
			if(((int)nAddSize<=iLeaveSize-8)&&nAddSize!=0)
			{
				memcpy((unsigned char*)mc+nFillSize,add_data,(unsigned int)nAddSize);
				nBitSize=nSize*8;
				memcpy((unsigned char*)mc+nFillSize+nAddSize,&nBitSize,8);
				*nOffset+=64;
				return 0;
			}
			else
			{
				memcpy((unsigned char*)mc+nFillSize,add_data,iLeaveSize);
				mc->nOddOffset=iLeaveSize;
				*nOffset+=iLeaveSize;
				return 1;
			}
		}
	}
	else
	{
		memcpy((unsigned char*)mc,add_data+mc->nOddOffset,56);
		nBitSize=nSize*8;
		memcpy((unsigned char*)mc+56,&nBitSize,8);
		*nOffset+=64;
		return 0;
	}

}

void md5_cycle(const MD5_CYCLE* mc,unsigned int* a,unsigned int* b,unsigned int* c,unsigned int* d)
{
	unsigned int a1=*a,b1=*b,c1=*c,d1=*d;
	FF(a1,b1,c1,d1,mc->m[0],7,0xd76aa478);
	FF(d1,a1,b1,c1,mc->m[1],12,0xe8c7b756);
	FF(c1,d1,a1,b1,mc->m[2],17,0x242070db);
	FF(b1,c1,d1,a1,mc->m[3],22,0xc1bdceee);
	FF(a1,b1,c1,d1,mc->m[4],7,0xf57c0faf);
	FF(d1,a1,b1,c1,mc->m[5],12,0x4787c62a);
	FF(c1,d1,a1,b1,mc->m[6],17,0xa8304613);
	FF(b1,c1,d1,a1,mc->m[7],22,0xfd469501);
	FF(a1,b1,c1,d1,mc->m[8],7,0x698098d8);
	FF(d1,a1,b1,c1,mc->m[9],12,0x8b44f7af);
	FF(c1,d1,a1,b1,mc->m[10],17,0xffff5bb1);
	FF(b1,c1,d1,a1,mc->m[11],22,0x895cd7be);
	FF(a1,b1,c1,d1,mc->m[12],7,0x6b901122);
	FF(d1,a1,b1,c1,mc->m[13],12,0xfd987193);
	FF(c1,d1,a1,b1,mc->m[14],17,0xa679438e);
	FF(b1,c1,d1,a1,mc->m[15],22,0x49b40821);

	GG(a1,b1,c1,d1,mc->m[1],5,0xf61e2562);
	GG(d1,a1,b1,c1,mc->m[6],9,0xc040b340);
	GG(c1,d1,a1,b1,mc->m[11],14,0x265e5a51);
	GG(b1,c1,d1,a1,mc->m[0],20,0xe9b6c7aa);
	GG(a1,b1,c1,d1,mc->m[5],5,0xd62f105d);
	GG(d1,a1,b1,c1,mc->m[10],9,0x02441453);
	GG(c1,d1,a1,b1,mc->m[15],14,0xd8a1e681);
	GG(b1,c1,d1,a1,mc->m[4],20,0xe7d3fbc8);
	GG(a1,b1,c1,d1,mc->m[9],5,0x21e1cde6);
	GG(d1,a1,b1,c1,mc->m[14],9,0xc33707d6);
	GG(c1,d1,a1,b1,mc->m[3],14,0xf4d50d87);
	GG(b1,c1,d1,a1,mc->m[8],20,0x455a14ed);
	GG(a1,b1,c1,d1,mc->m[13],5,0xa9e3e905);
	GG(d1,a1,b1,c1,mc->m[2],9,0xfcefa3f8);
	GG(c1,d1,a1,b1,mc->m[7],14,0x676f02d9);
	GG(b1,c1,d1,a1,mc->m[12],20,0x8d2a4c8a);

	HH(a1,b1,c1,d1,mc->m[5],4,0xfffa3942);
	HH(d1,a1,b1,c1,mc->m[8],11,0x8771f681);
	HH(c1,d1,a1,b1,mc->m[11],16,0x6d9d6122);
	HH(b1,c1,d1,a1,mc->m[14],23,0xfde5380c);
	HH(a1,b1,c1,d1,mc->m[1],4,0xa4beea44);
	HH(d1,a1,b1,c1,mc->m[4],11,0x4bdecfa9);
	HH(c1,d1,a1,b1,mc->m[7],16,0xf6bb4b60);
	HH(b1,c1,d1,a1,mc->m[10],23,0xbebfbc70);
	HH(a1,b1,c1,d1,mc->m[13],4,0x289b7ec6);
	HH(d1,a1,b1,c1,mc->m[0],11,0xeaa127fa);
	HH(c1,d1,a1,b1,mc->m[3],16,0xd4ef3085);
	HH(b1,c1,d1,a1,mc->m[6],23,0x04881d05);
	HH(a1,b1,c1,d1,mc->m[9],4,0xd9d4d039);
	HH(d1,a1,b1,c1,mc->m[12],11,0xe6db99e5);
	HH(c1,d1,a1,b1,mc->m[15],16,0x1fa27cf8);
	HH(b1,c1,d1,a1,mc->m[2],23,0xc4ac5665);

	II(a1,b1,c1,d1,mc->m[0],6,0xf4292244);
	II(d1,a1,b1,c1,mc->m[7],10,0x432aff97);
	II(c1,d1,a1,b1,mc->m[14],15,0xab9423a7);
	II(b1,c1,d1,a1,mc->m[5],21,0xfc93a039);
	II(a1,b1,c1,d1,mc->m[12],6,0x655b59c3);
	II(d1,a1,b1,c1,mc->m[3],10,0x8f0ccc92);
	II(c1,d1,a1,b1,mc->m[10],15,0xffeff47d);
	II(b1,c1,d1,a1,mc->m[1],21,0x85845dd1);
	II(a1,b1,c1,d1,mc->m[8],6,0x6fa87e4f);
	II(d1,a1,b1,c1,mc->m[15],10,0xfe2ce6e0);
	II(c1,d1,a1,b1,mc->m[6],15,0xa3014314);
	II(b1,c1,d1,a1,mc->m[13],21,0x4e0811a1);
	II(a1,b1,c1,d1,mc->m[4],6,0xf7537e82);
	II(d1,a1,b1,c1,mc->m[11],10,0xbd3af235);
	II(c1,d1,a1,b1,mc->m[2],15,0x2ad7d2bb);
	II(b1,c1,d1,a1,mc->m[9],21,0xeb86d391);

	*a+=a1;
	*b+=b1;
	*c+=c1;
	*d+=d1;
}

int md5(const unsigned char* pData,unsigned long long nSize,LPMD5 lpMd5)
{
	unsigned int a=A,b=B,c=C,d=D;
	unsigned long long nOffset=0;
	MD5_CYCLE mc={0};
	while (1)
	{
		int iResult=md5_fill(&mc,pData,nSize,&nOffset);
		md5_cycle(&mc,&a,&b,&c,&d);
		if(iResult==0)
			break;
	}

	lpMd5->iv[0]=a;
	lpMd5->iv[1]=b;
	lpMd5->iv[2]=c;
	lpMd5->iv[3]=d;

	return 0;
}

int md5_string(LPCMD5 lpMd5,char* szMd5,unsigned int nSize)
{
#if defined(_WIN32)||defined(_WIN64)
	return _snprintf_s(szMd5,nSize,nSize-1,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		lpMd5->bv[0],lpMd5->bv[1],lpMd5->bv[2],lpMd5->bv[3],lpMd5->bv[4],lpMd5->bv[5],lpMd5->bv[6],lpMd5->bv[7],
		lpMd5->bv[8],lpMd5->bv[9],lpMd5->bv[10],lpMd5->bv[11],lpMd5->bv[12],lpMd5->bv[13],lpMd5->bv[14],lpMd5->bv[15]);
#else
	return snprintf(szMd5,nSize-1,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		lpMd5->bv[0],lpMd5->bv[1],lpMd5->bv[2],lpMd5->bv[3],lpMd5->bv[4],lpMd5->bv[5],lpMd5->bv[6],lpMd5->bv[7],
		lpMd5->bv[8],lpMd5->bv[9],lpMd5->bv[10],lpMd5->bv[11],lpMd5->bv[12],lpMd5->bv[13],lpMd5->bv[14],lpMd5->bv[15]);
#endif
}



