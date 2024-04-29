/**
 * @file receiver_audio_aac.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include "receiver.h"

namespace litertp
{
	class receiver_audio_aac:public receiver
	{
	public:
		receiver_audio_aac(int ssrc, media_type_t mt, const sdp_format& fmt);
		virtual ~receiver_audio_aac();

		virtual bool insert_packet(packet_ptr pkt);
	private:
		void find_a_frame();
		bool process_rfc3640_frame(packet_ptr pkt);
		bool process_rfc3016_frame(packet_ptr pkt);

	private:
		
	};


}