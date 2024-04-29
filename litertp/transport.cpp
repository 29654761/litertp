/**
 * @file transport.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "transport.h"
#include "proto/rtcp_header.h"
#include "log.h"

#include <sys2/util.h>

namespace litertp {

	transport::transport()
	{
	}

	transport::~transport()
	{
		stop();
	}

	bool transport::set_sendbuf_size(int size)
	{
		return socket_->set_sendbuf_size(size);
	}

	bool transport::set_recvbuf_size(int size)
	{
		return socket_->set_recvbuf_size(size);
	}

	bool transport::start(int port)
	{
		if (active_) {
			return true;
		}
		active_ = true;

		port_ = port;
		
		socket_ = std::make_shared<sys::socket>();
		socket_->create_udp_socket(AF_INET);

		sockaddr_in v4 = { 0 };
		sys::socket::ep2addr(AF_INET,nullptr, port_, (sockaddr*)&v4);
		if (!socket_->bind((const sockaddr*)&v4, sizeof(v4))) {
			LOGE("transport start error: bind rtp port failed.");
			stop();
			return false;
		}

		socket_->set_recvbuf_size(1024 * 1024);
		socket_->set_sendbuf_size(1024 * 1024);

		receiver_ = new std::thread(&transport::run_recever, this);

		return true;
	}

	void transport::stop()
	{
		active_ = false;

		socket_.reset();


		if (receiver_) {
			receiver_->join();
			delete receiver_;
			receiver_ = nullptr;
		}


#ifdef LITERTP_SSL
		if (srtp_out_) {
			srtp_dealloc(srtp_out_);
			srtp_out_ = nullptr;
		}
		if (srtp_in_) {
			srtp_dealloc(srtp_in_);
			srtp_in_ = nullptr;
		}
#endif
	}

	bool transport::send_rtp_packet(packet_ptr packet,const sockaddr* addr,int addr_size)
	{
		std::string b;
		b.reserve(2048); // size for srtp
		if (!packet->serialize(b))
		{
			return false;
		}

		int size = b.size();

#ifdef LITERTP_SSL
		if (srtp_out_)
		{
			std::unique_lock<std::recursive_mutex> lk(mutex_);
			auto ret = srtp_protect(srtp_out_, (void*)b.data(), &size);
			if (ret != srtp_err_status_ok)
			{
				LOGE("srtp_protect err = %d", ret);
				return false;
			}
		}
#endif

		int r = socket_->sendto(b.data(), size, addr,addr_size);
		return r >= 0;
	}

	bool transport::send_rtcp_packet(uint8_t* rtcp_data, int size, const sockaddr* addr, int addr_size)
	{
	#ifdef LITERTP_SSL
			if (srtp_out_)
			{
				std::unique_lock<std::recursive_mutex> lk(mutex_);
				auto ret = srtp_protect_rtcp(srtp_out_, (void*)rtcp_data, &size);
				if (ret != srtp_err_status_ok)
				{
					LOGE("srtp_protect err = %d", ret);
					return false;
				}
			}
#endif

		int r = socket_->sendto((const char*)rtcp_data, size, addr,addr_size);
		if (r <= 0) {
			return false;
		}
		return true;
	}


	void transport::run_recever()
	{
		while (active_)
		{
			char buffer[2048] = { 0 };
			sockaddr_storage addr = { 0 };
			socklen_t addr_size = sizeof(addr);
			int size = socket_->recvfrom(buffer, 2048, (sockaddr*)&addr, &addr_size);
			if (size >= 0)
			{
				on_data_received_event((const uint8_t*)buffer, size, (sockaddr*)&addr, addr_size);
			}
		}
	}

	void transport::on_data_received_event(const uint8_t* data, int size, const sockaddr* addr, int addr_size)
	{
		auto proto = test_message(data[0]);
		if (proto == proto_stun)
		{
			on_stun_message(data, size, (const sockaddr*)addr, addr_size);
		}
		else if (proto == proto_dtls)
		{
#ifdef LITERTP_SSL
			on_dtls(data, size, (const sockaddr*)addr, addr_size);
#endif
		}
		else if (proto == proto_rtp)
		{
			on_rtp_data(data, size, (const sockaddr*)addr, addr_size);
		}
	}


#ifdef LITERTP_SSL
	void transport::on_dtls(const uint8_t* data, int size, const sockaddr* addr, int addr_size)
	{
		if (!dtls_)
		{
			return;
		}

		if (!dtls_->is_established())
		{
			std::string rsp = dtls_->accept(data, size);
			if (rsp.size() > 0)
			{
				socket_->sendto(rsp.data(), rsp.size(), addr, addr_size);
			}
		}
		else
		{
			dtls_->read(data, size);

			std::string rsp = dtls_->flush();
			if (rsp.size() > 0)
			{
				socket_->sendto(rsp.data(), rsp.size(), addr, addr_size);
			}

			if (dtls_->is_init_finished())
			{
				litertp::dtls_info_t info = { 0 };
				if (dtls_->export_key_material(&info))
				{
					memset(&srtp_in_policy_, 0, sizeof(srtp_in_policy_));
					memset(&srtp_out_policy_, 0, sizeof(srtp_out_policy_));

					srtp_out_policy_.allow_repeat_tx = 1; // for retransmissions 

					//srtp_crypto_policy_set_aes_gcm_128_8_auth(&srtp_in_policy_.rtp);
					//srtp_crypto_policy_set_aes_gcm_128_8_auth(&srtp_in_policy_.rtcp);					
					srtp_crypto_policy_set_from_profile_for_rtp(&srtp_in_policy_.rtp, (srtp_profile_t)info.profile_id);
					srtp_crypto_policy_set_from_profile_for_rtcp(&srtp_in_policy_.rtcp, (srtp_profile_t)info.profile_id);

					srtp_in_policy_.ssrc.type = ssrc_any_inbound;
					if (srtp_role_ == srtp_role_client)
					{
						srtp_in_policy_.key = info.server_key;
					}
					else
					{
						srtp_in_policy_.key = info.client_key;
					}

					//srtp_crypto_policy_set_rtp_default(&srtp_out_policy_.rtp);
					//srtp_crypto_policy_set_rtcp_default(&srtp_out_policy_.rtcp);
					srtp_crypto_policy_set_from_profile_for_rtp(&srtp_out_policy_.rtp, (srtp_profile_t)info.profile_id);
					srtp_crypto_policy_set_from_profile_for_rtcp(&srtp_out_policy_.rtcp, (srtp_profile_t)info.profile_id);
					srtp_out_policy_.ssrc.type = ssrc_any_outbound;
					if (srtp_role_ == srtp_role_client)
					{
						srtp_out_policy_.key = info.client_key;
					}
					else
					{
						srtp_out_policy_.key = info.server_key;
					}


					std::unique_lock<std::recursive_mutex> lk(mutex_);

					if (srtp_in_)
					{
						srtp_dealloc(srtp_in_);
						srtp_in_ = nullptr;
					}
					srtp_err_status_t r1 = srtp_create(&srtp_in_, &srtp_in_policy_);
					
					if (srtp_out_) 
					{
						srtp_dealloc(srtp_out_);
						srtp_out_ = nullptr;
					}
					r1 = srtp_create(&srtp_out_, &srtp_out_policy_);

					handshake = true;
				}
			}
		}

	}
#endif

	void transport::on_stun_message(const uint8_t* data, int size, const sockaddr* addr, int addr_size)
	{
		stun_message msg;
		uint32_t fp = 0;
		if (!msg.deserialize((const char*)data, size, fp)) {
			return;
		}

		stun_message_event_.invoke(socket_, msg, addr, addr_size);

		if (msg.type_ == stun_message_type_binding_response)
		{
#ifdef LITERTP_SSL
			if (msg.type_ == stun_message_type_binding_request)
			{
				if (dtls_ && srtp_role_ == srtp_role_client)
				{
					std::string req = dtls_->connect();
					if (req.size() > 0) {
						socket_->sendto(req.data(), req.size(), addr, addr_size);
					}
				}
			}
#endif
		}
		else if (msg.type_ == stun_message_type_binding_request)
		{
			stun_message rsp;
			rsp.type_ = stun_message_type_binding_response;
			memcpy(rsp.transaction_id_, msg.transaction_id_, 12);



			auto attr = stun_attribute_xor_mapped_address::create(addr, (const char*)msg.transaction_id_);
			rsp.attributes_.emplace_back(stun_attribute_type_xor_mapped_address, attr.serialize());


			std::string s = rsp.serialize(ice_pwd_local_);
			socket_->sendto(s.data(), s.size(), addr, addr_size);

			this->send_stun_request(addr, addr_size, 1853766911);
		}
	}

	void transport::on_rtp_data(const uint8_t* data, int size, const sockaddr* addr, int addr_size)
	{
		int pt = 0;
		if (this->test_rtcp_packet(data, size, &pt))
		{
#ifdef LITERTP_SSL
			if (srtp_in_)
			{
				srtp_err_status_t ret = srtp_unprotect_rtcp(srtp_in_, (void*)data, &size);
				if (ret != srtp_err_status_ok)
				{
					LOGE("srtp_unprotect failed err=%d\n", ret);
					return;
				}
			}
#endif
			rtcp_packet_event_.invoke(socket_,(uint16_t)pt, (const uint8_t*)data, size,addr,addr_size);
		}
		else
		{
#ifdef LITERTP_SSL
			if (srtp_in_)
			{
				srtp_err_status_t ret = srtp_unprotect(srtp_in_, (void*)data, &size);
				if (ret != srtp_err_status_ok)
				{
					LOGE("srtp_unprotect failed err=%d\n", ret);
					return;
				}
			}
#endif
			auto pkt = std::make_shared<packet>();
			if (pkt->parse((const uint8_t*)data, size)) {
				rtp_packet_event_.invoke(socket_, pkt, addr, addr_size);
			}
		}
	}


	proto_type_t transport::test_message(uint8_t b)
	{
		if (b > 127 && b < 192) {
			return proto_rtp;
		}
		else if (b >= 20 && b <= 64) {
			return proto_dtls;
		}
		else if (b < 2) {
			return proto_stun;
		}
		else {
			return proto_unknown;
		}
	}

	bool transport::test_rtcp_packet(const uint8_t* data, int size, int* pt)
	{
		rtcp_header hdr = { 0 };
		int ptv = rtcp_header_parse(&hdr, (const uint8_t*)data, size);
		if (pt)
		{
			*pt = ptv;
		}
		if (ptv < 0)
		{
			return false;
		}
		if (ptv == rtcp_packet_type::RTCP_APP ||
			ptv == rtcp_packet_type::RTCP_BYE ||
			ptv == rtcp_packet_type::RTCP_RR ||
			ptv == rtcp_packet_type::RTCP_SR ||
			ptv == rtcp_packet_type::RTCP_SDES ||
			ptv == rtcp_packet_type::RTCP_PSFB ||
			ptv == rtcp_packet_type::RTCP_RTPFB)
		{
			return true;
		}


		return false;
	}

	void transport::send_stun_request(const sockaddr* addr, int addr_size, uint32_t priority)
	{
		stun_message req;
		req.type_ = stun_message_type_binding_request;
		req.make_transaction_id();
		req.attributes_.emplace_back(stun_attribute_type_username, ice_ufrag_remote_ + ":" + ice_ufrag_local_);

		std::string net;
		uint16_t net_id = local_ice_network_id_;
		net_id = sys::socket::hton16(net_id);
		net.append((const char*)&net_id, 2);
		uint16_t net_cost = this->local_ice_network_cost_;
		net_cost = sys::socket::hton16(net_cost);
		net.append((const char*)&net_cost, 2);
		req.attributes_.emplace_back(stun_attribute_type_goog_network_info, net);

		uint64_t v = sys::util::random_number<uint64_t>(0, 0xFFFFFFFFFFFFFFFF);
		v = sys::socket::hton64(v);
		if (sdp_type_ == sdp_type_offer) {
			req.attributes_.emplace_back(stun_attribute_type_ice_controlling, (const char*)&v, sizeof(v));
		}
		else {
			req.attributes_.emplace_back(stun_attribute_type_ice_controlled, (const char*)&v, sizeof(v));
		}

		req.attributes_.emplace_back(stun_attribute_type_use_candidate, "");

		priority = sys::socket::hton32(priority);
		req.attributes_.emplace_back(stun_attribute_type_priority, (const char*)&priority, sizeof(priority));

		std::string sreq = req.serialize(ice_pwd_remote_);
		socket_->sendto(sreq.data(), sreq.size(), addr, addr_size);

#ifdef LITERTP_SSL
		if (dtls_ && !handshake && srtp_role_ == srtp_role_client)
		{
			std::string req = dtls_->connect();
			if (req.size() > 0) {
				socket_->sendto(req.data(), req.size(), addr, addr_size);
			}
		}
#endif
	}
}


