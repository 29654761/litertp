/**
 * @file report.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "report.h"

#include "litertp/proto/rtcp_util.h"
#include "litertp/proto/rtcp_rr.h"
#include "litertp/proto/rtcp_sr.h"


namespace litertp {
namespace rtcp {

report::report()
{
}

report::~report()
{
}

bool report::serialize(std::string& buffer)
{
	if (header_.common.pt == rtcp_packet_type::RTCP_SR)
	{
		return serialize_sr(buffer);
	}
	else if (header_.common.pt == rtcp_packet_type::RTCP_RR)
	{
		return serialize_rr(buffer);
	}
	else {
		return false;
	}
}



bool report::serialize_sr(std::string& buffer)
{
	rtcp_sr* sr = rtcp_sr_create();
	rtcp_sr_init(sr);
	sr->header = header_;
	sr->ssrc = ssrc_;
	sr->ntp_sec = ntp_sec_;
	sr->ntp_frac = ntp_frac_;
	sr->rtp_ts = rtp_ts_;
	sr->pkt_count = pkt_count_;
	for (auto report : reports_) {
		rtcp_sr_add_report(sr, &report);
	}
	if (ext_data_.size() > 0) {
		rtcp_sr_set_ext(sr, ext_data_.data(), ext_data_.size());
	}

	size_t size = rtcp_sr_size(sr);
	buffer.reserve(size);
	uint8_t* buf = (uint8_t*)malloc(size);
	if (buf == nullptr) {
		rtcp_sr_free(sr);
		return false;
	}
	if (rtcp_sr_serialize(sr, buf, size) < 0) {
		rtcp_sr_free(sr);
		free(buf);
		return false;
	}
	buffer.assign((const char*)buf, size);
	rtcp_sr_free(sr);
	free(buf);
	return true;
}

bool report::serialize_rr(std::string& buffer)
{
	rtcp_rr* rr = rtcp_rr_create();
	rtcp_rr_init(rr);
	rr->header = header_;
	rr->ssrc = ssrc_;
	
	for (auto report : reports_) {
		rtcp_rr_add_report(rr, &report);
	}
	if (ext_data_.size() > 0) {
		rtcp_rr_set_ext(rr, ext_data_.data(), ext_data_.size());
	}

	size_t size = rtcp_rr_size(rr);
	buffer.reserve(size);
	uint8_t* buf = (uint8_t*)malloc(size);
	if (buf == nullptr) {
		rtcp_rr_free(rr);
		return false;
	}
	if (rtcp_rr_serialize(rr, buf, size) < 0) {
		rtcp_rr_free(rr);
		free(buf);
		return false;
	}
	buffer.assign((const char*)buf, size);
	rtcp_rr_free(rr);
	free(buf);
	return true;
}



bool report::parse(const uint8_t* buffer, size_t size)
{
	auto pt = rtcp_type(buffer, size);
	if (pt == rtcp_packet_type::RTCP_RR) {
		return parse_rr(buffer, size);
	}
	else if (pt == rtcp_packet_type::RTCP_SR) {
		return parse_sr(buffer, size);
	}
	else {
		return false;
	}
}

bool report::parse_sr(const uint8_t* buffer, size_t size)
{
	rtcp_sr* sr = rtcp_sr_create();
	rtcp_sr_init(sr);

	if (rtcp_sr_parse(sr, buffer, size) < 0) {
		rtcp_sr_free(sr);
		return false;
	}

	header_ = sr->header;
	ssrc_ = sr->ssrc;
	ntp_sec_ = sr->ntp_sec;
	ntp_frac_ = sr->ntp_frac;
	rtp_ts_ = sr->rtp_ts;
	pkt_count_ = sr->pkt_count;
	byte_count_ = sr->byte_count;

	reports_.clear();
	for (uint8_t i = 0; i < sr->header.common.count; ++i)
	{
		reports_.push_back(sr->reports[i]);
	}
	if (sr->ext_size > 0)
	{
		ext_data_.assign((const char*)sr->ext_data, sr->ext_size);
	}

	rtcp_sr_free(sr);
	return true;
}

bool report::parse_rr(const uint8_t* buffer, size_t size)
{
	rtcp_rr* rr = rtcp_rr_create();
	rtcp_rr_init(rr);

	if (rtcp_rr_parse(rr, buffer, size) < 0) {
		rtcp_rr_free(rr);
		return false;
	}

	header_ = rr->header;
	ssrc_ = rr->ssrc;
	reports_.clear();
	for (uint8_t i = 0; i < rr->header.common.count; ++i)
	{
		reports_.push_back(rr->reports[i]);
	}
	if (rr->ext_size > 0) 
	{
		ext_data_.assign((const char*)rr->ext_data, rr->ext_size);
	}

	rtcp_rr_free(rr);
	return true;
}

}
}