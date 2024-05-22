/**
 * @file sdp.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "sdp_media.h"

namespace litertp {

	class sdp
	{
	public:
		std::string to_string()const;
		bool parse(const std::string& s);

	public:
		std::string v_ = "0";
		std::string o_ = "- 0 0 IN IP4 127.0.0.1";
		std::string s_ = "-";
		std::string i_;
		std::string u_;
		std::string e_;
		std::string p_;
		std::string c_ = "IN IP4 0.0.0.0";
		std::string b_;
		std::string z_;
		std::string k_;

		std::string t_= "0 0";
		std::string r_;

		
		bool bundle_ = false;

		std::vector<sdp_media> medias_;
		std::vector<sdp_pair> attrs_;

	};


}

