/**
 * @file sender.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "../transport.h"
#include "../packet.h"
#include "../proto/rtcp_sr.h"
#include "../sdp/sdp_format.h"

#include<sys2/callback.hpp>
#include <array>
#include <mutex>



namespace litertp
{
	typedef void (*send_rtp_packet_event)(void* ctx,packet_ptr packet);

	class sender
	{
	public:
		sender(uint32_t ssrc,media_type_t mt,const sdp_format& fmt);
		virtual ~sender();

		virtual bool send_frame(const uint8_t* frame, uint32_t size, uint32_t duration) = 0;
		virtual bool send_packet(packet_ptr pkt);

		uint32_t ssrc()const { return ssrc_; }
		const sdp_format& format()const { return format_; }
		uint32_t now_timestamp();

		uint16_t last_rtp_seq();
		uint32_t last_rtp_timestamp();
		double last_timestamp();

		void set_history(packet_ptr packet);
		packet_ptr get_history(uint16_t seq);

		media_type_t media_type()const { return media_type_; }

		void update_remote_report(const rtcp_report& report);
		void prepare_sr(rtcp_sr& sr);
		void get_stats(rtp_sender_stats_t& stats);
		void increase_fir();
		void increase_pli();
		void increase_nack();
		uint32_t fir_count()const { return fir_count_; }
		uint32_t pli_count()const { return pli_count_; }
		uint32_t nack_count()const { return nack_count_; }
	public:

		sys::callback<send_rtp_packet_event> send_rtp_packet_event_;

	protected:
		double ms_to_ts(double ms);
		double ts_to_ms(double ts);

	protected:
		std::shared_mutex mutex_;

		uint32_t ssrc_ = 0;
		sdp_format format_;
		media_type_t media_type_ = media_type_audio;
		
		rtp_sender_stats_t stats_;
		std::atomic<uint32_t> pli_count_ = 0;
		std::atomic<uint32_t> fir_count_ = 0;
		std::atomic<uint32_t> nack_count_ = 0;

		uint32_t timestamp_ = 0;
		uint16_t seq_ = 0;
		double timestamp_now_ = 0;

		std::shared_mutex history_packets_mutex_;
		std::array<packet_ptr, PACKET_BUFFER_SIZE> history_packets_;
	};

	typedef std::shared_ptr<sender> sender_ptr;
}