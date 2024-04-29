/**
 * @file vpx_header.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once

#include <stdint.h>

/// <summary>
/// 
///      0 1 2 3 4 5 6 7
///     +-+-+-+-+-+-+-+-+
///     |X|R|N|S| PartID| (REQUIRED)
///     +-+-+-+-+-+-+-+-+
/// X:  |I|L|T|K|  RSV  | (OPTIONAL)
///     +-+-+-+-+-+-+-+-+
/// I : |M|  PictureID  | (OPTIONAL)
///     +-+-+-+-+-+-+-+-+
/// L : |   TL0PICIDX   | (OPTIONAL)
///     +-+-+-+-+-+-+-+-+
/// T/K:|TID|Y|  KEYIDX | (OPTIONAL)
/// +-+-+-+-+-+-+-+-+
/// 
/// 
///  0 1 2 3 4 5 6 7
/// +-+-+-+-+-+-+-+-+
/// |Size0|H| VER |P|
/// +-+-+-+-+-+-+-+-+
/// |      Size1    |
/// +-+-+-+-+-+-+-+-+
/// |      Size2    |
/// +-+-+-+-+-+-+-+-+
/// | Bytes 4..N of |
/// | VP8 payload   |
/// :               :
/// +-+-+-+-+-+-+-+-+
/// | OPTIONAL RTP  |
/// | padding       |
/// :               :
/// +-+-+-+-+-+-+-+-+
/// </summary>
typedef struct _vp8_header
{
	uint8_t extended_present : 1;				//1b  1 for has OPTIONAL£¨X,I,L,TK£©
	uint8_t reserved : 1;						//1b
	uint8_t non_reference : 1;					//1b
	uint8_t startof_vp8_partition : 1;			//1b
	uint8_t part_index : 4;						//4b

	uint8_t pic_idx_present : 1;				//1b
	uint8_t tl0_pic_idx_present : 1;			//1b
	uint8_t tid_present : 1;					//1b
	uint8_t key_idx_present : 1;				//1b
	uint8_t reserved2 : 4;						//4b

	uint8_t pic_idx_len : 1;					//1b  0-7b; 1-15b;
	uint16_t pic_idx : 15;						//7 or 15 b

	uint8_t tl0_pic_idx;						//8b

	uint8_t tid : 2;							//2b
	uint8_t layer : 1;							//1b
	uint8_t key_idx : 5;						//5 bits temporal key frame index.

	uint8_t h:1;								
	uint8_t version : 3;
	uint8_t p : 1;					//1b key frame
	uint32_t first_partition_size : 19;
}vp8_header;








int vp8_header_deserialize(vp8_header* hdr,int* header_size, const uint8_t* buffer,int size);
int vp8_header_serialize(const vp8_header* hdr, int* header_size, uint8_t* buffer, int size);
