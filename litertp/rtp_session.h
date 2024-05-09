/**
 * @file rtp_session.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once
#include "media_stream.h"
#include <sys2/signal.h>

namespace litertp
{
	class rtp_session
	{
	public:
		rtp_session();
		~rtp_session();

		void close();

		media_stream_ptr create_media_stream(media_type_t mt, uint32_t ssrc, const char* local_address, int local_rtp_port, int local_rtcp_port);
		media_stream_ptr create_media_stream_tcp_server_1(media_type_t mt, uint32_t ssrc,int local_port,char rtp_channel,char rtcp_channel);
		media_stream_ptr create_media_stream_tcp_server_2(media_type_t mt, uint32_t ssrc, int local_port);

		media_stream_ptr create_media_stream_tcp_client_1(media_type_t mt, uint32_t ssrc,const char* address, int port, char rtp_channel, char rtcp_channel);
		media_stream_ptr create_media_stream_tcp_client_2(media_type_t mt, uint32_t ssrc, const char* address, int port);

		bool connect(media_type_t mt);

		media_stream_ptr get_media_stream(media_type_t mt);
		std::vector<media_stream_ptr> get_media_streams();
		void remove_media_stream(media_type_t mt);
		void clear_media_streams();
		void clear_local_candidates();
		void clear_remote_candidates();

		std::string create_offer();
		std::string create_answer();

		bool set_remote_sdp(const char* sdp, sdp_type_t sdp_type);
		bool set_local_sdp(const char* sdp);

		void require_keyframe();

		bool send_audio_frame(const uint8_t* frame, uint32_t size, uint32_t duration);
		bool send_video_frame(const uint8_t* frame, uint32_t size, uint32_t duration);

	private:
		transport_ptr get_udp_transport(int port);
		transport_ptr get_tcp_server1_transport(int port, char rtp_channel, char rtcp_channel);
		transport_ptr get_tcp_server2_transport(int port);
		transport_ptr get_tcp_client1_transport(const char* address,int port, char rtp_channel, char rtcp_channel);
		transport_ptr get_tcp_client2_transport(const char* address, int port);

		void remote_transport(int port);
		void clear_transports();



		static void s_litertp_on_frame(void* ctx, uint32_t ssrc, uint16_t pt, int frequency, int channels, const av_frame_t* frame);
		static void s_litertp_on_keyframe_required(void* ctx, uint32_t ssrc, int mode);
		static void s_litertp_on_rtcp_bye(void* ctx, uint32_t* ssrcs, int ssrc_count, const char* message);
		static void s_litertp_on_rtcp_app(void* ctx, uint32_t ssrc, uint32_t name, const char* appdata, uint32_t data_size);
		static void s_litertp_on_tcp_disconnect(void* ctx, media_type_t mt);

		void run();

		bool local_group_bundle();
	public:
		sys::callback<litertp_on_frame> litertp_on_frame_;
		sys::callback<litertp_on_keyframe_required> litertp_on_keyframe_required_;
		sys::callback<litertp_on_rtcp_app> litertp_on_rtcp_app_;
		sys::callback<litertp_on_rtcp_bye> litertp_on_rtcp_bye_;
		sys::callback<litertp_on_tcp_disconnect> litertp_on_tcp_disconnect_;
	private:
		bool active_ = true;
		
		sys::signal signal_;
		std::thread timer_;

		std::string cname_;
		std::string ice_ufrag_;
		std::string ice_pwd_;
		
		std::shared_mutex streams_mutex_;
		std::map<media_type_t,media_stream_ptr> streams_;
		

		std::shared_mutex transports_mutex_;
		std::map<int, transport_ptr> transports_;


	};


}