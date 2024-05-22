/**
 * @file receiver_audio_aac.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "receiver_audio_aac.h"

#include "../litertp_def.h"
#include "../util/sn.hpp"

#include <string.h>

namespace litertp
{
	receiver_audio_aac::receiver_audio_aac(int ssrc, media_type_t mt, const sdp_format& fmt)
		:receiver(ssrc,mt,fmt)
	{
		nack_max_count_ = 3;
	}

	receiver_audio_aac::~receiver_audio_aac()
	{
	}

	bool receiver_audio_aac::insert_packet(packet_ptr pkt)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);
		if (!receiver::insert_packet(pkt))
		{
			return false;
		}

		find_a_frame();
		return true;
	}


	void receiver_audio_aac::find_a_frame()
	{
		if (begin_seq_ < 0 || end_seq_ < 0|| sn::ahead_of<uint16_t>(begin_seq_, end_seq_))
		{
			return;
		}

		uint16_t i = begin_seq_;
		while (!sn::ahead_of<uint16_t>(i, end_seq_))
		{
			int pos = i % PACKET_BUFFER_SIZE;

			auto pkt = recv_packs_[pos];
			if (pkt == nullptr)
			{
				if (is_timeout())
				{
					i++; //drop this packet;
					stats_.frames_droped++;
					continue;
				}
				else 
				{
					break;
				}
			}

			int b = false;
			if (format_.codec_ == codec_type_mpeg4_generic) 
			{
				b=process_rfc3640_frame(pkt);
			}
			else if (format_.codec_ == codec_type_mp4a_latm)
			{
				b=process_rfc3016_frame(pkt);
			}
			
			if (!b)
			{
				stats_.frames_droped++;
			}
			recv_packs_[pos].reset();



			i++;
		}

		begin_seq_ = i;
		frame_begin_ts_ = std::chrono::high_resolution_clock::now();
	}

	bool receiver_audio_aac::process_rfc3640_frame(packet_ptr pkt)
	{
		const uint8_t* payload = pkt->payload();
		size_t payload_size = pkt->payload_size();

		if (payload_size < 2)
		{
			return false;
		}

		int pos = 0;

		uint16_t hdr_lens = payload[pos++] << 8;
		hdr_lens |= payload[pos++];
		uint16_t c = hdr_lens / 16;


		if (payload_size < pos + c * 2)
		{
			return false;
		}
		const uint8_t* audata = payload + pos + c * 2;

		int aupos = 0;
		for (uint16_t i = 0; i < c; i++)
		{
			uint16_t ausize = (payload[pos++] << 5);
			ausize |= (payload[pos++] >> 3);
			
			if (payload_size < pos + aupos + ausize)
			{
				break;
			}

			av_frame_t frame;
			memset(&frame, 0, sizeof(frame));
			frame.ct = format_.codec_;
			frame.mt = media_type_audio;
			frame.pts = pkt->handle_->header->ts;
			frame.dts = frame.pts;
			frame.data = (uint8_t*)(audata + aupos);
			frame.data_size = ausize;

			rtp_frame_event_.invoke(ssrc_, format_, frame);
			stats_.frames_received++;

			aupos += ausize;
		}


		return true;
	}

	bool receiver_audio_aac::process_rfc3016_frame(packet_ptr pkt)
	{
		const uint8_t* payload = pkt->payload();
		size_t payload_size = pkt->payload_size();

		size_t pos = 0;
		while (pos<payload_size)
		{
			int c = 0;
			uint8_t v = 0;
			for (;;)
			{
				if (pos+1 >= payload_size)
				{
					break;
				}
				v = payload[pos++];
				if (v != 0xFF)
				{
					break;
				}
				c++;
			}

			uint32_t ausize = c * 0xFF + v;

			if (pos + ausize > payload_size)
			{
				break;
			}

			av_frame_t frame;
			memset(&frame, 0, sizeof(frame));
			frame.ct = format_.codec_;
			frame.mt = media_type_audio;
			frame.pts = pkt->handle_->header->ts;
			frame.dts = frame.pts;
			frame.data = (uint8_t*)(payload + pos);
			frame.data_size = ausize;

			rtp_frame_event_.invoke(ssrc_, format_, frame);
			stats_.frames_received++;

			pos += ausize;
		}
		return true;
	}
}
