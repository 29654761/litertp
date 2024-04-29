/**
 * @file litertp.cpp
 * @brief C-API Interfaces implement.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "litertp.h"
#include "rtp_session.h"
#include "global.h"
#include <sys2/string_util.h>
#include "proto/rtcp_util.h"




LITERTP_API int LITERTP_CALL litertp_global_init()
{
	if (!litertp::g_instance.init())
	{
		return -1;
	}

	return 0;
}

LITERTP_API void LITERTP_CALL litertp_global_cleanup()
{
	litertp::g_instance.cleanup();
}

LITERTP_API litertp_session_t* LITERTP_CALL litertp_create_session()
{
	litertp::rtp_session* sess = new litertp::rtp_session();

	return (litertp_session_t*)sess;
}

LITERTP_API void LITERTP_CALL litertp_destory_session(litertp_session_t** session)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)(*session);
	if (sess)
	{
		delete sess;
	}
	*session = nullptr;
}

LITERTP_API void LITERTP_CALL litertp_free(void** mem)
{
	sys::string_util::free((char**)mem);
}




LITERTP_API int LITERTP_CALL litertp_set_on_frame_eventhandler(litertp_session_t* session, litertp_on_frame on_frame, void* ctx)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
		return -1;

	sess->litertp_on_frame_.clear();
	sess->litertp_on_frame_.add(on_frame, ctx);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_on_keyframe_required_eventhandler(litertp_session_t* session, litertp_on_keyframe_required on_keyframe_required, void* ctx)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
		return -1;

	sess->litertp_on_keyframe_required_.clear();
	sess->litertp_on_keyframe_required_.add(on_keyframe_required, ctx);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_on_rtcp_bye(litertp_session_t* session, litertp_on_rtcp_bye on_bye, void* ctx)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
		return -1;

	sess->litertp_on_rtcp_bye_.clear();
	sess->litertp_on_rtcp_bye_.add(on_bye, ctx);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_on_rtcp_app(litertp_session_t* session, litertp_on_rtcp_app on_app, void* ctx)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
		return -1;

	sess->litertp_on_rtcp_app_.clear();
	sess->litertp_on_rtcp_app_.add(on_app, ctx);
	return 0;
}



LITERTP_API int LITERTP_CALL litertp_create_media_stream(litertp_session_t* session, media_type_t mt, uint32_t ssrc, rtp_trans_mode_t trans_mode, bool security,
	const char* local_address, int local_rtp_port, int local_rtcp_port)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->create_media_stream(mt, ssrc, local_address, local_rtp_port, local_rtcp_port);
	if (!m)
	{
		return false;
	}

	m->set_local_trans_mode(trans_mode);
	if (security)
	{
		m->enable_dtls();
	}

	return 0;
}


LITERTP_API int LITERTP_CALL litertp_remove_media_stream(litertp_session_t* session, media_type_t mt)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	sess->remove_media_stream(mt);

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_clear_media_streams(litertp_session_t* session)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	sess->clear_media_streams();
	return 0;
}


LITERTP_API int LITERTP_CALL litertp_add_local_video_track(litertp_session_t* session, codec_type_t codec, uint16_t pt, int frequency)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(media_type_video);
	if (!m)
	{
		return -1;
	}

	if (!m->add_local_video_track(codec, pt, frequency))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_local_audio_track(litertp_session_t* session, codec_type_t codec, uint16_t pt, int frequency, int channels)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}


	auto m = sess->get_media_stream(media_type_audio);
	if (!m)
	{
		return -1;
	}

	if (!m->add_local_audio_track(codec, pt, frequency, channels))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_remote_video_track(litertp_session_t* session,codec_type_t codec, uint16_t pt, int frequency)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(media_type_video);
	if (!m)
	{
		return -1;
	}

	if (!m->add_remote_video_track(codec, pt, frequency))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_remote_audio_track(litertp_session_t* session, codec_type_t codec, uint16_t pt, int frequency, int channels)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(media_type_audio);
	if (!m)
	{
		return -1;
	}

	if (!m->add_remote_audio_track(codec, pt, frequency, channels))
	{
		return -1;
	}

	return 0;
}



LITERTP_API int LITERTP_CALL litertp_set_remote_trans_mode(litertp_session_t* session, media_type_t mt, rtp_trans_mode_t trans_mode)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	m->set_remote_trans_mode(trans_mode);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_remote_ssrc(litertp_session_t* session, media_type_t mt, uint32_t ssrc)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	m->set_remote_ssrc(ssrc);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_remote_mid(litertp_session_t* session, media_type_t mt, const char* mid)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	m->set_remote_mid(mid);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_remote_setup(litertp_session_t* session, media_type_t mt, sdp_setup_t setup)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	m->set_remote_setup(setup);
	return 0;
}



LITERTP_API int LITERTP_CALL litertp_set_remote_rtp_endpoint(litertp_session_t* session, media_type_t mt, int family, const char* address, int port)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}

	sockaddr_storage addr;
	sys::socket::ep2addr(family, address, port, (sockaddr*)&addr);
	m->set_remote_rtp_endpoint((const sockaddr*)&addr, sizeof(addr), 0xFFFFFFFF);

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_remote_rtcp_endpoint(litertp_session_t* session, media_type_t mt, int family, const char* address, int port)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}

	sockaddr_storage addr{ 0 };
	sys::socket::ep2addr(family, address, port, (sockaddr*)&addr);
	m->set_remote_rtcp_endpoint((const sockaddr*)&addr, sizeof(addr), 0xFFFFFFFF);
	return 0;
}


LITERTP_API int LITERTP_CALL litertp_add_local_candidate(litertp_session_t* session, media_type_t mt, uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	m->add_local_candidate(foundation, component, address, port, priority);
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_remote_candidate(litertp_session_t* session, media_type_t mt, uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return false;
	}
	m->add_remote_candidate(foundation, component, address, port, priority);

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_clear_local_candidates(litertp_session_t* session)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	sess->clear_local_candidates();
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_clear_remote_candidates(litertp_session_t* session)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	sess->clear_remote_candidates();
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_remote_attribute(litertp_session_t* session, media_type_t mt, uint16_t pt, const char* key, const char* val)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	if (!m->add_remote_attribute(pt, key, val))
	{
		return -1;
	}
	return 0;
}
LITERTP_API int LITERTP_CALL litertp_clear_remote_attributes(litertp_session_t* session, media_type_t mt, uint16_t pt)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	if (!m->clear_remote_attributes(pt))
	{
		return -1;
	}
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_add_local_attribute(litertp_session_t* session, media_type_t mt, uint16_t pt, const char* key, const char* val)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	if (!m->add_local_attribute(pt, key, val))
	{
		return -1;
	}
	return 0;
}
LITERTP_API int LITERTP_CALL litertp_clear_local_attributes(litertp_session_t* session, media_type_t mt, uint16_t pt)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	auto m = sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}
	if (!m->clear_local_attributes(pt))
	{
		return -1;
	}
	return 0;
}




LITERTP_API int LITERTP_CALL litertp_create_offer(litertp_session_t* session, char** local_sdp, int* sdp_size)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	std::string sdp = sess->create_offer();
	if (sdp.size() == 0)
	{
		return -1;
	}

	*local_sdp = sys::string_util::alloc(sdp);
	*sdp_size = (int)sdp.size();

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_create_answer(litertp_session_t* session, char** local_sdp, int* sdp_size)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	std::string sdp = sess->create_answer();

	if (sdp.size() == 0)
	{
		return -1;
	}

	*local_sdp = sys::string_util::alloc(sdp);
	*sdp_size = (int)sdp.size();

	return 0;
}


LITERTP_API int LITERTP_CALL litertp_set_remote_sdp(litertp_session_t* session, const char* sdp, sdp_type_t sdp_type)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	if (!sess->set_remote_sdp(sdp,sdp_type))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_set_local_sdp(litertp_session_t* session, const char* sdp)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	if (!sess->set_local_sdp(sdp))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_require_keyframe(litertp_session_t* session)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	sess->require_keyframe();
	return 0;
}

LITERTP_API int LITERTP_CALL litertp_send_audio_frame(litertp_session_t* session,const uint8_t* frame, uint32_t size, uint32_t duration)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	if (!sess->send_audio_frame(frame, size, duration))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_send_video_frame(litertp_session_t* session, const uint8_t* frame, uint32_t size, uint32_t duration)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	if (!sess->send_video_frame(frame, size, duration))
	{
		return -1;
	}

	return 0;
}

LITERTP_API int LITERTP_CALL litertp_get_stats(litertp_session_t* session, media_type_t mt, rtp_stats_t* stats)
{
	litertp::rtp_session* sess = (litertp::rtp_session*)session;
	if (!sess)
	{
		return -1;
	}

	litertp::media_stream_ptr m =sess->get_media_stream(mt);
	if (!m)
	{
		return -1;
	}

	m->get_stats(*stats);
	return 0;
}

