/**
 * @file transport_tcp_2326.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "transport.h"



namespace litertp {

	class transport_tcp_2326:public transport
	{
	public:
		transport_tcp_2326(const char* address,int port, char rtp_channel, char rtcp_channel);
		transport_tcp_2326(int listen_port, char rtp_channel, char rtcp_channel);
		virtual ~transport_tcp_2326();


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

		char rtp_channel_ = 0;
		char rtcp_channel_ = 1;

		std::thread* accept_ = nullptr;
		std::thread* receiver_ = nullptr;
	};

}
