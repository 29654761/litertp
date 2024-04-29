/**
 * @file packet.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once
#include <memory>
#include <string>
#include <sys2/socket.h>

#include "proto/rtp_packet.h"


namespace litertp {

	class packet
	{
	public:
		packet();
		packet(uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts);
		~packet();

		size_t size()const;
		size_t payload_size()const;
		const uint8_t* payload()const;
		
		bool serialize(std::string& buffer);
		bool parse(const uint8_t* buffer, size_t size);

		bool set_payload(const uint8_t* payload, size_t size);
		void clear_payload();

	private:
		void init(uint8_t pt, uint32_t ssrc, uint16_t seq, uint32_t ts);

	public:
		rtp_packet* handle_ = nullptr;
	};
	

	typedef std::shared_ptr<packet> packet_ptr;



}

