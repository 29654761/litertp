/**
 * @file receiver.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include "../avtypes.h"
#include "../packet.h"
#include "../proto/rtp_source.h"
#include "../proto/rtcp_sr.h"
#include "../proto/rtcp_rr.h"
#include "../sdp/sdp_format.h"

#include <sys2/callback.hpp>
#include <sys2/signal.h>
#include <shared_mutex>
#include <atomic>
#include <array>
#include <thread>


namespace litertp
{

	typedef struct _nack_pkt_t
	{
		int ssrc = 0;
		int seq = 0;
		int count = 1;
	}nack_pkt_t;


	typedef std::chrono::high_resolution_clock clock;

	typedef void(*rtp_frame_event)(void* ctx, uint32_t ssrc,const sdp_format& fmt, const av_frame_t& frame);
	typedef void(*rtp_nack_event)(void* ctx, uint32_t ssrc, const sdp_format& fmt,uint16_t pid,uint16_t bld);
	typedef void(*rtp_keyframe_event)(void* ctx, uint32_t ssrc, const sdp_format& fmt);


	class receiver
	{
	public:
		receiver(int ssrc,media_type_t mt,const sdp_format& fmt);
		virtual ~receiver();

		virtual bool insert_packet(packet_ptr pkt) = 0;

		void update_remote_sr(const rtcp_sr& sr);
		void prepare_rr(rtcp_report& rr);
		void get_stats(rtp_receiver_stats_t& stats);
		void increase_fir();
		void increase_pli();
		void increase_nack();
		uint32_t fir_count()const { return fir_count_; }
		uint32_t pli_count()const { return pli_count_; }
		uint32_t nack_count()const { return nack_count_; }

		uint32_t ssrc()const {
			return ssrc_;
		}

		const sdp_format& format()const {
			return format_;
		}

		uint16_t last_rtp_seq();
		uint32_t last_rtp_timestamp();
	protected:
		//get the now ntp convert to rtp timestamp.
		uint32_t get_now_timestamp();
		double ms_to_ts(double ms);
		double ts_to_ms(double ts);

		//Test whether the broken frame can be removed.
		bool is_timeout();

		void add_nack(uint16_t begin, uint16_t end);
		void remove_nack(uint16_t seq);
		void clear_nack();
		std::vector<nack_pkt_t> get_nack_list();


	private:
		void run_nack();

	public:
		sys::callback<rtp_frame_event> rtp_frame_event_;
		sys::callback<rtp_nack_event> rtp_nack_event_;
		sys::callback<rtp_keyframe_event> rtp_keyframe_event_;
	protected:
		std::shared_mutex mutex_;
		bool active_ = true;
		std::array<packet_ptr, PACKET_BUFFER_SIZE> recv_packs_;

		sys::signal signal_;
		std::shared_mutex nack_packs_mutex_;
		std::map<uint16_t,nack_pkt_t> nack_packs_;
		int nack_max_count_ = 1;
		bool reset_ = true;
		bool waiting_for_keyframe_=false;
		std::thread nack_thread_;

		rtp_source rtp_source_;

		uint32_t first_ts_ = 0;
		double first_sec_ = 0.0;


		int begin_seq_ = -1; //next frame begin seq
		int end_seq_ = -1;

		uint32_t timestamp_ = 0;

		int ssrc_ = 0; //remote ssrc
		media_type_t media_type_ = media_type_t::media_type_audio;
		sdp_format format_;

		int delay_ = 1000;
		clock::time_point frame_begin_ts_ = clock::time_point::min();

		rtp_receiver_stats_t stats_;
		std::atomic<uint32_t> pli_count_ = 0;
		std::atomic<uint32_t> fir_count_ = 0;
		std::atomic<uint32_t> nack_count_ = 0;
	};

	typedef std::shared_ptr<receiver> receiver_ptr;
}