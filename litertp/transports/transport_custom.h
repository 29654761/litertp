/**
 * @file transport_custom.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "transport.h"
#include <sys2/mutex_callback.hpp>





namespace litertp {



	class transport_custom :public transport
	{
	public:

		transport_custom(int port, litertp_on_send_packet on_send_packet,void* ctx);
		virtual ~transport_custom();


		virtual bool start();
		virtual void stop();
		virtual bool enable_security(bool enabled);
		virtual std::string fingerprint() const;

		virtual bool send_rtp_packet(packet_ptr packet, const sockaddr* addr, int addr_size);
		virtual bool send_rtcp_packet(const uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size);
		virtual void send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority);

		

	public:

		sys::mutex_callback<litertp_on_send_packet> send_event_;
	};

}
