/**
 * @file litertp.h
 * @brief C-API Interfaces defailed.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



#include "litertp_def.h"


typedef void litertp_session_t;

/**
 * @brief Initialize library.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_global_init();

/**
 * @brief Cleanup library.
 */
LITERTP_API void LITERTP_CALL litertp_global_cleanup();

/**
 * @brief Create an rtp session.
 * @return - a point to rtp session handler,or return nullptr when failed.
 */
LITERTP_API litertp_session_t* LITERTP_CALL litertp_create_session();

/**
 * @brief Destory an rtp session.
 */
LITERTP_API void LITERTP_CALL litertp_destory_session(litertp_session_t** session);

/**
 * @brief Free any memory alloced by library.
 *
 * @param [in/out] mem - A point to memory, this will be set nullptr after calling.
 */
LITERTP_API void LITERTP_CALL litertp_free(void** mem);

/**
 * @brief Set callback function, raised when a video or audio frame is arrived.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] on_frame - A function point to handle.
 * @param [in] ctx - Context to on_frame.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_on_frame_eventhandler(litertp_session_t* session, litertp_on_frame on_frame,void* ctx);

/**
 * @brief Set callback function, raised when remote end required keyframe.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] on_keyframe_required - A function point to handle.
 * @param [in] ctx - Context to on_keyframe_required.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_on_keyframe_required_eventhandler(litertp_session_t* session, litertp_on_keyframe_required on_keyframe_required, void* ctx);

/**
 * @brief Set callback function, raised when rtcp bye received from remote end.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] on_bye - A function point to handle.
 * @param [in] ctx - Context to on_bye.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_on_rtcp_bye(litertp_session_t* session, litertp_on_rtcp_bye on_bye, void* ctx);

/**
 * @brief Set callback function, raised when rtcp app received from remote end.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] on_app - A function point to handle.
 * @param [in] ctx - Context to on_app.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_on_rtcp_app(litertp_session_t* session, litertp_on_rtcp_app on_app, void* ctx);

/**
 * @brief Create a media stream for rtp session.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] ssrc - Specify ssrc for local stream, 0 indicate genrate randomly.
 * @param [in] trans_mode - Enum rtp_trans_mode_t
 * @param [in] security - Whether use DTLS, if security set true, the library must be build with LITERTP_SSL, or ignore this paramter
 * @param [in] local_address - Local address build into local sdp, if not used, set it to "0.0.0.0"
 * @param [in] local_rtp_port - Local rtp port build into local sdp, this will bind and listen immediately
 * @param [in] local_rtcp_port - Local rtcp port build into local sdp, if local_rtp_port == local_rtcp_port will enable rtcp_mux
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_create_media_stream(litertp_session_t* session, media_type_t mt, uint32_t ssrc, rtp_trans_mode_t trans_mode, bool security,
	const char* local_address, int local_rtp_port, int local_rtcp_port);

/**
 * @brief Remove a media stream from rtp session.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_remove_media_stream(litertp_session_t* session, media_type_t mt);

/**
 * @brief Clear all media streams from rtp session.
 *
 * @param [in] session - Created by litertp_create_session.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_clear_media_streams(litertp_session_t* session);

/**
 * @brief Add local video track.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] codec - Enum codec_type_t.
 * @param [in] pt - Payload type.
 * @param [in] frequency - For video is usually 90000.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_local_video_track(litertp_session_t* session, codec_type_t codec, uint16_t pt, int frequency);

/**
 * @brief Add local audio track.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] codec - Enum codec_type_t.
 * @param [in] pt - Payload type.
 * @param [in] frequency - For audio, this is samplerate.
 * @param [in] channels - Audio channels.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_local_audio_track(litertp_session_t* session, codec_type_t codec, uint16_t pt, int frequency, int channels);


/**
 * @brief Add remote audio track.
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] codec - Enum codec_type_t.
 * @param [in] pt - Payload type.
 * @param [in] frequency - For video is usually 90000.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_remote_video_track(litertp_session_t* session,codec_type_t codec, uint16_t pt, int frequency);

/**
 * @brief Add remote audio track. 
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] codec - Enum codec_type_t.
 * @param [in] pt - Payload type.
 * @param [in] frequency - For audio, this is samplerate.
 * @param [in] channels - Audio channels.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_remote_audio_track(litertp_session_t* session,codec_type_t codec, uint16_t pt, int frequency, int channels);

/**
 * @brief Set remote trans mode.
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] trans_mode - Enum rtp_trans_mode_t.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_trans_mode(litertp_session_t* session, media_type_t mt, rtp_trans_mode_t trans_mode);

/**
 * @brief Set remote ssrc.
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] ssrc - remote ssrc.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_ssrc(litertp_session_t* session, media_type_t mt, uint32_t ssrc);

/**
 * @brief Set remote mid. 
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] mid - remote mid.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_mid(litertp_session_t* session, media_type_t mt, const char* mid);

/**
 * @brief Set remote setup.
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] setup - remote setup.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_setup(litertp_session_t* session, media_type_t mt, sdp_setup_t setup);

/**
 * @brief Set remote rtp endpoint. 
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] family - AF_INET or AF_INET6.
 * @param [in] address - Remote rtp address
 * @param [in] port - Remote rtp port
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_rtp_endpoint(litertp_session_t* session, media_type_t mt, int family, const char* address, int port);

/**
 * @brief Set remote rtcp endpoint.
 * Before call this function must call litertp_create_media_stream.
 * Manually calling add remote track instead of negotiation.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] family - AF_INET or AF_INET6.
 * @param [in] address - Remote rtcp address
 * @param [in] port - Remote rtcp port
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_rtcp_endpoint(litertp_session_t* session, media_type_t mt, int family, const char* address, int port);

/**
 * @brief Add local candidate.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] foundation -
 * @param [in] component -
 * @param [in] address -
 * @param [in] port -
 * @param [in] priority -
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_local_candidate(litertp_session_t* session, media_type_t mt, uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority);

/**
 * @brief Add remote candidate.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] foundation - 
 * @param [in] component - 
 * @param [in] address - 
 * @param [in] port - 
 * @param [in] priority - 
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_remote_candidate(litertp_session_t* session, media_type_t mt, uint32_t foundation, uint32_t component, const char* address, int port, uint32_t priority);

/**
 * @brief Clear local candidate.
 *
 * @param [in] session - Created by litertp_create_session.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_clear_local_candidates(litertp_session_t* session);

/**
 * @brief Clear remote candidate.
 *
 * @param [in] session - Created by litertp_create_session.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_clear_remote_candidates(litertp_session_t* session);


/**
 * @brief Add remote attribute.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] pt - Payload type
 * @param [in] key - attribute key (eg. rtcp-fb, fmtp)
 * @param [in] val - attribute val
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_remote_attribute(litertp_session_t* session,media_type_t mt, uint16_t pt, const char* key, const char* val);

/**
 * @brief Clear remote attribute.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] pt - Payload type
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_clear_remote_attributes(litertp_session_t* session, media_type_t mt, uint16_t pt);


/**
 * @brief Add local attribute.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] pt - Payload type
 * @param [in] key - attribute key (eg. rtcp-fb, fmtp)
 * @param [in] val - attribute val
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_add_local_attribute(litertp_session_t* session, media_type_t mt, uint16_t pt, const char* key, const char* val);

/**
 * @brief Clear local attribute.
 * Before call this function must call litertp_create_media_stream.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t.
 * @param [in] pt - Payload type
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_clear_local_attributes(litertp_session_t* session, media_type_t mt, uint16_t pt);



/**
 * @brief Create offer sdp.
 * Before call this function must initialize local tracks first.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [out] local_sdp - Local sdp string, when No longer needed free it by litertp_free.
 * @param [out] sdp_size - Size of local_sdp
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_create_offer(litertp_session_t* session,char** local_sdp,int* sdp_size);

/**
 * @brief Create answer sdp.
 * Before call this function must initialize local tracks and remote tracks first.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [out] local_sdp - Local sdp string, when No longer needed free it by litertp_free.
 * @param [out] sdp_size - Size of local_sdp
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_create_answer(litertp_session_t* session, char** local_sdp, int* sdp_size);

/**
 * @brief Set remote sdp. this whill initialize and negotiate remote sdp
 * Before call this function must initialize local tracks first.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] sdp - The remote sdp.
 * @param [in] sdp_type - Enum sdp_type_t
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_remote_sdp(litertp_session_t* session,const char* sdp,sdp_type_t sdp_type);

/**
 * @brief Set local sdp. this whill initialize local sdp
 * Before call this function must initialize local tracks first.
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] sdp - The remote sdp.
 * @param [in] sdp_type - Enum sdp_type_t
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_set_local_sdp(litertp_session_t* session, const char* sdp);

/**
 * @brief Require a keyframe
 * Before call this function must initialize local tracks first.
 *
 * @param [in] session - Created by litertp_create_session.
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_require_keyframe(litertp_session_t* session);

/**
 * @brief Send a audio frame
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] frame - Audio frame data
 * @param [in] size - Size of audio frame data
 * @param [in] duration - Samples of audio frame data
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_send_audio_frame(litertp_session_t* session,const uint8_t* frame, uint32_t size, uint32_t duration);

/**
 * @brief Send a video frame
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] frame - Video frame data
 * @param [in] size - Size of Video frame data
 * @param [in] duration - Samples of video frame data.  (eg. 90000/fps)
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_send_video_frame(litertp_session_t* session,const uint8_t* frame, uint32_t size, uint32_t duration);

/**
 * @brief Get stats info
 *
 * @param [in] session - Created by litertp_create_session.
 * @param [in] mt - Enum media_type_t
 * @param [out] stats -  Struct rtp_stats_t
 * @return - Greater than or equal to 0 is successed, otherwise is failed.
 */
LITERTP_API int LITERTP_CALL litertp_get_stats(litertp_session_t* session, media_type_t mt, rtp_stats_t* stats);

#ifdef __cplusplus
}
#endif //__cplusplus
