/**
 * @file receiver_video_vp8.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "receiver_video_vp8.h"

#include "../litertp_def.h"
#include "../util/sn.hpp"
#include "../vpx/vpx_header.h"
#include "../log.h"

namespace litertp
{
	receiver_video_vp8::receiver_video_vp8(int ssrc, media_type_t mt, const sdp_format& fmt)
		:receiver(ssrc,mt, fmt)
	{
	}

	receiver_video_vp8::~receiver_video_vp8()
	{
	}

	bool receiver_video_vp8::insert_packet(packet_ptr pkt)
	{
		std::unique_lock<std::shared_mutex>lk(mutex_);
		if (!receiver::insert_packet(pkt))
		{
			return false;
		}

		std::vector<packet_ptr> frame;
		while (find_a_frame(frame))
		{
			// A completed nal
			frame_begin_ts_ = std::chrono::high_resolution_clock::now();
			combin_frame(frame);
		}
		check_for_drop();

		

		return true;
	}


	bool receiver_video_vp8::find_a_frame(std::vector<packet_ptr>& pkts)
	{
		pkts.clear();
		if (begin_seq_ < 0 || end_seq_ < 0|| sn::ahead_of<uint16_t>(begin_seq_, end_seq_))
		{
			return false;
		}

		

		std::vector<int> idx_lst;
		uint16_t i = begin_seq_;
		bool detected = false;
		while (!sn::ahead_of<uint16_t>(i, end_seq_))
		{
			int pos = i % PACKET_BUFFER_SIZE;
			i++;
			auto pkt = recv_packs_[pos];
			if (pkt == nullptr)
			{
				return false;
			}

			idx_lst.push_back(pos);

			if (pkt->handle_->header->m==1)
			{
				detected = true;
				break;
			}
		}

		if (!detected||idx_lst.size() == 0)
		{
			return false;
		}

		begin_seq_ = i;
		frame_begin_ts_ = std::chrono::high_resolution_clock::now();

		pkts.reserve(idx_lst.size());
		for (int idx : idx_lst)
		{
			pkts.push_back(recv_packs_[idx]);
			recv_packs_[idx].reset();
		}

		return true;
	}

	void receiver_video_vp8::check_for_drop()
	{
		if (begin_seq_ < 0 || end_seq_ < 0 || sn::ahead_of<uint16_t>(begin_seq_, end_seq_))
		{
			return;
		}
		if (!is_timeout())
		{
			return;
		}

		uint16_t i = begin_seq_;
		while (!sn::ahead_of<uint16_t>(i, end_seq_))
		{
			int idx = i % PACKET_BUFFER_SIZE;
			i++;
			auto pkt = recv_packs_[idx];
			if (!pkt)
			{
				continue;
			}
			LOGD("drop packet %d\n", idx);
			recv_packs_[idx].reset();

			vp8_header h = { 0 };
			int hsize = 0;
			vp8_header_deserialize(&h, &hsize, pkt->payload(), pkt->payload_size());
			if (h.non_reference == 0)  // nal ref idc
			{
				waiting_for_keyframe_ = true;
				break;
			}

			if (pkt->handle_->header->m==1)
			{
				//new start
				frame_begin_ts_ = std::chrono::high_resolution_clock::now();
				break;
			}
		}

		begin_seq_ = i;
		frame_begin_ts_ = std::chrono::high_resolution_clock::now();
	}

	//https://datatracker.ietf.org/doc/html/rfc7741#section-4.4.
	bool receiver_video_vp8::combin_frame(const std::vector<packet_ptr>& pkts)
	{
		if (pkts.size() <= 0) {
			return false;
		}

		packet_ptr first_pkt;
		vp8_header first_pkt_vp8_header = { 0 };

		std::string frame_data;
		frame_data.reserve(pkts.size()*MAX_RTP_PAYLOAD_SIZE);
		for (auto pkt : pkts)
		{
			const uint8_t* payload = pkt->payload();
			int payload_size = pkt->payload_size();

			vp8_header h = { 0 };
			int vp8_headersize=0;
			int pos=vp8_header_deserialize(&h,&vp8_headersize, payload, payload_size);
			if (pos < 0)
			{
				continue;
			}
			if (h.startof_vp8_partition)
			{
				first_pkt = pkt;
				first_pkt_vp8_header = h;
			}
			if (!first_pkt)
			{
				continue;
			}

			int frameSize = payload_size - vp8_headersize;
			if (frameSize <= 0)
			{
				continue;
			}
			frame_data.append((const char*)payload + vp8_headersize, frameSize);

			if (pkt->handle_->header->m == 1)
			{
				break;
			}
		}


		if (!first_pkt) 
		{
			return false;
		}

		if (first_pkt_vp8_header.startof_vp8_partition == 1 && first_pkt_vp8_header.part_index == 0&& first_pkt_vp8_header.p == 0)
		{
			//is key frame.
			waiting_for_keyframe_ = false;
		}

		av_frame_t frame;
		memset(&frame, 0, sizeof(frame));
		frame.ct = codec_type_vp8;
		frame.mt = media_type_video;
		frame.pts = first_pkt->handle_->header->ts;
		frame.dts = frame.pts;
		//frame.ft = first_pkt_vp8_header.show_frame==1?frame_type_iframe:frame_type_pframe;
		frame.data = (uint8_t*)frame_data.data();
		frame.data_size = frame_data.size();


		if (!waiting_for_keyframe_)
		{
			rtp_frame_event_.invoke(ssrc_, format_, frame);
			stats_.frames_received++;
		}
		else
		{
			stats_.frames_droped++;
		}

		return true;
	}



}
