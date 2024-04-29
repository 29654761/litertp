/**
 * @file packet.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "packet.h"



namespace litertp {

	packet::packet()
	{
		handle_ = rtp_packet_create();
	}
	packet::packet(uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts)
	{
		handle_ = rtp_packet_create();
		this->init(pt, ssrc, seq, ts);
	}

	packet::~packet()
	{
		rtp_packet_free(handle_);
	}

	void packet::init(uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts)
	{
		rtp_packet_init(handle_, pt, ssrc, seq, ts);
	}

	size_t packet::size()const
	{
		return rtp_packet_size(handle_);
	}

	size_t packet::payload_size()const
	{
		return handle_->payload_size;
	}

	const uint8_t* packet::payload()const
	{
		return (const uint8_t*)handle_->payload_data;
	}

	bool packet::serialize(std::string& buffer)
	{
		size_t size=rtp_packet_size(handle_);
		uint8_t* buf = (uint8_t*)malloc(size);
		if (buf == nullptr) {
			return false;
		}

		int r=rtp_packet_serialize(handle_, buf, size);
		if (r < 0) {
			free(buf);
			return false;
		}
		buffer.assign((const char*)buf,size);
		free(buf);
		return true;
	}

	bool packet::parse(const uint8_t* buffer, size_t size)
	{
		int r = rtp_packet_parse(handle_, buffer, size);
		if (r < 0) {
			return false;
		}

		return true;
	}

	bool packet::set_payload(const uint8_t* payload, size_t size)
	{
		if (rtp_packet_set_payload(handle_, payload, size) < 0) {
			return false;
		}
		return true;
	}

	void packet::clear_payload()
	{
		rtp_packet_clear_payload(handle_);
	}

}

