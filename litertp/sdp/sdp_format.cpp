/**
 * @file sdp_format.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#include "sdp_format.h"
#include <sys2/string_util.h>

namespace litertp
{
	sdp_format::sdp_format()
	{
	}

	sdp_format::sdp_format(int payload_type, const codec_type_t& codec, int frequency, int channels)
	{
		payload_type_ = payload_type;
		codec_ = codec;
		frequency_ = frequency;
		channels_ = channels;
	}

	sdp_format::~sdp_format()
	{
	}

	void sdp_format::set_codec(const std::string& codec)
	{
		this->codec_text_ = codec;
		this->codec_ = convert_codec(codec);
	}

	const std::string sdp_format::get_codec()const
	{
		std::string ret = this->codec_text_;
		if (this->codec_ != codec_type_unknown)
		{
			ret = convert_codec_text(this->codec_);
		}
		return ret;
	}

	void sdp_format::to_string(std::stringstream& ss)const
	{
		std::string codec_str = get_codec();
		
		ss << "a=rtpmap:" << payload_type_ << " " << codec_str << "/" << frequency_;
		if (channels_ > 1)
		{
			ss << "/" << channels_;
		}
		ss << std::endl;

		for (auto rtcp_fb : rtcp_fb_)
		{
			ss << "a=rtcp-fb:" << payload_type_ << " " << rtcp_fb << std::endl;
		}
		for (auto fmtp : fmtp_)
		{
			ss << "a=fmtp:" << payload_type_ << " " << fmtp << std::endl;
		}
		for (auto attr : attrs_)
		{
			ss << "a=" << attr.to_string() << std::endl;
		}
	}

	codec_type_t sdp_format::convert_codec(const std::string& codec)
	{
		if (sys::string_util::icasecompare(codec, "telephone-event"))
		{
			return codec_type_telephone_event;
		}
		else if (sys::string_util::icasecompare(codec, "mpeg4-generic"))
		{
			return codec_type_mpeg4_generic;
		}
		else if (sys::string_util::icasecompare(codec, "MP4A-LATM"))
		{
			return codec_type_mp4a_latm;
		}
		else if (sys::string_util::icasecompare(codec, "OPUS"))
		{
			return codec_type_opus;
		}
		else if (sys::string_util::icasecompare(codec, "PCMA"))
		{
			return codec_type_pcma;
		}
		else if (sys::string_util::icasecompare(codec, "PCMU"))
		{
			return codec_type_pcmu;
		}
		else if (sys::string_util::icasecompare(codec, "G722"))
		{
			return codec_type_g722;
		}
		else if (sys::string_util::icasecompare(codec, "CN"))
		{
			return codec_type_cn;
		}
		else if (sys::string_util::icasecompare(codec, "H264"))
		{
			return codec_type_h264;
		}
		else if (sys::string_util::icasecompare(codec, "VP8"))
		{
			return codec_type_vp8;
		}
		else if (sys::string_util::icasecompare(codec, "VP9"))
		{
			return codec_type_vp9;
		}
		else if (sys::string_util::icasecompare(codec, "AV1"))
		{
			return codec_type_av1;
		}
		else if (sys::string_util::icasecompare(codec, "rtx"))
		{
			return codec_type_rtx;
		}
		else if (sys::string_util::icasecompare(codec, "red"))
		{
			return codec_type_red;
		}
		else if (sys::string_util::icasecompare(codec, "ulpfec"))
		{
			return codec_type_ulpfec;
		}
		else
		{
			return codec_type_unknown;
		}
	}

	std::string sdp_format::convert_codec_text(codec_type_t codec)
	{
		if (codec== codec_type_telephone_event)
		{
			return "telephone-event";
		}
		else if (codec== codec_type_mpeg4_generic)
		{
			return "mpeg4-generic";
		}
		else if (codec == codec_type_mp4a_latm)
		{
			return "MP4A-LATM";
		}
		else if (codec== codec_type_opus)
		{
			return "OPUS";
		}
		else if (codec== codec_type_pcma)
		{
			return "PCMA";
		}
		else if (codec==codec_type_pcmu)
		{
			return "PCMU";
		}
		else if (codec == codec_type_g722)
		{
			return "G722";
		}
		else if (codec == codec_type_cn)
		{
			return "CN";
		}
		else if (codec== codec_type_h264)
		{
			return "H264";
		}
		else if (codec== codec_type_vp8)
		{
			return "VP8";
		}
		else if (codec== codec_type_vp9)
		{
			return "VP9";
		}
		else if (codec == codec_type_av1)
		{
			return "AV1";
		}
		else if (codec == codec_type_rtx)
		{
			return "rtx";
		}
		else if (codec == codec_type_red)
		{
			return "red";
		}
		else if (codec == codec_type_ulpfec)
		{
			return "ulpfec";
		}
		else
		{
			return "";
		}
	}


	bool sdp_format::extract_h264_fmtp(int* level_asymmetry_allowed, int* packetization_mode, int64_t* profile_level_id)const
	{
		bool detected = false;

		*level_asymmetry_allowed = -1;
		*packetization_mode = -1;
		*profile_level_id = -1;

		for (auto fmtp : fmtp_)
		{
			auto vec=sys::string_util::split(fmtp, ";");
			for (auto kv : vec)
			{
				auto vec2=sys::string_util::split(kv, "=");
				if (vec2.size() == 2)
				{
					std::string key = vec2[0];
					std::string val = vec2[1];

					if (key == "level-asymmetry-allowed")
					{
						char* endptr = nullptr;
						*level_asymmetry_allowed = strtol(val.c_str(),&endptr,0);
						detected = true;
					}
					else if (key == "packetization-mode")
					{
						char* endptr = nullptr;
						*packetization_mode = strtol(val.c_str(), &endptr, 0);
						detected = true;
					}
					else if (key == "profile-level-id")
					{
						char* endptr = nullptr;
						*profile_level_id = strtoll(val.c_str(), &endptr, 16);
						detected = true;
					}
				}
			}

			if (detected)
			{
				break;
			}
		}

		return detected;
	}
}