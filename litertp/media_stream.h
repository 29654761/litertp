/**
 * @file media_stream.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include "transports/transport_udp.h"
#include "transports/transport_tcp_2326.h"
#include "transports/transport_tcp_4571.h"

#include "senders/sender.h"
#include "receivers/receiver.h"

#include "proto/rtcp_app.h"
#include "proto/rtcp_bye.h"
#include "proto/rtcp_fb.h"
#include "proto/rtcp_report.h"
#include "proto/rtcp_rr.h"
#include "proto/rtcp_sr.h"
#include "proto/rtcp_sdes.h"

#include "sdp/sdp.h"

#include <sys2/signal.h>

namespace litertp
{
	class media_stream
	{
	public:
		media_stream(media_type_t media_type, uint32_t ssrc,const std::string& mid,const std::string& cname, const std::string& ice_ufrag, const std::string& ice_pwd,
			const std::string& local_address,transport_ptr transport_rtp, transport_ptr transport_rtcp);
		~media_stream();

		void close();


		bool add_local_video_track(codec_type_t codec, uint16_t pt, int frequency = 90000);
		bool add_local_audio_track(codec_type_t codec, uint16_t pt, int frequency, int channels = 1);

		bool add_remote_video_track(codec_type_t codec, uint16_t pt, int frequency = 90000);
		bool add_remote_audio_track(codec_type_t codec, uint16_t pt, int frequency, int channels = 1);

		
		void set_remote_trans_mode(rtp_trans_mode_t trans_mode);
		void set_local_trans_mode(rtp_trans_mode_t trans_mode);
		void set_remote_ssrc(uint32_t ssrc);
		void set_remote_setup(sdp_setup_t setup);
		void set_local_setup(sdp_setup_t setup);

		bool add_remote_attribute(uint16_t pt,const char* key, const char* val);
		bool clear_remote_attributes(uint16_t pt);

		bool add_local_attribute(uint16_t pt,const char* key, const char* val);
		bool clear_local_attributes(uint16_t pt);

		void add_local_candidate(uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority);
		void add_remote_candidate(uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority);
		void clear_local_candidates();
		void clear_remote_candidates();

		void enable_dtls();
		void disable_dtls();

		bool set_remote_sdp(const sdp_media& sdp, sdp_type_t sdp_type);
		void set_local_sdp(const sdp_media& sdp);

		sdp_type_t sdp_type() const { return sdp_type_; }
		void set_sdp_type(sdp_type_t sdp_type);

		void set_remote_mid(const char* mid);

		void set_remote_rtp_endpoint(const sockaddr* addr, int addr_size,uint32_t priority);
		void set_remote_rtcp_endpoint(const sockaddr* addr, int addr_size, uint32_t priority);
		bool get_remote_rtp_endpoint(sockaddr_storage* addr);
		bool get_remote_rtcp_endpoint(sockaddr_storage* addr);

		srtp_role_t srtp_role();
		bool check_setup();


		bool negotiate();

		void run_rtcp_stats();
		void run_stun_request();

		void send_rtcp_nack(uint32_t ssrc_sender, uint32_t ssrc_media, uint16_t pid, uint16_t blp);

		
		void send_rtcp_keyframe(uint32_t ssrc_media);
		void send_rtcp_keyframe(uint32_t ssrc_media,uint16_t pt);
		void send_rtcp_keyframe_pli(uint32_t ssrc_sender, uint32_t ssrc_media);
		void send_rtcp_keyframe_fir(uint32_t ssrc_sender, uint32_t ssrc_media);
		void send_rtcp_bye(const char* reason="closed");

		bool send_rtp_packet(packet_ptr packet);
		bool send_rtcp_packet(uint8_t* rtcp_data,int size);

		media_type_t media_type();

		bool send_frame(const uint8_t* frame, uint32_t size, uint32_t duration);

		sdp_media get_local_sdp();
		sdp_media get_remote_sdp();
		
		bool get_local_format(int pt, sdp_format& fmt);
		bool get_remote_format(int pt, sdp_format& fmt);

		uint32_t get_local_ssrc(int idx=0);
		uint32_t get_remote_ssrc(int idx=0);

		void get_stats(rtp_stats_t& stats);

	private:

		static void s_transport_rtp_packet(void* ctx, std::shared_ptr<sys::socket> skt, packet_ptr packet, const sockaddr* addr, int addr_size);
		static void s_transport_rtcp_packet(void* ctx, std::shared_ptr<sys::socket> skt, uint16_t pt, const uint8_t* buffer, size_t size, const sockaddr* addr, int addr_size);
		static void s_transport_stun_message(void* ctx, std::shared_ptr<sys::socket> skt, const stun_message& msg, const sockaddr* addr, int addr_size);
		static void s_transport_disconnect(void* ctx);

		static void s_rtp_frame_event(void* ctx, uint32_t ssrc, const sdp_format& fmt, const av_frame_t& frame);
		void on_rtp_frame_event(uint32_t ssrc, const sdp_format& fmt, const av_frame_t& frame);

		static void s_rtp_nack_event(void* ctx, uint32_t ssrc, const sdp_format& fmt, uint16_t pid, uint16_t bld);

		void on_rtp_nack_event(uint32_t ssrc, const sdp_format& fmt, uint16_t pid, uint16_t bld);

		static void s_rtp_keyframe_event(void* ctx, uint32_t ssrc, const sdp_format& fmt);
		void on_rtp_keyframe_event(uint32_t ssrc, const sdp_format& fmt);


		static void s_send_rtp_packet_event(void* ctx,packet_ptr packet);

		sender_ptr get_default_sender();
		sender_ptr get_sender(int pt);
		sender_ptr get_sender_by_ssrc(uint32_t ssrc);
		std::vector<sender_ptr> get_senders();
		sender_ptr create_sender(const sdp_format& fmt);

		receiver_ptr get_receiver(int pt);
		receiver_ptr get_receiver_by_ssrc(uint32_t ssrc);
		std::vector<receiver_ptr> get_receivers();

		bool has_remote_ssrc(uint32_t ssrc);
	private:

		void on_rtcp_app(const rtcp_app* app);
		void on_rtcp_bye(const rtcp_bye* bye);
		void on_rtcp_sr(const rtcp_sr* sr);
		void on_rtcp_rr(const rtcp_rr* rr);
		void on_rtcp_sdes(const rtcp_sdes* sdes);
		void on_rtcp_nack(uint32_t ssrc,uint16_t pid, uint16_t bld);
		void on_rtcp_pli(uint32_t ssrc);
		void on_rtcp_fir(uint32_t ssrc, uint8_t nr);

	public:

		sys::callback<litertp_on_frame> litertp_on_frame_;
		sys::callback<litertp_on_keyframe_required> litertp_on_keyframe_required_;
		sys::callback<litertp_on_rtcp_app> litertp_on_rtcp_app_;
		sys::callback<litertp_on_rtcp_bye> litertp_on_rtcp_bye_;
		sys::callback<litertp_on_tcp_disconnect> litertp_on_tcp_disconnect_;

		transport_ptr transport_rtp_;
		transport_ptr transport_rtcp_;
	private:
		std::string cname_;

		std::shared_mutex local_sdp_media_mutex_;
		sdp_media local_sdp_media_;

		std::shared_mutex remote_sdp_media_mutex_;
		sdp_media remote_sdp_media_;
		


		std::shared_mutex remote_rtp_endpoint_mutex_;
		sockaddr_storage remote_rtp_endpoint_ = { 0 };
		uint32_t remote_rtp_endpoint_priority = 0;

		std::shared_mutex remote_rtcp_endpoint_mutex_;
		sockaddr_storage remote_rtcp_endpoint_ = { 0 };
		uint32_t remote_rtcp_endpoint_priority = 0;
		

		std::shared_mutex senders_mutex_;
		std::map<int, sender_ptr> senders_;

		std::shared_mutex receivers_mutex_;
		std::map<int, receiver_ptr> receivers_;


		sdp_type_t sdp_type_= sdp_type_offer;

	};


	typedef std::shared_ptr<media_stream> media_stream_ptr;
}
