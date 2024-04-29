/**
 * @file sdes.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sdes.h"

namespace litertp {
namespace rtcp {

sdes::sdes()
{
	packet_ = rtcp_sdes_create();
	if (packet_) {
		rtcp_sdes_init(packet_);
	}
}

sdes::~sdes()
{
	if (packet_) {
		rtcp_sdes_free(packet_);
		packet_ = nullptr;
	}
}

size_t sdes::size()const
{
	return rtcp_sdes_size(packet_);
}

bool sdes::add_item(uint32_t src, rtcp_sdes_type type, const char* data, size_t size)
{
	if (rtcp_sdes_add_entry(packet_,src) < 0) {
		return false;
	}
	
	if (rtcp_sdes_set_item(packet_, src, type, data) > 0) {
		rtcp_sdes_remove_entry(packet_, src);
		return false;
	}

	return true;
}

void sdes::remove_item(uint32_t src, rtcp_sdes_type type)
{
	rtcp_sdes_clear_item(packet_, src, type);

	int idx=rtcp_sdes_find_entry(packet_, src);
	if (idx >= 0) {
		rtcp_sdes_entry* source = &packet_->srcs[idx];
		if (source->item_count == 0) {
			rtcp_sdes_remove_entry(packet_, src);
		}
	}
}

void sdes::remove_entry(uint32_t src)
{
	rtcp_sdes_remove_entry(packet_, src);
}

bool sdes::get_item(uint32_t src, rtcp_sdes_type type, rtcp_sdes_item& item)
{
	const int index = rtcp_sdes_find_entry(packet_, src);
	if (index < 0) {
		return false;
	}

	rtcp_sdes_entry* source = &packet_->srcs[index];

	for (uint8_t i = 0; i < source->item_count; ++i) {
		rtcp_sdes_item* it = &source->items[i];
		if (it->type == type){
			item = *it;
			return true;
		}
	}

	return false;
}



bool sdes::serialize(std::string& buffer)
{
	size_t size=rtcp_sdes_size(packet_);
	uint8_t* buf = (uint8_t*)malloc(size);
	if (!buf) {
		return false;
	}
	rtcp_sdes_serialize(packet_, buf, size);
	buffer.assign((const char*)buf, size);
	free(buf);
	return true;
}

bool sdes::parse(const uint8_t* buffer, size_t size)
{
	int r=rtcp_sdes_parse(packet_, buffer, size);
	if (r < 0) {
		return false;
	}
	return true;
}

}
}

