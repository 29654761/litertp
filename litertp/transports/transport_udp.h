/**
 * @file transport_udp.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "transport.h"

#ifdef LITERTP_SSL
#include <srtp2/srtp.h>
#include "../dtls/dtls.h"
#endif





namespace litertp {



	class transport_udp:public transport
	{
	public:

		transport_udp(int port);
		virtual ~transport_udp();


		virtual bool start();
		virtual void stop();
		virtual bool enable_security(bool enabled);
		virtual std::string fingerprint() const;

		virtual bool send_rtp_packet(packet_ptr packet, const sockaddr* addr, int addr_size);
		virtual bool send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size);
		virtual void send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority);

		
	private:
		void run_recever();
		void on_data_received_event(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
#ifdef LITERTP_SSL
		void on_dtls(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
#endif
		void on_stun_message(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
		void on_rtp_data(const uint8_t* data, int size, const sockaddr* addr, int addr_size);



		proto_type_t test_message(uint8_t b);

	public:

		std::thread* receiver_ = nullptr;

		bool handshake = false;
#ifdef LITERTP_SSL
		dtls_ptr dtls_;
		srtp_t srtp_in_ = nullptr;
		srtp_policy_t srtp_in_policy_;
		srtp_t srtp_out_ = nullptr;
		srtp_policy_t srtp_out_policy_;
#endif
	};

}
