/**
 * @file litertp.cpp
 * @brief Struct defined.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	//#ifdef MSVC

#ifdef _WIN32
	#ifdef LITERTP_DLL
		#define LITERTP_API __declspec(dllexport)
	#else
		#define LITERTP_API __declspec(dllimport)
	#endif
	#define LITERTP_CALL __cdecl
#else
	#define LITERTP_API
	#define LITERTP_CALL
#endif



//#else
//#define LITERTP_API
//#define LITERTP_CALL
//#endif //MSVC

#include "avtypes.h"

#define MAX_RTP_PAYLOAD_SIZE 1200
#define PACKET_BUFFER_SIZE 512

	typedef enum sdp_type_t
	{
		sdp_type_offer,
		sdp_type_answer,
	}sdp_type_t;

	typedef enum sdp_setup_t
	{
		sdp_setup_actpass,
		sdp_setup_active,
		sdp_setup_passive,
	}sdp_setup_t;

	typedef enum srtp_role_t
	{
		srtp_role_server,
		srtp_role_client,
	}srtp_role_t;

	typedef enum rtp_trans_mode_t
	{
		rtp_trans_mode_inactive = 0,
		rtp_trans_mode_recvonly = 0x01,
		rtp_trans_mode_sendonly = 0x02,
		rtp_trans_mode_sendrecv = 0x03,
	}rtp_trans_mode_t;



	typedef struct _rtp_sender_stats_t
	{
		uint32_t ssrc;
		uint32_t last_timestamp;
		uint16_t last_seq;

		double jitter;

		uint64_t packets_sent;
		uint64_t bytes_sent;
		uint32_t packets_sent_period;
		uint32_t bytes_sent_period;

		uint32_t lost;
		uint16_t lost_period;

		double lsr_unix;
		uint32_t nack;
		uint32_t pli;
		uint32_t fir;

	}rtp_sender_stats_t;

	typedef struct _rtp_receiver_stats_t
	{
		uint32_t ssrc;
		uint32_t last_timestamp;
		uint16_t last_seq;

		double jitter;

		uint64_t packets_received;
		uint64_t bytes_received;
		uint64_t frames_received;
		uint64_t frames_droped;
		uint32_t packets_received_period;
		uint32_t bytes_received_period;

		uint64_t packets_sent;
		uint64_t bytes_sent;
		uint32_t packets_sent_period;
		uint32_t bytes_sent_period;

		uint32_t lost;
		uint16_t lost_period;

		double lsr_unix;

		uint32_t nack;
		uint32_t pli;
		uint32_t fir;
	}rtp_receiver_stats_t;


	typedef struct _rtp_stats_t
	{
		uint16_t pt;
		codec_type_t ct;
		media_type_t mt;
		rtp_sender_stats_t sender_stats;
		rtp_receiver_stats_t receiver_stats;
	}rtp_stats_t;

	typedef void (*litertp_on_frame)(void* ctx, uint32_t ssrc, uint16_t pt, int frequency, int channels, const av_frame_t* frame);
	typedef void (*litertp_on_keyframe_required)(void* ctx, uint32_t ssrc, int mode);
	typedef void (*litertp_on_rtcp_bye)(void* ctx, uint32_t* ssrcs,int ssrc_count,const char* message);
	typedef void (*litertp_on_rtcp_app)(void* ctx, uint32_t ssrc, uint32_t name, const char* appdata,uint32_t data_size);

#ifdef __cplusplus
}
#endif //__cplusplus