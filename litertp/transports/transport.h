/**
 * @file transport.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "../packet.h"
#include "../litertp_def.h"
#include "../stun/stun_message.h"

#include <memory>
#include <thread>
#include <sys2/socket.h>
#include <sys2/blocking_queue.hpp>
#include <sys2/mutex_callback.hpp>




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

		typedef void (*transport_disconnect)(void* ctx);

		transport();
		~transport();


		virtual bool start();
		virtual void stop();

		virtual bool send_rtp_packet(packet_ptr packet, const sockaddr* addr, int addr_size) = 0;
		virtual bool send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size) = 0;
		virtual void send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority)=0;

		virtual bool enable_security(bool enabled)=0;
		virtual std::string fingerprint() const= 0;

		bool test_rtcp_packet(const uint8_t* data, int size, int* pt);
	public:

		std::recursive_mutex mutex_;
		bool active_ = false;
		std::shared_ptr<sys::socket> socket_;

		sys::mutex_callback<transport_rtp_packet> rtp_packet_event_;
		sys::mutex_callback<transport_rtcp_packet> rtcp_packet_event_;
		sys::mutex_callback<transport_stun_message> stun_message_event_;
		sys::mutex_callback<transport_disconnect> disconnect_event_;

		sdp_type_t sdp_type_ = sdp_type_offer;
		srtp_role_t srtp_role_ = srtp_role_server;





		int port_ = 0;

		std::string ice_ufrag_remote_;
		std::string ice_pwd_remote_;
		std::string ice_ufrag_local_;
		std::string ice_pwd_local_;
		uint16_t local_ice_network_id_ = 1;
		uint16_t local_ice_network_cost_ = 10;

	};


	typedef std::shared_ptr<transport> transport_ptr;
}
