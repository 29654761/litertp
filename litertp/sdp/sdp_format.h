/**
 * @file sdp_format.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once

#include "sdp_pair.h"
#include "../litertp_def.h"

#include <string>
#include <set>
#include <map>
#include <vector>
#include <sstream>

namespace litertp 
{
	class sdp_format
	{
	public:
		sdp_format();
		sdp_format(int payload_type, const codec_type_t& codec, int frequency, int channels = 1);
		~sdp_format();

		void to_string(std::stringstream& ss)const;

		static codec_type_t convert_codec(const std::string& codec);
		static std::string convert_codec_text(codec_type_t codec);

		void set_codec(const std::string& codec);
		const std::string get_codec()const;

		bool extract_h264_fmtp(int* level_asymmetry_allowed, int* packetization_mode, int64_t* profile_level_id)const;
	public:
		uint16_t payload_type_ = 128;
		codec_type_t codec_ = codec_type_unknown;
		std::string codec_text_;
		int frequency_ = 0;
		int channels_ = 1;



		std::set<std::string> fmtp_;
		std::set<std::string> rtcp_fb_;

		std::vector<sdp_pair> attrs_;
	};


}


