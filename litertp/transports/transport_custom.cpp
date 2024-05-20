/**
 * @file transport_custom.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "transport_custom.h"
#include "../proto/rtcp_header.h"
#include "../log.h"
#include "../global.h"

#include <sys2/util.h>

namespace litertp {

	transport_custom::transport_custom(int port,litertp_on_send_packet on_send_packet, void* ctx)
	{
		port_ = port;
		send_event_.add(on_send_packet, ctx);
	}

	transport_custom::~transport_custom()
	{
		stop();
	}


	bool transport_custom::start()
	{
		return transport::start();
	}

	void transport_custom::stop()
	{
		transport::stop();
	}

	bool transport_custom::enable_security(bool enabled)
	{
		return true;
	}

	std::string transport_custom::fingerprint()const
	{
		return "";
	}

	bool transport_custom::send_rtp_packet(packet_ptr packet,const sockaddr* addr,int addr_size)
	{
		std::string b;
		b.reserve(2048);
		if (!packet->serialize(b))
		{
			return false;
		}

		send_event_.invoke(port_, 0, (const uint8_t*)b.data(), b.size());
		return true;
	}

	bool transport_custom::send_rtcp_packet(const uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size)
	{
		send_event_.invoke(port_, 1, rtcp_data, size);
		return true;
	}


	void transport_custom::send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority)
	{
	}



	

}


