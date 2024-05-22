/**
 * @file sdp_media.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sdp_media.h"
#include "sys2/string_util.h"

#include <algorithm>

namespace litertp {


	sdp_media::sdp_media()
	{
		protos_.insert("UDP");
		protos_.insert("RTP");
		protos_.insert("AVP");
	}

	sdp_media::~sdp_media()
	{
	}

	bool sdp_media::parse_header(const std::string& m)
	{
		std::vector<std::string> ss=sys::string_util::split(m, " ");
		if (ss.size() < 3)
		{
			return false;
		}

		//media type
		if (ss[0] == "video")
		{
			media_type_ = media_type_video;
		}
		else if (ss[0] == "audio")
		{
			media_type_ = media_type_audio;
		}
		else
		{
			media_type_ = media_type_data;
		}

		//port
		rtp_port_ = atoi(ss[1].c_str());

		//protos
		protos_.clear();
		std::vector<std::string> protos = sys::string_util::split(ss[2], "/");
		for (auto proto : protos)
		{
			std::transform(proto.begin(), proto.end(), proto.begin(), ::toupper);
			protos_.insert(proto);
		}

		//payload type
		rtpmap_.clear();
		for (int i = 3; i < ss.size(); i++)
		{
			sdp_format fmt;
			fmt.payload_type_ = atoi(ss[i].c_str());
			rtpmap_.insert(std::make_pair(fmt.payload_type_,fmt));
		}

		return true;
	}

	bool sdp_media::parse_c(const std::string& c)
	{
		std::vector<std::string> ss=sys::string_util::split(c, " ");
		if (ss.size() < 3)
		{
			return false;
		}
		this->rtp_network_type_ = ss[0];
		this->rtp_address_type_ = ss[1];
		this->rtp_address_ = ss[2];
		return true;
	}

	bool sdp_media::parse_a(const std::string& s)
	{
		sdp_pair attr(":");
		if (!attr.parse(s))
		{
			return false;
		}

		if (attr.key_ == "rtcp")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ");
			if (ss.size() >= 4)
			{
				this->rtcp_port_ = atoi(ss[0].c_str());
				this->rtcp_network_type_ = ss[1];
				this->rtcp_address_type_ = ss[2];
				this->rtcp_address_ = ss[3];
			}
		}
		else if (attr.key_ == "ice-ufrag")
		{
			this->ice_ufrag_ = attr.val_;
		}
		else if (attr.key_ == "ice-pwd")
		{
			this->ice_pwd_ = attr.val_;
		}
		else if (attr.key_ == "ice-options")
		{
			this->ice_options_ = attr.val_;
		}
		else if (attr.key_ == "fingerprint")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			if (ss.size() >= 2) 
			{
				this->fingerprint_sign_ = ss[0];
				this->fingerprint_ = ss[1];
			}
		}
		else if (attr.key_ == "setup")
		{
			if (attr.val_ == "actpass")
			{
				this->setup_ = sdp_setup_actpass;
			}
			else if (attr.val_ == "active")
			{
				this->setup_ = sdp_setup_active;
			}
			else if (attr.val_ == "passive")
			{
				this->setup_ = sdp_setup_passive;
			}
		}
		else if (attr.key_ == "mid")
		{
			this->mid_ = attr.val_;
		}
		else if (attr.key_ == "msid")
		{
			if (attr.val_.find('-') == 0)
			{
				attr.val_.erase(0, 1);
			}
			sys::string_util::trim(attr.val_);
			this->msid_ = attr.val_;
		}
		else if (attr.key_ == "extmap")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			if (ss.size() >= 2)
			{
				int pt = atoi(ss[0].c_str());
				auto iter=extmap_.insert(std::make_pair(pt, ss[1]));
				if (!iter.second)
				{
					iter.first->second = ss[1];
				}
			}
		}
		else if (attr.key_ == "sendonly")
		{
			this->trans_mode_ = rtp_trans_mode_sendonly;
		}
		else if (attr.key_ == "recvonly")
		{
			this->trans_mode_ = rtp_trans_mode_recvonly;
		}
		else if (attr.key_ == "sendrecv")
		{
			this->trans_mode_ = rtp_trans_mode_sendrecv;
		}
		else if (attr.key_=="rtcp-mux")
		{
			this->rtcp_mux_ = true;
		}
		else if (attr.key_=="rtcp-rsize")
		{
			this->rtcp_rsize_ = true;
		}
		else if (attr.key_ == "candidate")
		{
			candidate cd;
			if (cd.parse(attr.val_))
			{
				candidates_.push_back(cd);
			}
		}
		else if (attr.key_=="rtpmap")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			if (ss.size() >= 2)
			{
				int pt = atoi(ss[0].c_str());
				std::vector<std::string> ss2= sys::string_util::split(ss[1], "/");

				auto iter=this->rtpmap_.find(pt);
				if (iter != this->rtpmap_.end())
				{
					if (ss2.size() >= 1)
					{
						iter->second.set_codec(ss2[0]);
					}
					if (ss2.size() >= 2)
					{
						iter->second.frequency_ = atoi(ss2[1].c_str());
					}
					if (ss2.size() >= 3)
					{
						iter->second.channels_ = atoi(ss2[2].c_str());
					}
				}
			}
		}
		else if (attr.key_ == "rtcp-fb")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			if (ss.size() >= 2)
			{
				int pt = atoi(ss[0].c_str());
				auto iter = this->rtpmap_.find(pt);
				if (iter != this->rtpmap_.end())
				{
					iter->second.rtcp_fb_.insert(ss[1]);
				}
			}
			
		}
		else if (attr.key_ == "fmtp")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			if (ss.size() >= 2)
			{
				int pt = atoi(ss[0].c_str());
				auto iter = this->rtpmap_.find(pt);
				if (iter != this->rtpmap_.end())
				{
					iter->second.fmtp_.insert(ss[1]);
				}
			}
		}
		else if (attr.key_ == "ssrc-group")
		{
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ");
			this->ssrc_group_ = ss[0];
			for (int i = 1; i < ss.size(); i++)
			{
				ssrc_t ssrc;
				char* endptr = nullptr;
				ssrc.ssrc = strtoul(ss[i].c_str(),&endptr,0);


				auto itr=std::find_if(ssrcs_.begin(), ssrcs_.end(), [&ssrc](auto& a){
					return a.ssrc==ssrc.ssrc;
				});

				if (itr == ssrcs_.end())
				{
					ssrcs_.push_back(ssrc);
				}
			}
		}
		else if (attr.key_ == "ssrc")
		{
			ssrc_t ssrc;
			std::vector<std::string> ss = sys::string_util::split(attr.val_, " ",2);
			char* endptr = nullptr;
			ssrc.ssrc = strtoul(ss[0].c_str(),&endptr,0);

			auto itr = std::find_if(ssrcs_.begin(), ssrcs_.end(), [&ssrc](auto& a){
				return a.ssrc == ssrc.ssrc;
			});
			if (itr == ssrcs_.end())
			{
				itr = ssrcs_.insert(ssrcs_.end(), ssrc);
			}


			if (ss.size() > 1)
			{
				std::vector<std::string> ss2 = sys::string_util::split(ss[1], ":",2);
				if (ss2.size() >= 2)
				{
					const std::string& key = ss2[0];
					std::string& val = ss2[1];
					if (key == "cname")
					{
						itr->cname = val;
					}
					else if (key == "msid")
					{
						if (val.find('-') ==0)
						{
							val.erase(0, 1);
						}
						sys::string_util::trim(val);
						itr->msid = val;
					}
				}
			}
			

		}
		else
		{
			return false;
		}

		return true;
	}

	//protocols need to order
	void sdp_media::to_protocols_string(std::stringstream& ss)const
	{
		bool has_protocal = false;
		if (protos_.find("UDP") != protos_.end())
		{
			ss << "UDP/";
			has_protocal = true;
		}
		else if (protos_.find("TCP") != protos_.end())
		{
			ss << "TCP/";
			has_protocal = true;
		}


		if (protos_.find("TLS") != protos_.end())
		{
			ss << "TLS/";
			has_protocal = true;
		}


		if (protos_.find("RTP") != protos_.end())
		{
			ss << "RTP/";
			has_protocal = true;
		}


		if (protos_.find("SAVPF") != protos_.end())
		{
			ss << "SAVPF/";
			has_protocal = true;
		}
		else if (protos_.find("AVPF") != protos_.end())
		{
			ss << "AVPF/";
			has_protocal = true;
		}
		else if (protos_.find("SAVP") != protos_.end())
		{
			ss << "SAVP/";
			has_protocal = true;
		}
		else if (protos_.find("AVP") != protos_.end())
		{
			ss << "AVP/";
			has_protocal = true;
		}

		
		if (has_protocal)
		{
			ss.seekp(-1, std::ios_base::end);
		}
	}

	void sdp_media::to_string(std::stringstream& ss)const
	{
		//m
		ss << "m=";
		if (media_type_ == media_type_audio)
		{
			ss << "audio";
		}
		else if (media_type_ == media_type_video)
		{
			ss << "video";
		}
		else
		{
			ss << "message";
		}
		
		ss<< " " << rtp_port_ << " ";
		to_protocols_string(ss);
		ss << " ";

		for (auto fmt : rtpmap_)
		{
			ss << fmt.second.payload_type_ << " ";
		}
		if (protos_.size() > 0)
		{
			ss.seekp(-1, std::ios_base::end);
		}
		ss << std::endl;

		//c
		ss << "c=" << rtp_network_type_ << " " << rtp_address_type_ << " " << rtp_address_ << std::endl;

		ss << "a=rtcp:" << rtcp_port_ << " " << rtcp_network_type_ << " " << rtcp_address_type_ << " " << rtcp_address_ << std::endl;

		for (auto& ca : candidates_)
		{
			ss << "a=candidate:" << ca.to_string() << std::endl;
		}

		if (ice_ufrag_.size() > 0) 
		{
			ss << "a=ice-ufrag:" << ice_ufrag_ << std::endl;
		}
		if (ice_pwd_.size() > 0)
		{
			ss << "a=ice-pwd:" << ice_pwd_ << std::endl;
		}
		if (ice_options_.size() > 0) 
		{
			ss << "a=ice-options:" << ice_options_ << std::endl;
		}
		if (fingerprint_.size() > 0)
		{
			ss << "a=fingerprint:" << fingerprint_sign_ << " " << fingerprint_ << std::endl;
		}


		if (setup_ == sdp_setup_active)
		{
			ss << "a=setup:active";
		}
		else if (setup_ == sdp_setup_actpass)
		{
			ss << "a=setup:actpass";
		}
		else if (setup_ == sdp_setup_passive)
		{
			ss << "a=setup:passive";
		}
		ss << std::endl;
		if (mid_.size() > 0) 
		{
			ss << "a=mid:" << mid_ << std::endl;
		}
		if (msid_.size() > 0)
		{
			ss << "a=msid:- " << msid_ << std::endl;
		}
		if (rtcp_mux_)
		{
			ss << "a=rtcp-mux" << std::endl;
		}
		if (rtcp_rsize_)
		{
			ss << "a=rtcp-rsize" << std::endl;
		}


		for (auto& extmap : extmap_)
		{
			ss << "a=extmap:" << extmap.first << " " << extmap.second << std::endl;
		}

		if (trans_mode_ == rtp_trans_mode_inactive) 
		{
			ss << "a=inactive" << std::endl;
		}
		else if (trans_mode_ == rtp_trans_mode_recvonly)
		{
			ss << "a=recvonly" << std::endl;
		}
		else if(trans_mode_ == rtp_trans_mode_sendonly)
		{
			ss << "a=sendonly" << std::endl;
		}
		else if (trans_mode_ == rtp_trans_mode_sendrecv)
		{
			ss << "a=sendrecv" << std::endl;
		}

		for (auto& fmt : rtpmap_)
		{
			fmt.second.to_string(ss);
		}

		if (ssrc_group_.size() > 0)
		{
			ss << "a=ssrc-group:" << ssrc_group_;
			for (auto& ssrct : ssrcs_)
			{
				ss << " " << ssrct.ssrc;
			}
			ss << std::endl;
		}

		for (auto& ssrct : ssrcs_)
		{
			if (ssrct.cname.size() == 0 && ssrct.msid.size() == 0)
			{
				ss << "a=ssrc:" << ssrct.ssrc << std::endl;
			}
			else
			{
				if (ssrct.cname.size() > 0)
				{
					ss << "a=ssrc:" << ssrct.ssrc << " cname:" << ssrct.cname << std::endl;
				}
				if (ssrct.msid.size() > 0)
				{
					ss << "a=ssrc:" << ssrct.ssrc << " msid:- " << ssrct.msid << std::endl;
				}
			}
		}

		for (auto& attr : attrs_)
		{
			ss << "a=" << attr.to_string() << std::endl;
		}

	}


	bool sdp_media::negotiate(const sdp_format* fmt_in, sdp_format* fmt_out)
	{
		for (auto& itr : rtpmap_)
		{
			if (itr.second.codec_ == fmt_in->codec_ && itr.second.frequency_ == fmt_in->frequency_)
			{
				int level_asymmetry_allowed = 0, packetization_mode = 0;
				int64_t profile_level_id = 0;
				if (fmt_in->extract_h264_fmtp(&level_asymmetry_allowed, &packetization_mode, &profile_level_id))
				{
					int level_asymmetry_allowed2 = 0, packetization_mode2 = 0;
					int64_t profile_level_id2 = 0;
					if (itr.second.extract_h264_fmtp(&level_asymmetry_allowed2, &packetization_mode2, &profile_level_id2))
					{
						if (level_asymmetry_allowed >= 0 && level_asymmetry_allowed2 >= 0 && level_asymmetry_allowed != level_asymmetry_allowed2)
						{
							continue;
						}

						if (packetization_mode >= 0 && packetization_mode2 >= 0 && packetization_mode != packetization_mode2)
						{
							continue;
						}

						if (profile_level_id >= 0 && profile_level_id2 >= 0 && profile_level_id != profile_level_id2)
						{
							continue;
						}
					}
				}

				if (fmt_out)
				{
					*fmt_out = itr.second;
				}

				return true;
			}
		}
		return false;
	}

	uint32_t sdp_media::get_default_ssrc()const
	{
		auto itr=ssrcs_.begin();
		if (itr == ssrcs_.end())
		{
			return 0;
		}
		return itr->ssrc;
	}

	bool sdp_media::has_ssrc(uint32_t ssrc)const
	{
		for (auto itr = ssrcs_.begin(); itr != ssrcs_.end(); itr++)
		{
			if (itr->ssrc == ssrc)
			{
				return true;
			}
		}
		return false;
	}
}

