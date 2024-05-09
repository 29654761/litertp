/**
 * @file transport_tcp_4571.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "transport_tcp_4571.h"
#include "../proto/rtcp_header.h"
#include "../log.h"

#include <sys2/util.h>

namespace litertp {

	transport_tcp_4571::transport_tcp_4571(const char* address, int port)
	{
		remote_address_ = port;
		port_ = port;
		is_client_ = true;
	}

	transport_tcp_4571::transport_tcp_4571(int listen_port)
	{
		port_ = listen_port;
		is_client_ = false;
	}

	transport_tcp_4571::~transport_tcp_4571()
	{
		stop();
	}


	bool transport_tcp_4571::start()
	{
		if (!transport::start())
		{
			return false;
		}
		
		socket_ = std::make_shared<sys::socket>();
		socket_->create_tcp_socket(AF_INET);

		if (is_client_)
		{
			client_ = socket_;
			sockaddr_storage addr = { 0 };
			sys::socket::ep2addr(remote_address_.c_str(), port_, (sockaddr*)&addr);
			if (!client_->connect((const sockaddr*)&addr, sizeof(addr)))
			{
				stop();
				return false;
			}

			client_->set_recvbuf_size(1024 * 1024);
			client_->set_sendbuf_size(1024 * 1024);

			receiver_ = new std::thread(&transport_tcp_4571::run_receive, this);
		}
		else 
		{
			sockaddr_in v4 = { 0 };
			sys::socket::ep2addr(AF_INET, nullptr, port_, (sockaddr*)&v4);
			if (!socket_->bind((const sockaddr*)&v4, sizeof(v4))) {
				LOGE("transport start error: bind rtp port failed.");
				stop();
				return false;
			}

			if (!socket_->listen(0x7FFFFFFF))
			{
				LOGE("transport start error: listen failed.");
				stop();
				return false;
			}

			accept_ = new std::thread(&transport_tcp_4571::run_accept, this);
		}
		return true;
	}

	void transport_tcp_4571::stop()
	{
		transport::stop();

		if (socket_)
		{
			socket_->close();
		}

		if (client_)
		{
			client_->close();
		}

		if (accept_) {
			accept_->join();
			delete accept_;
			accept_ = nullptr;
		}
		if (receiver_) {
			receiver_->join();
			delete receiver_;
			receiver_ = nullptr;
		}

		socket_.reset();
		client_.reset();
	}

	bool transport_tcp_4571::enable_security(bool enabled)
	{
		return false;
	}

	std::string transport_tcp_4571::fingerprint()const
	{
		return "";
	}

	bool transport_tcp_4571::send_rtp_packet(packet_ptr packet,const sockaddr* addr,int addr_size)
	{
		if (!client_)
		{
			return false;
		}

		std::string b;
		b.reserve(2048);
		if (!packet->serialize(b))
		{
			return false;
		}


		uint16_t len = (uint16_t)b.size();
		len = sys::socket::hton16(len);
		if (client_->send((const char*)&len, 2) <= 0)
		{
			return false;
		}

		if (client_->send(b.data(), (int)b.size()) <= 0)
		{
			return false;
		}

		return true;
	}

	bool transport_tcp_4571::send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size)
	{
		if (!client_)
		{
			return false;
		}

		uint16_t len = sys::socket::hton16((uint16_t)size);
		if (client_->send((const char*)&len, 2) <= 0)
		{
			return false;
		}

		if(client_->send((const char*)rtcp_data,size)<=0)
		{
			return false;
		}
		return true;
	}

	void transport_tcp_4571::send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority)
	{

	}

	void transport_tcp_4571::run_accept()
	{
		while (active_)
		{
			client_=socket_->accept();
			if (client_)
			{
				client_->set_recvbuf_size(1024 * 1024);
				client_->set_sendbuf_size(1024 * 1024);

				receiver_ = new std::thread(&transport_tcp_4571::run_receive, this);
				break;
			}
		}
	}

	void transport_tcp_4571::run_receive()
	{

		const sockaddr* addr=client_->remote_addr();
		socklen_t addr_size = client_->remote_addr_size();

		while (active_)
		{
			uint8_t magic = 0;
			if (!blocking_read(&magic, 1))
			{
				break;
			}

			uint16_t len = 0;
			if (!blocking_read((uint8_t*)&len, 2))
			{
				break;
			}
			len=sys::socket::ntoh16(len);
			if (len >= 2048)
			{
				break;
			}

			uint8_t buffer[2048] = { 0 };

			if (!blocking_read(buffer, len))
			{
				break;
			}

			int pt = 0;
			if (test_rtcp_packet(buffer, len, &pt))
			{
				rtcp_header hdr = { 0 };
				int pt = rtcp_header_parse(&hdr, buffer, len);
				if (pt >= 0)
				{
					rtcp_packet_event_.invoke(socket_, (uint16_t)pt, buffer, len, addr, addr_size);
				}
			}
			else
			{
				auto pkt = std::make_shared<packet>();
				if (pkt->parse(buffer, len)) {
					rtp_packet_event_.invoke(client_, pkt, addr, addr_size);
				}
			}

			disconnect_event_.invoke();
		}
	}


	bool transport_tcp_4571::blocking_read(uint8_t* data, int size)
	{
		while (size > 0)
		{
			int rd=client_->recv((char*)data, size);
			if (rd <= 0)
			{
				return false;
			}

			size -= rd;
			data += rd;
		}
		return true;
	}
}


