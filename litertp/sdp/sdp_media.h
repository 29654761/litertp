/**
 * @file sdp_media.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "sdp_format.h"
#include "candidate.h"

namespace litertp {

	struct ssrc_t
	{
		uint32_t ssrc=0;
		std::string cname;
		std::string msid;
	};

	class sdp_media
	{
	public:
		sdp_media();
		~sdp_media();

	public:
		void to_string(std::stringstream& ss)const;
		bool parse_header(const std::string& m);
		bool parse_c(const std::string& c);
		bool parse_a(const std::string& a);

		bool negotiate(const sdp_format* fmt_in,sdp_format* fmt_out);

		uint32_t get_default_ssrc()const;

		bool has_ssrc(uint32_t ssrc)const;
	private:
		void to_protocols_string(std::stringstream& ss)const;
		
	public:
		media_type_t media_type_ = media_type_audio;  //audio|video|text|application|message

		std::set<std::string> protos_; //UDP/TLS/RTP/SAVPF;

		std::string rtp_network_type_ = "IN";
		std::string rtp_address_type_ = "IP4";
		std::string rtp_address_ = "0.0.0.0";
		int rtp_port_ = 0;

		bool rtcp_mux_ = false;
		bool rtcp_rsize_ = false;
		std::string rtcp_network_type_ = "IN";
		std::string rtcp_address_type_ = "IP4";
		std::string rtcp_address_ = "0.0.0.0";
		int rtcp_port_ = 0;



		std::string ice_ufrag_;
		std::string ice_pwd_;
		std::string ice_options_ = "trickle";

		std::string fingerprint_sign_ = "sha-256";
		std::string fingerprint_;

		std::string mid_;
		std::string msid_;
		std::map<int, std::string> extmap_;

		rtp_trans_mode_t trans_mode_ = rtp_trans_mode_sendrecv;
		sdp_setup_t setup_ = sdp_setup_actpass;

		std::map<int, sdp_format> rtpmap_;

		std::string ssrc_group_;// = "FID";
		std::vector<ssrc_t> ssrcs_;

		std::vector<sdp_pair> attrs_;
		std::vector<candidate> candidates_;
	};



}
