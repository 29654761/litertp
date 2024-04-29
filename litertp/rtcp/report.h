/**
 * @file report.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <string>
#include <vector>

#include "../proto/rtcp_header.h"
#include "../proto/rtcp_report.h"

namespace litertp {
namespace rtcp {


	class report
	{
	public:
		report();
		~report();

		bool serialize(std::string& buffer);
		bool serialize_sr(std::string& buffer);
		bool serialize_rr(std::string& buffer);

		bool parse(const uint8_t* buffer, size_t size);
		bool parse_sr(const uint8_t* buffer, size_t size);
		bool parse_rr(const uint8_t* buffer, size_t size);

	public:
		rtcp_header header_;
		uint32_t ssrc_;
		uint32_t ntp_sec_;
		uint32_t ntp_frac_;
		uint32_t rtp_ts_;
		uint32_t pkt_count_;
		uint32_t byte_count_;
		std::vector<rtcp_report> reports_;
		std::string ext_data_;
	};
}
}



