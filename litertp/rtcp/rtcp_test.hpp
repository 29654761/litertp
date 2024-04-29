/**
 * @file rtcp_test.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once
#include "../proto/rtcp_fb.h"
#include "../proto/rtcp_bye.h"

void test_rtcp_nack()
{
	rtcp_fb* packet=rtcp_fb_create();

	rtcp_fb_init(packet, RTCP_RTPFB, RTCP_RTPFB_FMT_NACK);
	rtcp_rtpfb_nack_set(packet, 100, 0x0001);

	size_t size = rtcp_fb_size(packet);
	uint8_t* buffer = (uint8_t*)malloc(size);
	rtcp_fb_serialize(packet,buffer,size);

	rtcp_fb* packet2 = rtcp_fb_create();
	rtcp_fb_parse(packet2, buffer, size);

	uint16_t fst = 0, nxt = 0;
	rtcp_rtpfb_nack_get(packet, &fst, &nxt);

	free(buffer);
	rtcp_fb_free(packet);
	rtcp_fb_free(packet2);
}



void test_rtcp_fb()
{
	rtcp_fb* packet = rtcp_fb_create();
	rtcp_fb_init(packet, RTCP_RTPFB, RTCP_RTPFB_FMT_NACK);
	rtcp_fb_set_fci(packet, (const uint8_t*)"12345", 5);
	
	size_t size = rtcp_fb_size(packet);
	uint8_t* buffer = (uint8_t*)malloc(size);
	rtcp_fb_serialize(packet, buffer, size);

	rtcp_fb* packet2 = rtcp_fb_create();
	rtcp_fb_parse(packet2, buffer, size);

	rtcp_fb_free(packet);
	rtcp_fb_free(packet2);
}



void test_rtcp_fir()
{
	rtcp_fb* packet = rtcp_fb_create();
	rtcp_fb_init(packet, RTCP_PSFB, RTCP_PSFB_FMT_FIR);

	rtcp_psfb_fir_item items[5];
	memset(items,0, sizeof(items));
	for (int i = 0; i < 5; i++)
	{
		items[i].ssrc = i;
		items[i].seq_nr = 11;
	}
	rtcp_psfb_fir_set_items(packet, items, 5);

	
	size_t size = rtcp_fb_size(packet);
	uint8_t* buffer = (uint8_t*)malloc(size);
	rtcp_fb_serialize(packet, buffer, size);




	rtcp_fb* packet2 = rtcp_fb_create();
	rtcp_fb_parse(packet2, buffer, size);

	int count=rtcp_psfb_fir_item_count(packet);
	for (int i = 0; i < count; i++)
	{
		rtcp_psfb_fir_item item;
		rtcp_psfb_fir_get_item(packet, i, &item);

	}
	{
		rtcp_psfb_fir_item item;
		rtcp_psfb_fir_find_item(packet, 3, &item);
	}
	rtcp_fb_free(packet);
	rtcp_fb_free(packet2);
}

