/**
 * @file receivver.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "receiver.h"
#include "../proto/ntp.h"
#include "../util/time.h"
#include "../util/sn.hpp"
#include "../util/nack_pid_bid.h"
#include "../log.h"
#include <shared_mutex>
#include <string.h>

namespace litertp
{
	receiver::receiver(int ssrc,media_type_t mt, const sdp_format& fmt)
		:nack_thread_(&receiver::run_nack,this)
	{
		ssrc_ = ssrc;
		media_type_ = mt;
		format_ = fmt;
		memset(&rtp_source_, 0, sizeof(rtp_source_));
		rtp_source_init(&rtp_source_, ssrc_, 0);

		memset(&stats_, 0, sizeof(stats_));
		stats_.ssrc = ssrc_;

		if (mt == media_type_video)
		{
			waiting_for_keyframe_ = true;
		}
	}

	receiver::~receiver()
	{
		active_ = false;
		signal_.notify();
		nack_thread_.join();
	}



	bool receiver::insert_packet(packet_ptr pkt)
	{
		if (reset_)
		{
			begin_seq_ = pkt->handle_->header->seq;
			end_seq_ = pkt->handle_->header->seq;
			frame_begin_ts_ = std::chrono::high_resolution_clock::now();

			for (int i = 0; i < PACKET_BUFFER_SIZE; i++)
			{
				recv_packs_[i].reset();
			}

			reset_ = false;
		}

		if (sn::ahead_of<uint16_t>(begin_seq_, pkt->handle_->header->seq))
		{
			return false;
		}



		if (first_sec_ == 0)
		{
			first_sec_ = litertp::time_util::cur_time();
			first_ts_ = pkt->handle_->header->ts;
		}

		stats_.packets_received++;
		stats_.packets_received_period++;
		stats_.bytes_received += pkt->payload_size();
		stats_.bytes_received_period += pkt->payload_size();

		timestamp_ = pkt->handle_->header->ts;

		rtp_source_update_seq(&rtp_source_, pkt->handle_->header->seq);
		rtp_source_update_jitter(&rtp_source_, pkt->handle_->header->ts, this->now_timestamp());


		int idx = pkt->handle_->header->seq % PACKET_BUFFER_SIZE;
		//LOGD("insert packet %d seq=%d\n",idx, pkt->handle_->header->seq);
		recv_packs_[idx] = pkt;


		if (sn::ahead_of<uint16_t>(end_seq_, pkt->handle_->header->seq))
		{
			//the lost seq is received.
			LOGD("take lost seq=%u", pkt->handle_->header->seq);
		}
		else
		{
			auto n = sn::forward_diff<uint16_t>(end_seq_, pkt->handle_->header->seq);
			if (n > 1)
			{
				// loss packet
				add_nack(end_seq_ + 1, pkt->handle_->header->seq);
			}
			end_seq_ = pkt->handle_->header->seq;
		}


		// packet buffer is empty or full
		if (sn::ahead_of<uint16_t, PACKET_BUFFER_SIZE>(begin_seq_, end_seq_))
		{
			begin_seq_ = end_seq_;
		}




		return true;
	}


	uint16_t receiver::last_rtp_seq()
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		return end_seq_;
	}

	uint32_t receiver::last_rtp_timestamp()
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		return timestamp_;
	}




	uint32_t receiver::now_timestamp()
	{
		double now = litertp::time_util::cur_time() * 1000;
		double ms = now - first_sec_ * 1000;
		double ts = ms_to_ts(ms);
		ntp_tv ntp = ntp_from_double(ts);
		return ntp_short(ntp);
	}

	bool receiver::is_timeout()
	{
		auto now = std::chrono::high_resolution_clock::now();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(now - frame_begin_ts_);
		if (msec.count() >= delay_) {
			return true;
		}
		return false;

		
	}

	void receiver::add_nack(uint16_t begin, uint16_t end)
	{
		std::unique_lock<std::shared_mutex> lk(nack_packs_mutex_);
		if (nack_max_count_ <= 0)
		{
			return;
		}

		uint16_t i = begin;
		while (!sn::ahead_of<uint16_t>(i,end))
		{
			nack_pkt_t nack;
			nack.seq = i;
			nack.count = nack_max_count_;
			nack.ssrc = ssrc();
			nack_packs_.insert(std::make_pair(nack.seq, nack));
			i++;
		}
		signal_.notify();
	}

	void receiver::remove_nack(uint16_t seq)
	{
		std::unique_lock<std::shared_mutex>lk(nack_packs_mutex_);
		nack_packs_.erase(seq);
	}

	void receiver::clear_nack()
	{
		std::unique_lock<std::shared_mutex>lk(nack_packs_mutex_);
		nack_packs_.clear();
	}

	std::vector<nack_pkt_t> receiver::get_nack_list()
	{
		std::vector<nack_pkt_t> vec;

		{
			std::unique_lock<std::shared_mutex>lk(nack_packs_mutex_);
			vec.reserve(nack_packs_.size());
			for (auto& itr : nack_packs_)
			{
				vec.push_back(itr.second);
			}
		}

		std::sort(vec.begin(), vec.end(),
			[](const nack_pkt_t& a, const nack_pkt_t& b) {
				return a.seq > b.seq;
			}
		);
		return vec;
	}

	void receiver::update_remote_sr(const rtcp_sr& sr)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);
		ntp_tv ntp;
		ntp.sec = sr.ntp_sec;
		ntp.frac = sr.ntp_frac;

		rtp_source_update_lsr(&rtp_source_, ntp);

		stats_.bytes_sent += sr.byte_count;
		stats_.bytes_sent_period = sr.byte_count;
		stats_.packets_sent += sr.pkt_count;
		stats_.packets_sent_period = sr.pkt_count;

		stats_.lsr_unix = ntp_to_unix(ntp);
	}


	void receiver::prepare_rr(rtcp_report& rr)
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		double now = time_util::cur_time();
		ntp_tv now_ntp = ntp_from_unix(now);
		rtp_source_update_lost(&rtp_source_);
		rtcp_report_init(&rr, &rtp_source_, now_ntp);

		stats_.packets_received_period = 0;
		stats_.bytes_received_period = 0;
	}

	void receiver::get_stats(rtp_receiver_stats_t& stats)
	{
		std::shared_lock<std::shared_mutex>lk(mutex_);
		stats_.last_seq = end_seq_;
		stats_.last_timestamp = timestamp_;
		stats_.lost = rtp_source_.lost;
		stats_.lost_period = rtp_source_.fraction;
		stats_.fir = fir_count_;
		stats_.pli = pli_count_;
		stats_.nack = nack_count_;
		stats = stats_;
	}

	void receiver::increase_fir()
	{
		fir_count_++;
		reset_ = true;
	}
	void receiver::increase_pli()
	{
		pli_count_++;
		reset_ = true;
	}

	void receiver::increase_nack()
	{
		nack_count_++;
	}


	double receiver::ms_to_ts(double ms)
	{
		return ms * (format_.frequency_ / 1000);
	}

	double receiver::ts_to_ms(double ts)
	{
		return ts / (format_.frequency_ / 1000);
	}


	void receiver::run_nack()
	{
		double ts = time_util::cur_time();
		while (active_)
		{
			auto nack_pkts = get_nack_list();

			if ((nack_pkts.size() >= 1000|| waiting_for_keyframe_))
			{
				clear_nack();
				reset_ = true;
				if (media_type_ == media_type_video)
				{
					double now = time_util::cur_time();
					if (now - ts >= 3) 
					{
						ts = now;
						rtp_keyframe_event_.invoke(ssrc_, format_);
					}
				}
				signal_.wait(1000);
				continue;
			}

			nack_pid_bid pb;

			for (auto itr : nack_pkts)
			{
				if (!pb.add(itr.seq))
				{
					rtp_nack_event_.invoke(ssrc(), format(), pb.pid_, pb.bid_);
					pb.reset();
					pb.add(itr.seq);
				}


				itr.count++;

				if (itr.count >= nack_max_count_)
				{
					remove_nack(itr.seq);
				}
				else
				{
					std::shared_lock<std::shared_mutex>lk(nack_packs_mutex_);
					auto itr2=nack_packs_.find(itr.seq);
					if (itr2 != nack_packs_.end())
					{
						itr2->second.count = itr.count;
					}
				}
			}

			if (pb.has_pid_)
			{
				rtp_nack_event_.invoke(ssrc(), format(), pb.pid_, pb.bid_);
			}


			size_t count = 0;
			{
				std::shared_lock<std::shared_mutex>lk(nack_packs_mutex_);
				count = nack_packs_.size();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (count > 0) {
				signal_.wait(10);
			}
			else {
				signal_.wait(1000);
			}
		}
	}
}