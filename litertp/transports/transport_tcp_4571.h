/**
 * @file transport_tcp_4571.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "transport.h"



namespace litertp {

	class transport_tcp_4571:public transport
	{
	public:
		transport_tcp_4571(const char* address,int port);
		transport_tcp_4571(int listen_port);
		virtual ~transport_tcp_4571();


		virtual bool start();
		virtual void stop();
		virtual bool enable_security(bool enabled);
		virtual std::string fingerprint() const;

		virtual bool send_rtp_packet(packet_ptr packet, const sockaddr* addr, int addr_size);
		virtual bool send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size);
		virtual void send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority);

		bool is_client()const { return is_client_; }
	private:
		void run_accept();
		void run_receive();
		bool blocking_read(uint8_t* data, int size);

	private:
		bool is_client_ = false;

		std::string remote_address_;
		std::shared_ptr<sys::socket> client_;


		std::thread* accept_ = nullptr;
		std::thread* receiver_ = nullptr;
	};

}
