/**
 * @file sender_audio_aac.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#include "sender_audio_aac.h"

namespace litertp
{
	sender_audio_aac::sender_audio_aac(uint32_t ssrc, media_type_t mt, const sdp_format& fmt)
		:sender(ssrc,mt,fmt)
	{
	}

	sender_audio_aac::~sender_audio_aac()
	{
	}


	bool sender_audio_aac::send_frame(const uint8_t* frame, uint32_t size, uint32_t duration)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);

		if (format_.codec_ == codec_type_mpeg4_generic)
		{
			return send_frame_rfc3640(frame,size,duration);
		}
		else if (format_.codec_ == codec_type_mp4a_latm)
		{
			return send_frame_rfc3016(frame, size, duration);
		}
		else 
		{
			return false;
		}
	}

	bool sender_audio_aac::send_frame_rfc3640(const uint8_t* frame, uint16_t size, uint32_t duration)
	{
		packet_ptr pkt = std::make_shared<packet>(format_.payload_type_, ssrc_, seq_, timestamp_);
		// RFC3551 specifies that for audio the marker bit should always be 0 except for when returning
		// from silence suppression. For video the marker bit DOES get set to 1 for the last packet
		// in a frame.
		pkt->handle_->header->m = 0;

		std::string data;
		data.reserve(MAX_RTP_PAYLOAD_SIZE);

		// one au header
		data.append(1,(char)0x00);
		data.append(1,(char)0x10);

		// 13bits au data size
		uint16_t c = size << 3;
		data.append(1,(char)((c & 0xFF00) >> 8));
		data.append(1, (char)(c & 0x00F8));

		// au data
		data.append((const char*)frame, size);

		pkt->set_payload((const uint8_t*)data.data(), data.size());


		timestamp_ += duration;

		return send_packet(pkt);

	}

	bool sender_audio_aac::send_frame_rfc3016(const uint8_t* frame, uint16_t size, uint32_t duration)
	{
		packet_ptr pkt = std::make_shared<packet>(format_.payload_type_, ssrc_, seq_, timestamp_);
		// RFC3551 specifies that for audio the marker bit should always be 0 except for when returning
		// from silence suppression. For video the marker bit DOES get set to 1 for the last packet
		// in a frame.
		pkt->handle_->header->m = 0;

		std::string data;
		data.reserve(MAX_RTP_PAYLOAD_SIZE);

		//au size
		int c=size / 0xFF;
		for (int i = 0; i < c; i++)
		{
			data.append(1, (char)0xFF);
		}
		data.append(1, (char)(size % 0xFF));

		// au data
		data.append((const char*)frame, size);

		pkt->set_payload((const uint8_t*)data.data(), data.size());
		
		timestamp_ += duration;

		return send_packet(pkt);
	}


}
