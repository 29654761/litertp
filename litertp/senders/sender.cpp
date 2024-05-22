/**
 * @file sender.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sender.h"

#include "../util/time.h"
#include <sys2/util.h>
#include <string.h>

namespace litertp
{
	sender::sender(uint32_t ssrc, media_type_t mt,const sdp_format& fmt)
	{
		ssrc_ = ssrc;
		format_ = fmt;
		media_type_ = mt;
		seq_ = sys::util::random_number<uint16_t>(0, 0xFF);

		memset(&stats_, 0, sizeof(stats_));
		stats_.ssrc = ssrc_;
	}

	sender::~sender()
	{
	}

	bool sender::send_packet(packet_ptr pkt)
	{
		send_rtp_packet_event_.invoke(pkt);
		
		stats_.packets_sent_period++;
		stats_.packets_sent++;
		stats_.bytes_sent_period += pkt->payload_size();
		stats_.bytes_sent += pkt->payload_size();

		seq_ = pkt->handle_->header->seq+1;

		double now = time_util::cur_time();
		timestamp_now_= ms_to_ts(now * 1000);

		set_history(pkt);
		
		return true;
	}

	uint16_t sender::last_rtp_seq()
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		return this->seq_;
	}

	uint32_t sender::last_rtp_timestamp()
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		return this->timestamp_;
	}

	double sender::last_timestamp()
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		return timestamp_now_;
	}


	void sender::set_history(packet_ptr packet)
	{
		std::unique_lock<std::shared_mutex>lk(history_packets_mutex_);
		int idx = packet->handle_->header->seq % PACKET_BUFFER_SIZE;
		history_packets_[idx] = packet;
	}

	packet_ptr sender::get_history(uint16_t seq)
	{
		std::shared_lock<std::shared_mutex> lk(history_packets_mutex_);
		int idx = seq % PACKET_BUFFER_SIZE;

		return history_packets_[idx];
	}

	void sender::update_remote_report(const rtcp_report& report)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);
		
		stats_.lost = report.lost;
		stats_.lost_period = report.fraction;
		stats_.jitter = report.jitter;
	}

	void sender::prepare_sr(rtcp_sr& sr)
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		memset(&sr, 0, sizeof(sr));
		rtcp_sr_init(&sr);

		sr.ssrc = ssrc_;
		sr.rtp_ts = now_timestamp();

		double now = time_util::cur_time();
		ntp_tv ntp = ntp_from_unix(now);
		sr.ntp_frac = ntp.frac;
		sr.ntp_sec = ntp.sec;
		
		sr.byte_count = stats_.bytes_sent_period;
		sr.pkt_count = stats_.packets_sent_period;
		stats_.bytes_sent_period = 0;
		stats_.packets_sent_period = 0;
		stats_.lsr_unix = now;
	}



	void sender::get_stats(rtp_sender_stats_t& stats)
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);

		stats_.last_seq = seq_;
		stats_.last_timestamp = timestamp_;
		stats_.pli = pli_count_;
		stats_.fir = fir_count_;
		stats_.nack = nack_count_;
		stats = stats_;
	}

	void sender::increase_fir()
	{
		fir_count_++;
	}
	void sender::increase_pli()
	{
		pli_count_++;
	}
	void sender::increase_nack()
	{
		nack_count_++;
	}
	
	uint32_t sender::now_timestamp()
	{
		double now = time_util::cur_time();
		double now_ts = ms_to_ts(now * 1000);
		uint32_t ts = now_ts - timestamp_now_;

		return timestamp_ + ts;
	}
	

	double sender::ms_to_ts(double ms)
	{
		return ms * (format_.frequency_ / 1000);
	}

	double sender::ts_to_ms(double ts)
	{
		return ts / (format_.frequency_ / 1000);
	}
}
