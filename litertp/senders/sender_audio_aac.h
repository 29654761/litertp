/**
 * @file sender_audio_aac.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "sender.h"

namespace litertp
{
	class sender_audio_aac:public sender
	{
	public:
		sender_audio_aac(uint32_t ssrc, media_type_t mt, const sdp_format& fmt);
		virtual ~sender_audio_aac();

		bool send_frame(const uint8_t* frame, uint32_t size, uint32_t duration);

	private:
		bool send_frame_rfc3640(const uint8_t* frame, uint16_t size, uint32_t duration);
		bool send_frame_rfc3016(const uint8_t* frame, uint16_t size, uint32_t duration);
	};


}

