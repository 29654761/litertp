/**
 * @file sender_audio.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sender_audio.h"

namespace litertp
{
	sender_audio::sender_audio(uint32_t ssrc, media_type_t mt, const sdp_format& fmt)
		:sender(ssrc,mt,fmt)
	{
	}

	sender_audio::~sender_audio()
	{
	}


	bool sender_audio::send_frame(const uint8_t* frame, uint32_t size, uint32_t duration)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);
		uint32_t payload_duration = 0;
		for (int index = 0; index * MAX_RTP_PAYLOAD_SIZE < size; index++)
		{
			int offset = (index == 0) ? 0 : (index * MAX_RTP_PAYLOAD_SIZE);
			int payload_length = (offset + MAX_RTP_PAYLOAD_SIZE < size) ? MAX_RTP_PAYLOAD_SIZE : size - offset;
			

			packet_ptr pkt = std::make_shared<packet>(format_.payload_type_, ssrc_, seq_, timestamp_);
			
			// RFC3551 specifies that for audio the marker bit should always be 0 except for when returning
			// from silence suppression. For video the marker bit DOES get set to 1 for the last packet
			// in a frame.
			pkt->handle_->header->m = 0;

			pkt->set_payload((const uint8_t*)(frame + offset), payload_length);
			this->send_packet(pkt);
			
			//logger.LogDebug($"send audio { audioRtpChannel.RTPLocalEndPoint}->{AudioDestinationEndPoint}.");

			payload_duration = (uint32_t)((payload_length*1.0 / size) * duration);
			timestamp_ += payload_duration;
		}


		return true;
	}






}
