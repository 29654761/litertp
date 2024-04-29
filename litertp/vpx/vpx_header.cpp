/**
 * @file vpx_header.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "vpx_header.h"
#include <string.h>


#define CHECK_SIZE(size1,size2) if (size1 < size2) return -1

int vp8_header_deserialize(vp8_header* hdr, int* header_size, const uint8_t* buffer, int size)
{
	memset((void*)hdr, 0, sizeof(vp8_header));
	CHECK_SIZE(size, 1);


	int pos = 0;
	uint8_t v = buffer[pos++];
	hdr->extended_present = (v & 0x80) >> 7;
	hdr->reserved = (v & 0x40) >> 6;
	hdr->non_reference = (v & 0x20) >> 5;
	hdr->startof_vp8_partition = (v & 0x10) >> 4;
	hdr->part_index = (v & 0x0F);

	*header_size = 1;

	if (hdr->extended_present == 0)
	{
		if (hdr->startof_vp8_partition == 1 && hdr->part_index == 0)
		{
			CHECK_SIZE(size, pos+3);

			v = buffer[pos++];

			hdr->h = (v & 0x10) >> 4;
			hdr->version= (v & 0x0E) >> 1;
			hdr->p= (v & 0x01);

			hdr->first_partition_size = (((uint32_t)(v & 0xE0)) << 11);
			hdr->first_partition_size |= (buffer[pos++] << 8);
			hdr->first_partition_size |= buffer[pos++];
		}
		return pos;
	}
	else
	{
		*header_size = 2;
		CHECK_SIZE(size, pos + 1);

		v = buffer[pos++];
		hdr->pic_idx_present = (v & 0x80) >> 7;
		hdr->tl0_pic_idx_present = (v & 0x40) >> 6;
		hdr->tid_present = (v & 0x20) >> 5;
		hdr->key_idx_present = (v & 0x10) >> 4;
		hdr->reserved2 = (v & 0x0F);


		//=====
		if (hdr->pic_idx_present != 0) 
		{
			CHECK_SIZE(size, pos + 1);
			uint8_t v0 = buffer[pos++];
			hdr->pic_idx_len = (v0 & 0x80) >> 7;
			if (hdr->pic_idx_len == 0)
			{
				hdr->pic_idx = (v0 & 0x7F);
				*header_size = 3;
			}
			else
			{
				CHECK_SIZE(size, pos + 1);
				uint8_t v1 = buffer[pos++];
				hdr->pic_idx = (((uint16_t)(v0 & 0x7F)) << 8) | v1;
				*header_size = 4;
			}

		}
		
		if (hdr->tl0_pic_idx_present != 0) 
		{
			CHECK_SIZE(size, pos + 1);
			hdr->tl0_pic_idx = buffer[pos++];
		}

		//=====
		if (hdr->key_idx_present != 0)
		{
			CHECK_SIZE(size, pos + 1);
			v = buffer[pos++];
			hdr->tid = (v & 0xC0) >> 6;
			hdr->layer = (v & 0x20) >> 5;
			hdr->key_idx = (v & 0x1F);
		}

		if (hdr->startof_vp8_partition == 1 && hdr->part_index == 0)
		{
			CHECK_SIZE(size, pos + 3);
			v = buffer[pos++];

			hdr->h = (v & 0x10) >> 4;
			hdr->version = (v & 0x0E) >> 1;
			hdr->p = (v & 0x01);

			hdr->first_partition_size = (((uint32_t)(v & 0xE0)) << 11);
			hdr->first_partition_size |= (buffer[pos++] << 8);
			hdr->first_partition_size |= buffer[pos++];
		}
		return pos;
	}
}

int vp8_header_serialize(const vp8_header* hdr, int* header_size, uint8_t* buffer, int size)
{
	int pos = 0;
	*header_size = 1;
	//======================
	uint8_t v = 0;
	v |= hdr->extended_present << 7;
	//v |= (hdr->reserved&0x01) << 6;
	v |= (hdr->non_reference & 0x01) << 5;
	v |= (hdr->startof_vp8_partition & 0x01) << 4;
	v |= (hdr->startof_vp8_partition & 0x0F);

	CHECK_SIZE(size, pos + 1);
	buffer[pos++] = v;

	if (hdr->extended_present == 0)
	{
		if (hdr->startof_vp8_partition == 1)
		{
			CHECK_SIZE(size, pos + 3);
			uint8_t v0 = 0, v1 = 0, v2 = 0;
			v0 |= hdr->h << 4;
			v0 |= hdr->version << 1;
			v0 |= hdr->p;
			v0 |= (uint8_t)((hdr->first_partition_size & 0x00070000) >> 11);
			buffer[pos++] = v0;

			v1 = (uint8_t)((hdr->first_partition_size & 0x0000FF00) >> 8);
			v2 = (uint8_t)(hdr->first_partition_size & 0x000000FF);
			buffer[pos++] = v1;
			buffer[pos++] = v2;
		}
		return pos;
	}
	else
	{
		*header_size = 2;
		//======================
		v = 0;
		v |= hdr->pic_idx_present << 7;
		v |= (hdr->tl0_pic_idx_present & 0x01) << 6;
		v |= (hdr->tid_present & 0x01) << 5;
		v |= (hdr->key_idx_present & 0x01) << 4;
		//v |= (hdr->reserved2 & 0x0F);
		buffer[pos++] = v;

		//======================
		if (hdr->pic_idx_present != 0)
		{
			if (hdr->pic_idx_len == 0)
			{
				*header_size = 3;
				CHECK_SIZE(size, pos + 1);

				buffer[pos] |= hdr->pic_idx_len << 7;
				buffer[pos] |= ((uint8_t)hdr->pic_idx) & 0x7F;
				pos++;
			}
			else
			{
				*header_size = 4;
				CHECK_SIZE(size, pos + 2);

				buffer[pos] |= hdr->pic_idx_len << 7;
				buffer[pos] |= (hdr->pic_idx & 0x7FFF) >> 8;
				pos++;
				buffer[pos++] = (hdr->pic_idx & 0x00FF);
			}
		}

		//======================
		if (hdr->tl0_pic_idx_present != 0) 
		{
			CHECK_SIZE(size, pos + 1);
			buffer[pos++] = hdr->tl0_pic_idx;
		}
		//======================
		if (hdr->key_idx_present != 0)
		{
			v = 0;
			v |= hdr->tid << 6;
			v |= (hdr->layer & 0x01) << 5;
			v |= (hdr->key_idx & 0x0F);
			buffer[pos++] = v;
		}
		//======================

		if (hdr->startof_vp8_partition == 1)
		{
			CHECK_SIZE(size, pos + 3);
			uint8_t v0 = 0, v1 = 0, v2 = 0;
			v0 |= hdr->h << 4;
			v0 |= hdr->version << 1;
			v0 |= hdr->p;
			v0 |= (uint8_t)((hdr->first_partition_size & 0x00070000) >> 11);

			v1 = (uint8_t)((hdr->first_partition_size & 0x0000FF00) >> 8);
			v2 = (uint8_t)(hdr->first_partition_size & 0x000000FF);
			buffer[pos++] = v1;
			buffer[pos++] = v2;
		}
		return pos;
	}
	


}

