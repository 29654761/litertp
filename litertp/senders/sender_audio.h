/**
 * @file sender_audio.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "sender.h"

namespace litertp
{
	class sender_audio:public sender
	{
	public:
		sender_audio(uint32_t ssrc, media_type_t mt, const sdp_format& fmt);
		virtual ~sender_audio();

		bool send_frame(const uint8_t* frame, uint32_t size, uint32_t duration);


	};


}

