/**
 * @file transport.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <memory>
#include <thread>
#include <sys2/socket.h>
#include <sys2/blocking_queue.hpp>
#include <sys2/mutex_callback.hpp>

#ifdef LITERTP_SSL
#include <srtp2/srtp.h>
#include "dtls/dtls.h"
#endif

#include "stun/stun_message.h"
#include "packet.h"
#include "litertp_def.h"


namespace litertp {

	typedef enum proto_type_t
	{
		proto_unknown = 0,
		proto_rtp = 1,
		proto_dtls = 2,
		proto_stun = 3,
	}proto_type_t;

	class transport
	{
	public:
		typedef void (*transport_rtp_packet)(void* ctx, std::shared_ptr<sys::socket> skt, packet_ptr packet, const sockaddr* addr, int addr_size);
		typedef void (*transport_rtcp_packet)(void* ctx, std::shared_ptr<sys::socket> skt, uint16_t pt, const uint8_t* buffer, size_t size, const sockaddr* addr, int addr_size);
		typedef void (*transport_stun_message)(void* ctx, std::shared_ptr<sys::socket> skt, const stun_message& msg, const sockaddr* addr, int addr_size);

		transport();
		~transport();

		bool set_sendbuf_size(int size);
		bool set_recvbuf_size(int size);


		bool start(int port);
		void stop();

		bool send_rtp_packet(packet_ptr packet, const sockaddr* addr, int addr_size);
		bool send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size);
		void send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority);
	private:
		void run_recever();
		void on_data_received_event(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
#ifdef LITERTP_SSL
		void on_dtls(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
#endif
		void on_stun_message(const uint8_t* data, int size, const sockaddr* addr, int addr_size);
		void on_rtp_data(const uint8_t* data, int size, const sockaddr* addr, int addr_size);



		proto_type_t test_message(uint8_t b);
		bool test_rtcp_packet(const uint8_t* data, int size, int* pt);
	public:
		std::recursive_mutex mutex_;
		bool active_ = false;

		std::shared_ptr<sys::socket> socket_;
		sys::mutex_callback<transport_rtp_packet> rtp_packet_event_;
		sys::mutex_callback<transport_rtcp_packet> rtcp_packet_event_;
		sys::mutex_callback<transport_stun_message> stun_message_event_;

		std::thread* receiver_ = nullptr;

		int port_ = 0;

		std::string ice_ufrag_remote_;
		std::string ice_pwd_remote_;
		std::string ice_ufrag_local_;
		std::string ice_pwd_local_;
		uint16_t local_ice_network_id_ = 1;
		uint16_t local_ice_network_cost_ = 10;
		sdp_type_t sdp_type_ = sdp_type_offer;


		bool handshake = false;
		srtp_role_t srtp_role_ = srtp_role_server;
#ifdef LITERTP_SSL
		dtls_ptr dtls_;
		srtp_t srtp_in_ = nullptr;
		srtp_policy_t srtp_in_policy_;
		srtp_t srtp_out_ = nullptr;
		srtp_policy_t srtp_out_policy_;
#endif
	};


	typedef std::shared_ptr<transport> transport_ptr;
}
