/**
 * @file transport.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "transport.h"
#include "../proto/rtcp_header.h"
#include "../log.h"

#include <sys2/util.h>

namespace litertp {

	transport::transport()
	{
	}

	transport::~transport()
	{
		stop();
	}


	bool transport::start()
	{
		if (active_) {
			return true;
		}
		active_ = true;

		return true;
	}

	void transport::stop()
	{
		active_ = false;
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

	bool transport::receive_rtp_packet(const uint8_t* rtp_packet, int size)
	{
		auto pkt = std::make_shared<packet>();
		if (!pkt->parse(rtp_packet, size)) 
		{
			return false;
		}

		rtp_packet_event_.invoke(socket_, pkt, nullptr, 0);
		return true;
	}

	bool transport::receive_rtcp_packet(const uint8_t* rtcp_packet, int size)
	{
		rtcp_header hdr = { 0 };
		int pt = rtcp_header_parse(&hdr, rtcp_packet, size);
		if (pt >= 0)
		{
			return false;
		}
		rtcp_packet_event_.invoke(socket_, (uint16_t)pt, rtcp_packet, size, nullptr, 0);
		return true;
	}
}


