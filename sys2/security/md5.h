/**
 * @file md5.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#ifndef __MD5_H__
#define __MD5_H__


#ifdef __cplusplus
extern "C"
{
#endif


#define A	0x67452301
#define B	0xEFCDAB89
#define C	0x98BADCFE
#define D	0x10325476

	typedef union _MD5
	{
		unsigned int iv[4];
		unsigned char bv[16];
	}MD5,*LPMD5;

	typedef const MD5* LPCMD5;

	typedef struct _MD5_CYCLE
	{
		unsigned int m[16];
		unsigned int nOddOffset;
	}MD5_CYCLE,*LP_MD5_CYCLE;

	unsigned long long md5_calc_add_size(unsigned long long nSize);
	int md5_fill(MD5_CYCLE* mc,const unsigned char* pData,unsigned long long nSize,unsigned long long* nOffset);
	void md5_cycle(const MD5_CYCLE* mc,unsigned int* a,unsigned int* b,unsigned int* c,unsigned int* d);
	int md5(const unsigned char* pData,unsigned long long nSize,LPMD5 lpMd5);
	int md5_string(LPCMD5 lpMd5,char* szMd5,unsigned int nSize);



#ifdef __cplusplus
}
#endif

#endif//__MD5_H__

