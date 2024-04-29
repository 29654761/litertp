#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * This is the standard Gary S. Brown's 32 bit CRC algorithm, but
	 * accumulate the CRC into the result of a previous CRC.
	 */
	unsigned int cyg_crc32_accumulate(unsigned int crc32val,const unsigned char* s, int len);

	/**
	 * This is the standard Gary S. Brown's 32 bit CRC algorithm
	 */
	unsigned int cyg_crc32(const unsigned char* s, int len);

	/**
	 * Return a 32-bit CRC of the contents of the buffer accumulating the
	 * result from a previous CRC calculation. This uses the Ethernet FCS
	 * algorithm.
	 */
	unsigned int cyg_ether_crc32_accumulate(unsigned int crc32val, const unsigned char* s, int len);

	/**
	 * Return a 32-bit CRC of the contents of the buffer, using the
	 * Ethernet FCS algorithm.
	 */
	unsigned int cyg_ether_crc32(const unsigned char* s, int len);

	/**
		* cyg_crc32_in_table - src32_tab[i] = value;  return i
		* @return: return i, or -1 for no such value in tabale
		*/
	static int cyg_crc32_in_table(unsigned int value);

	/**
	 * add 4 bytes num[4] to change crc32 value from crc_src to crc_dst
	 * @return: 0 on success, -1 on error.
	 */
	int cyg_crc32_change(unsigned int crc_dst, unsigned int crc_src, uint8_t num[4]);

	/**
	 * cyg_crc32_reserve - reserve CRC32 value by dropping data[len]
	 * @return: return the CRC value before data[len]
	 */
	unsigned int cyg_crc32_reserve(unsigned int crc, const  void* data, int len);

#ifdef __cplusplus
}
#endif

#endif//__CRC32_H__

