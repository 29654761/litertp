/**
 * @file media_stream.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "media_stream.h"
#include "log.h"



#include "senders/sender_audio.h"
#include "senders/sender_audio_aac.h"
#include "senders/sender_video_h264.h"
#include "senders/sender_video_vp8.h"


#include "receivers/receiver_audio.h"
#include "receivers/receiver_audio_aac.h"
#include "receivers/receiver_video_h264.h"
#include "receivers/receiver_video_vp8.h"

#include "proto/rtp_source.h"

#include "util/time.h"

#include <sys2/util.h>
#include <sys2/string_util.h>

namespace litertp
{

	media_stream::media_stream(media_type_t media_type,uint32_t ssrc, const std::string& mid, const std::string& cname, const std::string& ice_ufrag, const std::string& ice_pwd,
		const std::string& local_address, transport_ptr transport_rtp, transport_ptr transport_rtcp)
	{
		local_sdp_media_.media_type_ = media_type;
		local_sdp_media_.ice_ufrag_= ice_ufrag;
		local_sdp_media_.ice_pwd_= ice_pwd;
		local_sdp_media_.msid_ = sys::util::uuid();
		local_sdp_media_.mid_ = mid;
		local_sdp_media_.rtcp_address_ = local_address;
		local_sdp_media_.rtcp_port_ = transport_rtcp->port_;
		local_sdp_media_.rtp_address_ = local_address;
		local_sdp_media_.rtp_port_ = transport_rtp->port_;
		cname_ = cname;


		local_sdp_media_.rtcp_mux_ = (transport_rtp == transport_rtcp);

		if (ssrc == 0)
		{
			ssrc = sys::util::random_number<uint32_t>(0x10000, 0xFFFFFFFF);
		}

		ssrc_t ssrct;
		ssrct.ssrc = ssrc;
		ssrct.cname = cname_;
		ssrct.msid = local_sdp_media_.msid_;
		local_sdp_media_.ssrcs_.push_back(ssrct);



		transport_rtp_ = transport_rtp;
		transport_rtp_->rtp_packet_event_.add(s_transport_rtp_packet, this);
		transport_rtp_->stun_message_event_.add(s_transport_stun_message, this);

		transport_rtcp_ = transport_rtcp;
		transport_rtcp_->rtcp_packet_event_.add(s_transport_rtcp_packet, this);
		if (transport_rtcp_ != transport_rtp_)
		{
			transport_rtcp_->stun_message_event_.add(s_transport_stun_message, this);
		}
	}

	media_stream::~media_stream()
	{
		transport_rtp_->rtp_packet_event_.remove(s_transport_rtp_packet, this);
		transport_rtcp_->rtcp_packet_event_.remove(s_transport_rtcp_packet, this);
		transport_rtp_->stun_message_event_.remove(s_transport_stun_message, this);
		transport_rtcp_->stun_message_event_.remove(s_transport_stun_message, this);
		close();
	}



	void media_stream::close()
	{
		send_rtcp_bye();

		{
			std::unique_lock<std::shared_mutex>lk(senders_mutex_);
			senders_.clear();
		}
		{
			std::unique_lock<std::shared_mutex>lk(receivers_mutex_);
			receivers_.clear();
		}


	}

	
	bool media_stream::add_local_video_track(codec_type_t codec, uint16_t pt, int frequency)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		if (local_sdp_media_.media_type_ != media_type_video)
		{
			return false;
		}

		sdp_format fmt(pt, codec, frequency);
		fmt.rtcp_fb_.insert("goog-remb");
		//fmt.rtcp_fb_.insert("transport-cc");
		fmt.rtcp_fb_.insert("ccm fir");
		fmt.rtcp_fb_.insert("nack");
		fmt.rtcp_fb_.insert("nack pli");
		local_sdp_media_.rtpmap_.insert(std::make_pair((int)pt,fmt));



		return true;
	}

	bool media_stream::add_local_audio_track(codec_type_t codec, uint16_t pt, int frequency, int channels)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		if (local_sdp_media_.media_type_ != media_type_audio)
		{
			return false;
		}

		sdp_format fmt(pt, codec, frequency, channels);
		local_sdp_media_.rtpmap_.insert(std::make_pair((int)pt, fmt));

		
		return true;
	}

	bool media_stream::add_remote_video_track(codec_type_t codec, uint16_t pt, int frequency)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		if (media_type() != media_type_video)
		{
			return false;
		}

		sdp_format fmt(pt, codec, frequency);
		remote_sdp_media_.rtpmap_.insert(std::make_pair((int)pt, fmt));

		
		return true;
	}

	bool media_stream::add_remote_audio_track(codec_type_t codec, uint16_t pt, int frequency, int channels)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		if (media_type()!= media_type_audio)
		{
			return false;
		}

		sdp_format fmt(pt, codec, frequency, channels);
		remote_sdp_media_.rtpmap_.insert(std::make_pair((int)pt, fmt));

		return true;
	}


	void media_stream::set_remote_trans_mode(rtp_trans_mode_t trans_mode)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		remote_sdp_media_.trans_mode_ = trans_mode;
	}
	void media_stream::set_local_trans_mode(rtp_trans_mode_t trans_mode)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		local_sdp_media_.trans_mode_ = trans_mode;
	}

	void media_stream::set_remote_ssrc(uint32_t ssrc)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		ssrc_t ssrct;
		ssrct.ssrc = ssrc;
		ssrct.cname = cname_;
		ssrct.msid = remote_sdp_media_.msid_;
		remote_sdp_media_.ssrcs_.push_back(ssrct);
	}

	void media_stream::set_remote_setup(sdp_setup_t setup)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		remote_sdp_media_.setup_ = setup;
	}
	void media_stream::set_local_setup(sdp_setup_t setup)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		local_sdp_media_.setup_ = setup;
	}

	bool media_stream::add_remote_attribute(uint16_t pt, const char* key, const char* val)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		auto itr = remote_sdp_media_.rtpmap_.find(pt);
		if (itr == remote_sdp_media_.rtpmap_.end())
		{
			return false;
		}

		if (sys::string_util::icasecompare(key, "rtcp_fb"))
		{
			itr->second.rtcp_fb_.insert(val);
		}
		else if (sys::string_util::icasecompare(key, "fmtp"))
		{
			itr->second.fmtp_.insert(val);
		}
		else
		{
			itr->second.attrs_.push_back(sdp_pair(key, val,":"));
		}
		return true;
	}

	bool media_stream::clear_remote_attributes(uint16_t pt)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		auto itr = remote_sdp_media_.rtpmap_.find(pt);
		if (itr == remote_sdp_media_.rtpmap_.end())
		{
			return false;
		}
		itr->second.attrs_.clear();
		itr->second.rtcp_fb_.clear();
		itr->second.fmtp_.clear();
		return true;
	}

	bool media_stream::add_local_attribute(uint16_t pt, const char* key, const char* val)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		auto itr=local_sdp_media_.rtpmap_.find(pt);
		if (itr == local_sdp_media_.rtpmap_.end())
		{
			return false;
		}
		
		if (sys::string_util::icasecompare(key,"rtcp_fb"))
		{
			itr->second.rtcp_fb_.insert(val);
		}
		else if (sys::string_util::icasecompare(key, "fmtp"))
		{
			itr->second.fmtp_.insert(val);
		}
		else 
		{
			itr->second.attrs_.push_back(sdp_pair(key, val,":"));
		}
		return true;
	}

	bool media_stream::clear_local_attributes(uint16_t pt)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		auto itr = local_sdp_media_.rtpmap_.find(pt);
		if (itr == local_sdp_media_.rtpmap_.end())
		{
			return false;
		}
		itr->second.attrs_.clear();
		itr->second.rtcp_fb_.clear();
		itr->second.fmtp_.clear();
		return true;
	}

	void media_stream::add_local_candidate(uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority)
	{
		candidate ca;
		ca.foundation_ = foundation;
		ca.component_ = component;
		ca.address_ = address;
		ca.port_ = port;
		ca.priority_ = priority;
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		local_sdp_media_.candidates_.push_back(ca);
	}

	void media_stream::add_remote_candidate(uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority)
	{
		candidate ca;
		ca.foundation_ = foundation;
		ca.component_ = component;
		ca.address_ = address;
		ca.port_ = port;
		ca.priority_ = priority;
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		remote_sdp_media_.candidates_.push_back(ca);
	}

	void media_stream::clear_local_candidates()
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		local_sdp_media_.candidates_.clear();
	}

	void media_stream::clear_remote_candidates()
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		remote_sdp_media_.candidates_.clear();
	}

	void media_stream::enable_dtls()
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
#ifdef LITERTP_SSL
		if (transport_rtp_->enable_security(true))
		{
			local_sdp_media_.protos_.erase("AVP");
			local_sdp_media_.protos_.erase("AVPF");
			local_sdp_media_.protos_.insert("TLS");
			local_sdp_media_.protos_.insert("SAVPF");

			local_sdp_media_.fingerprint_ = transport_rtp_->fingerprint();
		}
#endif
	}

	void media_stream::disable_dtls()
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
#ifdef LITERTP_SSL
		local_sdp_media_.protos_.erase("SAVP");
		local_sdp_media_.protos_.erase("SAVPF");
		local_sdp_media_.protos_.erase("TLS");
		local_sdp_media_.protos_.insert("AVPF");
		local_sdp_media_.fingerprint_ = "";
		if (transport_rtp_)
		{
			transport_rtp_->enable_security(false);
		}
		if (transport_rtcp_)
		{
			transport_rtcp_->enable_security(false);
		}
#endif
	}


	bool media_stream::set_remote_sdp(const sdp_media& sdp, sdp_type_t sdp_type)
	{
		{
			std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
			remote_sdp_media_ = sdp;
		}

		sdp_media sdpm_local = this->get_local_sdp();

		transport_rtp_->ice_ufrag_remote_ = sdp.ice_ufrag_;
		transport_rtp_->ice_pwd_remote_ = sdp.ice_pwd_;
		transport_rtcp_->ice_ufrag_remote_ = sdp.ice_ufrag_;
		transport_rtcp_->ice_pwd_remote_ = sdp.ice_pwd_;

		if (sdp_type == sdp_type_offer)
		{
			//this is the offer from remote end, we can modify the local sdp

			if (sdp.setup_ == sdp_setup_actpass)
			{
				this->set_local_setup(sdp_setup_active);
			}

			if (sdp.trans_mode_ == rtp_trans_mode_recvonly)
			{
				this->set_local_trans_mode(rtp_trans_mode_sendonly);
			}
			else if (sdp.trans_mode_ == rtp_trans_mode_sendonly)
			{
				this->set_local_trans_mode(rtp_trans_mode_recvonly);
			}

			if (sdp.protos_.find("TLS") != sdp.protos_.end())
			{
				enable_dtls();
			}
			else
			{
				disable_dtls();
			}

		}
		else 
		{
			//this is the answer from remote end
			if (!check_setup())
			{
				return false;
			}

			if (!this->negotiate())
			{
				return false;
			}

			{
				std::shared_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
				if (sdp.protos_.find("TLS") != sdp.protos_.end())
				{
					if (local_sdp_media_.protos_.find("TLS") == local_sdp_media_.protos_.end())
					{
						return false;
					}
				}
			}
			
		}

		sockaddr_storage addr_rtp = { 0 };
		if (sys::string_util::icasecompare(sdp.rtp_network_type_, "IP6"))
		{
			sys::socket::ep2addr(AF_INET6, sdp.rtp_address_.c_str(), sdp.rtp_port_, (sockaddr*)&addr_rtp);
		}
		else
		{
			sys::socket::ep2addr(AF_INET, sdp.rtp_address_.c_str(), sdp.rtp_port_, (sockaddr*)&addr_rtp);
		}
		this->set_remote_rtp_endpoint((const sockaddr*)&addr_rtp, sizeof(addr_rtp),1);

		sockaddr_storage addr_rtcp = { 0 };
		if (sys::string_util::icasecompare(sdp.rtcp_network_type_, "IP6"))
		{
			sys::socket::ep2addr(AF_INET6, sdp.rtcp_address_.c_str(), sdp.rtcp_port_, (sockaddr*)&addr_rtcp);
		}
		else
		{
			sys::socket::ep2addr(AF_INET, sdp.rtcp_address_.c_str(), sdp.rtcp_port_, (sockaddr*)&addr_rtcp);
		}
		this->set_remote_rtcp_endpoint((const sockaddr*)&addr_rtcp, sizeof(addr_rtcp), 1);

		return true;
	}

	void media_stream::set_local_sdp(const sdp_media& sdp)
	{
		std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		this->local_sdp_media_ = sdp;
	}

	void media_stream::set_sdp_type(sdp_type_t sdp_type)
	{
		sdp_type_ = sdp_type;
		if (sdp_type_ == sdp_type_offer)
		{
			std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
			local_sdp_media_.setup_ = sdp_setup_actpass;
		}
	}

	void media_stream::set_remote_mid(const char* mid)
	{
		std::unique_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		remote_sdp_media_.mid_ = mid;
	}

	void media_stream::set_remote_rtp_endpoint(const sockaddr* addr, int addr_size, uint32_t priority)
	{
		std::unique_lock<std::shared_mutex>lk(remote_rtp_endpoint_mutex_);
		if (priority > remote_rtp_endpoint_priority)
		{
			memcpy(&remote_rtp_endpoint_, addr, addr_size);
			remote_rtp_endpoint_priority = priority;
		}
	}

	void media_stream::set_remote_rtcp_endpoint(const sockaddr* addr, int addr_size, uint32_t priority)
	{
		std::unique_lock<std::shared_mutex>lk(remote_rtcp_endpoint_mutex_);
		if (priority > remote_rtcp_endpoint_priority) 
		{
			memcpy(&remote_rtcp_endpoint_, addr, addr_size);
			remote_rtcp_endpoint_priority = priority;
		}
	}

	bool media_stream::get_remote_rtp_endpoint(sockaddr_storage* addr)
	{
		std::shared_lock<std::shared_mutex> lk(remote_rtp_endpoint_mutex_);
		if (remote_rtp_endpoint_priority == 0)
		{
			return false;
		}
		memcpy(addr, &remote_rtp_endpoint_, sizeof(remote_rtp_endpoint_));
		return true;
	}

	bool media_stream::get_remote_rtcp_endpoint(sockaddr_storage* addr)
	{
		std::shared_lock<std::shared_mutex>lk(remote_rtcp_endpoint_mutex_);
		if (remote_rtcp_endpoint_priority == 0)
		{
			return false;
		}
		memcpy(addr, &remote_rtcp_endpoint_, sizeof(remote_rtcp_endpoint_));
		return true;
	}

	srtp_role_t media_stream::srtp_role()
	{
		auto sdpm_remote = this->get_remote_sdp();
		auto sdpm_local = this->get_local_sdp();

		if (sdpm_local.setup_ == sdp_setup_active&& (sdpm_remote.setup_ == sdp_setup_actpass || sdpm_remote.setup_ == sdp_setup_passive))
		{
			return srtp_role_client;
		}
		else if (sdpm_local.setup_ == sdp_setup_passive && (sdpm_remote.setup_ == sdp_setup_actpass|| sdpm_remote.setup_ == sdp_setup_active))
		{
			return srtp_role_server;
		}
		else if (sdpm_local.setup_ == sdp_setup_actpass && (sdpm_remote.setup_ == sdp_setup_active))
		{
			return srtp_role_server;
		}
		else if (sdpm_local.setup_ == sdp_setup_actpass && (sdpm_remote.setup_ == sdp_setup_passive))
		{
			return srtp_role_client;
		}
		else
		{
			return srtp_role_client;
		}
	}

	bool media_stream::check_setup()
	{
		auto sdpm_remote = this->get_remote_sdp();
		auto sdpm_local = this->get_local_sdp();
		if (sdpm_local.setup_ == sdp_setup_active && (sdpm_remote.setup_ == sdp_setup_actpass || sdpm_remote.setup_ == sdp_setup_passive))
		{
			return true;
		}
		else if (sdpm_local.setup_ == sdp_setup_passive && (sdpm_remote.setup_ == sdp_setup_actpass || sdpm_remote.setup_ == sdp_setup_active))
		{
			return true;
		}
		else if (sdpm_local.setup_ == sdp_setup_actpass && (sdpm_remote.setup_ == sdp_setup_active))
		{
			return true;
		}
		else if (sdpm_local.setup_ == sdp_setup_actpass && (sdpm_remote.setup_ == sdp_setup_passive))
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool media_stream::negotiate()
	{
		{
			std::unique_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
			std::unique_lock<std::shared_mutex> lk2(remote_sdp_media_mutex_);

			if (sdp_type_ == sdp_type_offer)
			{
				//I am an offer, this called by remote answer
				for (auto itr = local_sdp_media_.rtpmap_.begin(); itr != local_sdp_media_.rtpmap_.end();)
				{
					sdp_format fmt;
					if (!remote_sdp_media_.negotiate(&itr->second, &fmt))
					{
						itr = local_sdp_media_.rtpmap_.erase(itr);
					}
					else
					{
						itr++;
					}
				}

				if (local_sdp_media_.rtpmap_.size() == 0)
				{
					return false;
				}

			}
			else if (sdp_type_ == sdp_type_answer)
			{
				//I am an anwser, this called by remote offer
				for (auto itr = remote_sdp_media_.rtpmap_.begin(); itr != remote_sdp_media_.rtpmap_.end();)
				{
					sdp_format fmt;
					if (!local_sdp_media_.negotiate(&itr->second, &fmt))
					{
						itr = remote_sdp_media_.rtpmap_.erase(itr);
					}
					else
					{
						itr->second.rtcp_fb_.erase("transport-cc");  //not suported
						itr++;
					}
				}



				if (remote_sdp_media_.rtpmap_.size() == 0)
				{
					return false;
				}

				//If not clear this, webrtc stream will be delayed.
				remote_sdp_media_.extmap_.clear();

				//use remote payload type to send
				local_sdp_media_.rtpmap_ = remote_sdp_media_.rtpmap_;

			}
			else
			{
				return false;
			}
		}
		transport_rtp_->srtp_role_ = srtp_role();
		transport_rtcp_->srtp_role_ = srtp_role();
		transport_rtp_->sdp_type_ = sdp_type_;
		transport_rtcp_->sdp_type_ = sdp_type_;
		return true;
	}



	void media_stream::run_rtcp_stats()
	{
		std::string compound_pkt;
		compound_pkt.reserve(2048);
		auto senders=get_senders();
		if (senders.size() > 0)
		{
			for (auto sender : senders) 
			{
				rtcp_sr sr = { 0 };
				sender->prepare_sr(sr);

				auto receivers = get_receivers();
				for (auto receiver : receivers)
				{
					rtcp_report rp;
					receiver->prepare_rr(rp);
					rtcp_sr_add_report(&sr, &rp);
				}

				size_t size = rtcp_sr_size(&sr);
				uint8_t* buf_sr = (uint8_t*)malloc(2048);
				if (buf_sr) {
					rtcp_sr_serialize(&sr, buf_sr, size);
					compound_pkt.append((const char*)buf_sr, size);
					free(buf_sr);
				}
			}
		}
		else
		{
			auto receivers = get_receivers();
			for (auto receiver : receivers)
			{
				rtcp_rr* rr = rtcp_rr_create();
				rr->ssrc = receiver->ssrc();
				rtcp_rr_init(rr);
				rtcp_report rp;
				receiver->prepare_rr(rp);
				rtcp_rr_add_report(rr, &rp);
				size_t size = rtcp_rr_size(rr);
				uint8_t* buf_rr = (uint8_t*)malloc(2048);
				if (buf_rr) {
					rtcp_rr_serialize(rr, buf_rr, size);
					compound_pkt.append((const char*)buf_rr, size);
					free(buf_rr);
				}
				rtcp_rr_free(rr);
			}

			
		}


		rtcp_sdes* sdes = rtcp_sdes_create();
		rtcp_sdes_init(sdes);
		
		auto sdpm_local = this->get_local_sdp();
		for (auto itr : sdpm_local.ssrcs_) 
		{
			rtcp_sdes_add_entry(sdes, itr.ssrc);
			rtcp_sdes_set_item(sdes, itr.ssrc, RTCP_SDES_CNAME, itr.cname.c_str());
		}
		size_t size = rtcp_sdes_size(sdes);
		uint8_t* buf_sdes = (uint8_t*)malloc(size);
		if (buf_sdes) {
			rtcp_sdes_serialize(sdes, buf_sdes, size);
			compound_pkt.append((const char*)buf_sdes, size);
			free(buf_sdes);
		}
		rtcp_sdes_free(sdes);

		send_rtcp_packet((uint8_t*)compound_pkt.data(), (int)compound_pkt.size());



	}

	void media_stream::run_stun_request()
	{
		sockaddr_storage addr_rtp = { 0 };
		sockaddr_storage addr_rtcp = { 0 };

		auto sdpm_remote = this->get_remote_sdp();

		if (get_remote_rtp_endpoint(&addr_rtp) && get_remote_rtcp_endpoint(&addr_rtcp))
		{
			this->get_remote_rtp_endpoint(&addr_rtp);
			transport_rtp_->send_stun_request((const sockaddr*)&addr_rtp, sizeof(addr_rtp), remote_rtp_endpoint_priority);
			if (!sdpm_remote.rtcp_mux_)
			{
				transport_rtcp_->send_stun_request((const sockaddr*)&addr_rtcp, sizeof(addr_rtcp), remote_rtcp_endpoint_priority);
			}
		}
		else
		{
			for (auto& ca : sdpm_remote.candidates_)
			{
				if (ca.component_ == 1)
				{
					sockaddr_storage addr = { 0 };
					sys::socket::ep2addr(AF_INET, ca.address_.c_str(), ca.port_, (sockaddr*)&addr);

					transport_rtp_->send_stun_request((const sockaddr*)&addr, sizeof(addr), ca.priority_);
				}
				else if (ca.component_ == 2)
				{
					sockaddr_storage addr = { 0 };
					sys::socket::ep2addr(AF_INET, ca.address_.c_str(), ca.port_, (sockaddr*)&addr);

					transport_rtcp_->send_stun_request((const sockaddr*)&addr, sizeof(addr), ca.priority_);
				}
			}
		}
		
	}


	void media_stream::send_rtcp_nack(uint32_t ssrc_sender, uint32_t ssrc_media, uint16_t pid, uint16_t blp)
	{
		rtcp_fb fb;
		rtcp_fb_init(&fb, RTCP_RTPFB, RTCP_RTPFB_FMT_NACK);
		fb.ssrc_sender = ssrc_sender;
		fb.ssrc_media = ssrc_media;
		rtcp_rtpfb_nack_set(&fb, pid, blp);

		uint8_t buffer[2048] = { 0 };// size 2048 for srtp
		int size=rtcp_fb_serialize(&fb, buffer,sizeof(buffer));
		if (size>0)
		{
			send_rtcp_packet(buffer, size);
		}

		auto receiver = get_receiver_by_ssrc(ssrc_media);
		if (receiver)
		{
			receiver->increase_nack();
		}
	}


	void media_stream::send_rtcp_keyframe(uint32_t ssrc_media)
	{
		auto sdpm_remote = get_remote_sdp();
		uint32_t ssrc=get_local_ssrc();
		
		for (auto itr_fmt = sdpm_remote.rtpmap_.begin(); itr_fmt != sdpm_remote.rtpmap_.end(); itr_fmt++)
		{
			if (itr_fmt->second.rtcp_fb_.find("nack pli") != itr_fmt->second.rtcp_fb_.end())
			{
				send_rtcp_keyframe_pli(ssrc, ssrc_media);
			}
			else if (itr_fmt->second.rtcp_fb_.find("ccm fir") != itr_fmt->second.rtcp_fb_.end())
			{
				send_rtcp_keyframe_fir(ssrc, ssrc_media);
			}
			else
			{
				send_rtcp_keyframe_pli(ssrc, ssrc_media);
			}
		}
	}

	void media_stream::send_rtcp_keyframe(uint32_t ssrc_media, uint16_t pt)
	{
		auto sdpm_remote = get_remote_sdp();

		uint32_t ssrc = get_local_ssrc();

		auto itr_fmt = sdpm_remote.rtpmap_.find(pt);
		if (itr_fmt == sdpm_remote.rtpmap_.end())
		{
			return;
		}

		if (itr_fmt->second.rtcp_fb_.find("nack pli") == itr_fmt->second.rtcp_fb_.end())
		{
			send_rtcp_keyframe_pli(ssrc, ssrc_media);
		}
		else if (itr_fmt->second.rtcp_fb_.find("ccm fir") == itr_fmt->second.rtcp_fb_.end())
		{
			send_rtcp_keyframe_fir(ssrc, ssrc_media);
		}
		else
		{
			send_rtcp_keyframe_pli(ssrc, ssrc_media);
		}
	}

	void media_stream::send_rtcp_keyframe_pli(uint32_t ssrc_sender, uint32_t ssrc_media)
	{
		auto receiver = get_receiver_by_ssrc(ssrc_media);
		if (receiver)
		{
			receiver->increase_pli();
		}

		rtcp_fb fb;
		rtcp_fb_init(&fb, RTCP_PSFB, RTCP_PSFB_FMT_PLI);
		fb.ssrc_sender = ssrc_sender;
		fb.ssrc_media = ssrc_media;
		uint8_t buffer[2048] = { 0 };// size 2048 for srtp
		int size=rtcp_fb_serialize(&fb, buffer, sizeof(buffer));
		if (size > 0) 
		{
			send_rtcp_packet(buffer, size);
		}
	}

	void media_stream::send_rtcp_keyframe_fir(uint32_t ssrc_sender, uint32_t ssrc_media)
	{
		auto receiver = get_receiver_by_ssrc(ssrc_media);

		rtcp_fb fb;
		rtcp_fb_init(&fb, RTCP_PSFB, RTCP_PSFB_FMT_FIR);
		fb.ssrc_sender = ssrc_sender;
		fb.ssrc_media = ssrc_media;
		
		uint8_t fci[8] = { 0 };
		fci[0] = (ssrc_media & 0xFF000000) >> 24;
		fci[1] = (ssrc_media & 0x00FF0000) >> 16;
		fci[2] = (ssrc_media & 0x0000FF00) >> 8;
		fci[3] = ssrc_media & 0x000000FF;
		if (receiver) 
		{
			fci[4] = receiver->fir_count();
			receiver->increase_fir();
		}
		rtcp_fb_set_fci(&fb, fci, sizeof(fci));

		uint8_t buffer[2048] = { 0 };// size 2048 for srtp
		int size = rtcp_fb_serialize(&fb, buffer, sizeof(buffer));
		if (size > 0)
		{
			send_rtcp_packet(buffer, size);
		}


	}

	void media_stream::send_rtcp_bye(const char* reason)
	{
		rtcp_bye* pkt=rtcp_bye_create();
		pkt->message = (char*)reason;
		
		auto senders = this->get_senders();
		
		pkt->src_ids = new uint32_t[senders.size()]();
		for (auto sender:senders)
		{
			rtcp_bye_add_source(pkt, sender->ssrc());
		}
		
		uint8_t buffer[2048] = { 0 };
		int size=rtcp_bye_serialize(pkt, buffer, sizeof(buffer));
		if (size > 0)
		{
			send_rtcp_packet(buffer, size);
		}
	}


	bool media_stream::send_rtp_packet(packet_ptr packet)
	{
		sockaddr_storage addr = { 0 };
		this->get_remote_rtp_endpoint(&addr);

		return transport_rtp_->send_rtp_packet(packet, (const sockaddr*)&addr, sizeof(addr));
	}

	bool media_stream::send_rtcp_packet(uint8_t* rtcp_data, int size)
	{
		sockaddr_storage addr = { 0 };
		this->get_remote_rtcp_endpoint(&addr);

		return transport_rtcp_->send_rtcp_packet(rtcp_data, size, (const sockaddr*)&addr, sizeof(addr));
	}

	media_type_t media_stream::media_type()
	{
		std::shared_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		return local_sdp_media_.media_type_;
	}

	bool media_stream::send_frame(const uint8_t* frame, uint32_t size, uint32_t duration)
	{
		auto sender= get_default_sender();
		if (!sender)
		{
			return false;
		}

		return sender->send_frame(frame, size, duration);
	}


	sdp_media media_stream::get_local_sdp()
	{
		std::shared_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		return local_sdp_media_;

	}

	sdp_media media_stream::get_remote_sdp()
	{
		std::shared_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		return remote_sdp_media_;
	}

	bool media_stream::get_local_format(int pt, sdp_format& fmt)
	{
		std::shared_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		auto itr=local_sdp_media_.rtpmap_.find(pt);
		if (itr == local_sdp_media_.rtpmap_.end())
		{
			return false;
		}

		fmt = itr->second;
		return true;
	}
	bool media_stream::get_remote_format(int pt, sdp_format& fmt)
	{
		std::shared_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		auto itr = remote_sdp_media_.rtpmap_.find(pt);
		if (itr == remote_sdp_media_.rtpmap_.end())
		{
			return false;
		}

		fmt = itr->second;
		return true;
	}

	uint32_t media_stream::get_local_ssrc(int idx)
	{
		std::shared_lock<std::shared_mutex> lk(local_sdp_media_mutex_);
		if (idx < 0 || idx >= local_sdp_media_.ssrcs_.size())
		{
			return 0;
		}

		return local_sdp_media_.ssrcs_[idx].ssrc;
	}
	uint32_t media_stream::get_remote_ssrc(int idx)
	{
		std::shared_lock<std::shared_mutex> lk(remote_sdp_media_mutex_);
		if (idx < 0 || idx >= remote_sdp_media_.ssrcs_.size())
		{
			return 0;
		}

		return remote_sdp_media_.ssrcs_[idx].ssrc;
	}

	void media_stream::get_stats(rtp_stats_t& stats)
	{
		memset(&stats, 0, sizeof(stats));
		auto sender=this->get_default_sender();
		auto sdp_local = this->get_local_sdp();

		uint16_t pt = 0;
		
		if (sender) 
		{
			pt=sender->format().payload_type_;
		}
		
		if (pt == 0)
		{
			auto itr = sdp_local.rtpmap_.begin();
			if (itr != sdp_local.rtpmap_.end())
			{
				pt = itr->first;
			}
		}


		stats.pt = pt;
		stats.mt = sdp_local.media_type_;

		receiver_ptr receiver = this->get_receiver(pt);

		if (sender)
		{
			stats.ct = sender->format().codec_;
			sender->get_stats(stats.sender_stats);
		}

		if (receiver)
		{
			if (stats.ct == codec_type_unknown)
			{
				stats.ct = receiver->format().codec_;
			}
			
			receiver->get_stats(stats.receiver_stats);
		}
		
	}






	sender_ptr media_stream::get_sender(int pt)
	{
		{
			std::shared_lock<std::shared_mutex>lk(senders_mutex_);
			auto itr = senders_.find(pt);
			if (itr != senders_.end())
			{
				return itr->second;
			}
		}


		sdp_format fmt;
		if (!get_local_format(pt, fmt))
		{
			return nullptr;
		}

		return create_sender(fmt);

	}

	sender_ptr media_stream::get_sender_by_ssrc(uint32_t ssrc)
	{
		std::shared_lock<std::shared_mutex>lk(senders_mutex_);
		for (auto itr = senders_.begin(); itr != senders_.end(); itr++)
		{
			if (itr->second->ssrc() == ssrc)
			{
				return itr->second;
			}
		}
		return nullptr;
	}

	sender_ptr media_stream::get_default_sender()
	{
		{
			std::shared_lock<std::shared_mutex>lk(senders_mutex_);
			auto itr = senders_.begin();
			if (itr != senders_.end())
			{
				return itr->second;
			}
		}



		auto sdp = this->get_local_sdp();
		auto itr_fmt = sdp.rtpmap_.begin();
		if (itr_fmt == sdp.rtpmap_.end())
		{
			return nullptr;
		}


		return create_sender(itr_fmt->second);
	}

	sender_ptr media_stream::create_sender(const sdp_format& fmt)
	{
		std::unique_lock<std::shared_mutex>lk(senders_mutex_);

		uint32_t ssrc = this->get_local_ssrc();

		sender_ptr sender;
		if (fmt.codec_ == codec_type_h264)
		{
			sender = std::make_shared<sender_video_h264>(ssrc, media_type(), fmt);
		}
		else if (fmt.codec_ == codec_type_vp8)
		{
			sender = std::make_shared <sender_video_vp8>(ssrc, media_type(), fmt);
		}
		else if (fmt.codec_ == codec_type_mpeg4_generic || fmt.codec_ == codec_type_mp4a_latm)
		{
			sender = std::make_shared<sender_audio_aac>(ssrc, media_type(), fmt);
		}
		else
		{
			sender = std::make_shared<sender_audio>(ssrc, media_type(), fmt);
		}

		sender->send_rtp_packet_event_.add(s_send_rtp_packet_event, this);

		senders_.insert(std::make_pair(fmt.payload_type_, sender));

		return sender;
	}

	std::vector<sender_ptr> media_stream::get_senders()
	{
		std::shared_lock<std::shared_mutex>lk(senders_mutex_);

		std::vector<sender_ptr> vec;
		vec.reserve(senders_.size());
		for (auto sender : senders_)
		{
			vec.push_back(sender.second);
		}
		return vec;
	}

	receiver_ptr media_stream::get_receiver(int pt)
	{
		{
			std::shared_lock<std::shared_mutex>lk(receivers_mutex_);
			auto itr = receivers_.find(pt);
			if (itr != receivers_.end())
			{
				return itr->second;
			}
		}

		{
			std::unique_lock<std::shared_mutex>lk(receivers_mutex_);
			sdp_format fmt;
			if (!get_remote_format(pt, fmt))
			{
				return nullptr;
			}
			uint32_t ssrc = get_remote_ssrc();

			receiver_ptr receiver;
			if (fmt.codec_ == codec_type_h264)
			{
				receiver = std::make_shared<receiver_video_h264>(ssrc, media_type_video, fmt);
			}
			else if (fmt.codec_ == codec_type_vp8)
			{
				receiver = std::make_shared <receiver_video_vp8>(ssrc, media_type_video, fmt);
			}
			else if (fmt.codec_ == codec_type_mpeg4_generic || fmt.codec_ == codec_type_mp4a_latm)
			{
				receiver = std::make_shared <receiver_audio_aac>(ssrc, media_type_video, fmt);
			}
			else
			{
				receiver = std::make_shared<receiver_audio>(ssrc, media_type_audio, fmt);
			}
			receiver->rtp_frame_event_.add(s_rtp_frame_event, this);
			receiver->rtp_nack_event_.add(s_rtp_nack_event, this);
			receiver->rtp_keyframe_event_.add(s_rtp_keyframe_event, this);
			receivers_.insert(std::make_pair(pt, receiver));

			return receiver;
		}
	}

	receiver_ptr media_stream::get_receiver_by_ssrc(uint32_t ssrc)
	{
		std::shared_lock<std::shared_mutex>lk(receivers_mutex_);
		for (auto itr = receivers_.begin(); itr != receivers_.end(); itr++)
		{
			if (itr->second->ssrc() == ssrc)
			{
				return itr->second;
			}
		}
		return nullptr;
	}

	std::vector<receiver_ptr> media_stream::get_receivers()
	{
		std::shared_lock<std::shared_mutex>lk(receivers_mutex_);

		std::vector<receiver_ptr> vec;
		vec.reserve(receivers_.size());
		for (auto receiver : receivers_)
		{
			vec.push_back(receiver.second);
		}
		return vec;
	}

	bool media_stream::has_remote_ssrc(uint32_t ssrc)
	{
		std::shared_lock<std::shared_mutex>lk(remote_sdp_media_mutex_);
		return remote_sdp_media_.has_ssrc(ssrc);
	}


	//bool random()
	//{
	//	int n = sys::util::random_number(0, 100);
	//	if (n <= 10)
	//	{
	//		return true;
	//	}
	//	return false;
	//}

	void media_stream::s_transport_rtp_packet(void* ctx, std::shared_ptr<sys::socket> skt, packet_ptr packet, const sockaddr* addr, int addr_size)
	{
		media_stream* p = (media_stream*)ctx;

		// no need to check ssrc
		//if (!p->has_remote_ssrc(packet->handle_->header->ssrc))
		//{
		//	return;
		//}


		auto receiver = p->get_receiver(packet->handle_->header->pt);
		if (!receiver)
		{
			return;
		}
		receiver->insert_packet(packet);

		
	}

	void media_stream::s_transport_rtcp_packet(void* ctx, std::shared_ptr<sys::socket> skt, uint16_t pt, const uint8_t* buffer, size_t size, const sockaddr* addr, int addr_size)
	{
		media_stream* p = (media_stream*)ctx;

		if (pt == rtcp_packet_type::RTCP_APP)
		{
			auto app = rtcp_app_create();
			if (rtcp_app_parse(app, buffer, size) >= 0)
			{
				if (p->has_remote_ssrc(app->ssrc))
				{
					p->on_rtcp_app(app);
				}
			}
			rtcp_app_free(app);
		}
		else if (pt == rtcp_packet_type::RTCP_BYE)
		{
			auto bye = rtcp_bye_create();
			if (rtcp_bye_parse(bye, buffer, size) >= 0)
			{
				for (int i = 0; i < bye->header.common.count; i++)
				{
					if (p->has_remote_ssrc(bye->src_ids[i])) 
					{
						p->on_rtcp_bye(bye);
					}
				}
			}
			rtcp_bye_free(bye);
		}
		else if (pt == rtcp_packet_type::RTCP_RR)
		{
			auto rr = rtcp_rr_create();
			if (rtcp_rr_parse(rr, buffer, size) >= 0)
			{
				if (p->has_remote_ssrc(rr->ssrc))
				{
					p->on_rtcp_rr(rr);
				}
			}
			rtcp_rr_free(rr);
		}
		else if (pt == rtcp_packet_type::RTCP_SR)
		{
			auto sr = rtcp_sr_create();
			if (rtcp_sr_parse(sr, buffer, size) >= 0)
			{
				if (p->has_remote_ssrc(sr->ssrc))
				{
					p->on_rtcp_sr(sr);
				}
			}
			rtcp_sr_free(sr);
		}
		else if (pt == rtcp_packet_type::RTCP_SDES)
		{
			auto sdes = rtcp_sdes_create();
			if (rtcp_sdes_parse(sdes, buffer, size) >= 0)
			{
				p->on_rtcp_sdes(sdes);
			}
			rtcp_sdes_free(sdes);
		}
		else if (pt == rtcp_packet_type::RTCP_RTPFB)
		{
			auto fb = rtcp_fb_create();
			if (rtcp_fb_parse(fb, buffer, size) >= 0)
			{
				if(p->has_remote_ssrc(fb->ssrc_sender))
				{
					if (fb->header.app.subtype == RTCP_RTPFB_FMT_NACK)
					{
						uint16_t pid = 0, bld = 0;
						rtcp_rtpfb_nack_get(fb, &pid, &bld);

						p->on_rtcp_nack(fb->ssrc_media, pid, bld);
					}
				}
			}
			rtcp_fb_free(fb);
		}
		else if (pt == rtcp_packet_type::RTCP_PSFB)
		{
			auto fb = rtcp_fb_create();
			if (rtcp_fb_parse(fb, buffer, size) >= 0)
			{
				if (p->has_remote_ssrc(fb->ssrc_sender))
				{
					if (fb->header.app.subtype == RTCP_PSFB_FMT_PLI)
					{
						if (fb->ssrc_media == p->get_local_ssrc(0))
						{
							p->on_rtcp_pli(fb->ssrc_media);
						}
						else
						{
							LOGW("received unmatched ssrc pli,sender=%u,media=%u", fb->ssrc_sender, fb->ssrc_media);
						}
					}
					else if (fb->header.app.subtype == RTCP_PSFB_FMT_FIR)
					{
						if (fb->ssrc_media == p->get_local_ssrc(0))
						{
							rtcp_psfb_fir_item item;
							rtcp_psfb_fir_get_item(fb, 0, &item);
							p->on_rtcp_fir(item.ssrc, item.seq_nr);
						}
						else
						{
							LOGW("received unmatched ssrc fir,sender=%u,media=%u", fb->ssrc_sender, fb->ssrc_media);
						}
					}
				}
			}
			rtcp_fb_free(fb);
		}
	}

	void media_stream::s_transport_stun_message(void* ctx, std::shared_ptr<sys::socket> skt, const stun_message& msg, const sockaddr* addr, int addr_size)
	{
		if (msg.type_ == stun_message_type_binding_request)
		{
			std::string spriority=msg.get_attribute(stun_attribute_type_priority);
			uint32_t priority = 0;
			if (spriority.size() >= 4)
			{
				priority |= ((uint8_t)spriority[0]) << 24;
				priority |= ((uint8_t)spriority[1]) << 16;
				priority |= ((uint8_t)spriority[2]) << 8;
				priority |= ((uint8_t)spriority[3]);
			}

			media_stream* p = (media_stream*)ctx;
			if (p->transport_rtp_->socket_ == skt)
			{
				p->set_remote_rtp_endpoint(addr, addr_size, priority);
			}

			if (p->transport_rtcp_->socket_ == skt)
			{
				p->set_remote_rtcp_endpoint(addr, addr_size, priority);
			}
		}
	}

	static void s_transport_disconnect(void* ctx)
	{
		media_stream* p = (media_stream*)ctx;
		p->litertp_on_tcp_disconnect_.invoke(p->media_type());
	}

	void media_stream::on_rtcp_app(const rtcp_app* app)
	{
		litertp_on_rtcp_app_.invoke(app->ssrc, app->name, (const char*)app->app_data, app->app_size);
	}

	void media_stream::on_rtcp_bye(const rtcp_bye* bye)
	{
		litertp_on_rtcp_bye_.invoke(bye->src_ids,bye->header.common.count, (const char*)bye->message);
	}

	void media_stream::on_rtcp_sr(const rtcp_sr* sr)
	{
		auto senders = get_senders();
		for (auto sender : senders)
		{
			rtcp_report* report=rtcp_sr_find_report(sr,sender->ssrc());
			if (report)
			{
				sender->update_remote_report(*report);
			}
		}

		auto receivers = get_receivers();
		for (auto receiver : receivers)
		{
			if (receiver->ssrc() == sr->ssrc)
			{
				receiver->update_remote_sr(*sr);
			}
		}

		LOGT("ssrc %d send report", sr->ssrc);
	}

	void media_stream::on_rtcp_rr(const rtcp_rr* rr)
	{

		auto senders = get_senders();
		for (auto sender : senders)
		{
			rtcp_report* report = rtcp_rr_find_report(rr, sender->ssrc());
			if (report)
			{
				sender->update_remote_report(*report);
			}
		}


		LOGT("ssrc %d receive report", rr->ssrc);
	}

	void media_stream::on_rtcp_sdes(const rtcp_sdes* sdes)
	{

	}

	void media_stream::on_rtcp_nack(uint32_t ssrc, uint16_t pid, uint16_t bld)
	{
		auto sender=this->get_default_sender();
		if (sender)
		{
			auto pkt = sender->get_history(pid);
			if (pkt)
			{
				this->send_rtp_packet(pkt);
			}

			for (int i = 0; i < 16; i++)
			{
				bld = bld >> i;
				if (bld & 0x0001)
				{
					pkt = sender->get_history(pid + i + 1);
					if (pkt)
					{
						this->send_rtp_packet(pkt);
					}
				}
			}

			sender->increase_nack();
		}

	}

	void media_stream::on_rtcp_pli(uint32_t ssrc)
	{
		LOGD("ssrc %d required keyframe by pli",ssrc);

		auto sender = this->get_default_sender();
		if (sender)
		{
			sender->increase_pli();
		}

		litertp_on_keyframe_required_.invoke(ssrc, 0);
	}

	void media_stream::on_rtcp_fir(uint32_t ssrc, uint8_t nr)
	{
		LOGD("ssrc %d required keyframe by fir",ssrc);

		auto sender = this->get_default_sender();
		if (sender)
		{
			sender->increase_fir();
		}

		litertp_on_keyframe_required_.invoke(ssrc, 1);
	}




	void media_stream::s_rtp_frame_event(void* ctx, uint32_t ssrc, const sdp_format& fmt, const av_frame_t& frame)
	{
		media_stream* p = (media_stream*)ctx;
		p->on_rtp_frame_event(ssrc, fmt, frame);
	}
	void media_stream::on_rtp_frame_event(uint32_t ssrc, const sdp_format& fmt, const av_frame_t& frame)
	{
		litertp_on_frame_.invoke(ssrc,fmt.payload_type_,fmt.frequency_,fmt.channels_,&frame);
	}


	void media_stream::s_rtp_nack_event(void* ctx, uint32_t ssrc, const sdp_format& fmt, uint16_t pid, uint16_t bld)
	{
		media_stream* p = (media_stream*)ctx;
		p->on_rtp_nack_event(ssrc, fmt, pid, bld);
	}
	void media_stream::on_rtp_nack_event(uint32_t ssrc, const sdp_format& fmt, uint16_t pid, uint16_t bld)
	{
		LOGD("send nack pid=%u bld=%x\n",pid,bld);
		this->send_rtcp_nack(get_local_ssrc(), ssrc, pid, bld);
	}


	void media_stream::s_rtp_keyframe_event(void* ctx, uint32_t ssrc, const sdp_format& fmt)
	{
		media_stream* p = (media_stream*)ctx;
		p->on_rtp_keyframe_event(ssrc, fmt);
	}
	void media_stream::on_rtp_keyframe_event(uint32_t ssrc, const sdp_format& fmt)
	{
		this->send_rtcp_keyframe(ssrc,fmt.payload_type_);
	}

	void media_stream::s_send_rtp_packet_event(void* ctx, packet_ptr packet)
	{
		media_stream* p = (media_stream*)ctx;
		p->send_rtp_packet(packet);
	}



	

}
