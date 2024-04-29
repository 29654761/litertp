/**
 * @file receiver_video_h264.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "receiver_video_h264.h"

#include "../litertp_def.h"
#include "../util/sn.hpp"
#include "../h264/nal_header.h"
#include "../log.h"


#include <sys2/util.h>

namespace litertp
{
	receiver_video_h264::receiver_video_h264(int ssrc, media_type_t mt, const sdp_format& fmt)
		:receiver(ssrc,mt,fmt)
	{
	}

	receiver_video_h264::~receiver_video_h264()
	{
	}



	bool receiver_video_h264::insert_packet(packet_ptr pkt)
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
			combin_frame(frame);
		}
		
		check_for_drop();

		

		return true;
	}


	bool receiver_video_h264::find_a_frame(std::vector<packet_ptr>& pkts)
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
			if (!pkt)
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

	void receiver_video_h264::check_for_drop()
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

			nal_header_t fui = { 0 };
			nal_header_set(&fui, pkt->payload()[0]);

			if (fui.nri > 0)  // nal ref idc
			{
				waiting_for_keyframe_ = true;
			}

			LOGD("drop packet %d\n",idx);
			recv_packs_[idx].reset();

			if (pkt->handle_->header->m==1)
			{
				//new start
				frame_begin_ts_ = std::chrono::high_resolution_clock::now();
				break;
			}
		}

		begin_seq_ = i;
	}

	bool receiver_video_h264::combin_frame(const std::vector<packet_ptr>& pkts)
	{
		if (pkts.size() <= 0) {
			return false;
		}

		packet_ptr first_pkt;
		std::string fu_frame_data;
		
		for (auto pkt : pkts)
		{
			const uint8_t* payload = pkt->payload();
			int payload_size = pkt->payload_size();

			if (payload_size < 2)
			{
				continue;
			}

			nal_header_t fui = { 0 };
			nal_header_set(&fui, payload[0]);
			if (fui.f != 0)
			{
				continue;
			}
			av_frame_t frame;
			memset(&frame, 0, sizeof(frame));
			frame.ct = codec_type_h264;
			frame.mt = media_type_video;
			if (fui.t <= 23)
			{
				frame.pts = pkt->handle_->header->ts;
				frame.dts = frame.pts;
				
				std::string frame_data;
				frame_data.reserve(payload_size + 4);
				frame_data.append(3, 0);
				frame_data.append(1, 1);
				frame_data.append((const char*)payload, payload_size);
				
				frame.data = (uint8_t*)frame_data.data();
				frame.data_size = frame_data.size();

				if (fui.t == 5 || fui.t == 7 || fui.t == 8)
				{
					waiting_for_keyframe_ = false;
				}

				if (!waiting_for_keyframe_)
				{
					rtp_frame_event_.invoke(ssrc_, format_, frame);
					stats_.frames_received++;
				}
				else
				{
					stats_.frames_droped++;
				}
			}
			else if (fui.t == 24)  //STAP-A
			{
				frame.pts = pkt->handle_->header->ts;
				frame.dts = frame.pts;
				const uint8_t* buf = payload + 1; //skip fui
				int size = payload_size - 1;

				while (size >= 2)
				{
					uint16_t nal_size = 0;
					nal_size = buf[0] << 8 | buf[1];

					std::string frame_data;
					frame_data.reserve(nal_size + 4);
					frame_data.append(3, 0);
					frame_data.append(1, 1);
					frame_data.append((const char*)buf + 2, nal_size);

					frame.data = (uint8_t*)frame_data.data();
					frame.data_size = frame_data.size();

					nal_header_t nalh = { 0 };
					nal_header_set(&nalh, frame.data[4]);

					if (nalh.t == 5 || nalh.t == 7 || nalh.t == 8)
					{
						waiting_for_keyframe_ = false;
					}

					if (!waiting_for_keyframe_) 
					{
						rtp_frame_event_.invoke(ssrc_, format_, frame);
						stats_.frames_received++;
					}
					else
					{
						stats_.frames_droped++;
					}
					buf += nal_size + 2;
					size -= nal_size + 2;
				}
			}
			else if (fui.t == 25)  //STAP-B
			{
				frame.pts = pkt->handle_->header->ts;
				frame.dts = frame.pts;
				uint16_t don = payload[1] << 8 | payload[2];
				const uint8_t* buf = payload + 3; // skip fui bite and DON
				int size = payload_size - 3;
				while (size >= 2)
				{
					uint16_t nal_size = 0;
					nal_size = buf[0] << 8 | buf[1];

					std::string frame_data;
					frame_data.clear();
					frame_data.reserve(nal_size + 4);
					frame_data.append(3, 0);
					frame_data.append(1, 1);
					frame_data.append((const char*)buf + 2, nal_size);

					frame.data = (uint8_t*)frame_data.data();
					frame.data_size = frame_data.size();

					nal_header_t nalh = { 0 };
					nal_header_set(&nalh, frame.data[4]);
					if (nalh.t == 5 || nalh.t == 7 || nalh.t == 8)
					{
						waiting_for_keyframe_ = false;
					}

					if (!waiting_for_keyframe_)
					{
						rtp_frame_event_.invoke(ssrc_, format_, frame);
						stats_.frames_received++;
					}
					else
					{
						stats_.frames_droped++;
					}
					buf += nal_size + 2;
					size -= nal_size + 2;
				}
			}
			else if (fui.t == 26)  //MTAP-A
			{
				int64_t ts = pkt->handle_->header->ts;
				uint16_t donb = payload[1] << 8 | payload[2];
				const uint8_t* buf = payload + 3; // skip fui bite and DON
				int size = payload_size - 3;
				while (size >= 2)
				{
					uint16_t nal_size = 0;
					nal_size = buf[0] << 8 | buf[1];
					uint8_t dond = buf[2];
					int32_t ts_offset = buf[3] << 8 | buf[4];

					std::string frame_data;
					frame_data.reserve(nal_size + 4);
					frame_data.append(3, 0);
					frame_data.append(1, 1);
					frame_data.append((const char*)buf + 5, nal_size);

					frame.data = (uint8_t*)frame_data.data();
					frame.data_size = frame_data.size();

					frame.pts = ts + ts_offset;
					frame.dts = frame.pts;

					nal_header_t nalh = { 0 };
					nal_header_set(&nalh, frame.data[4]);
					if (nalh.t == 5 || nalh.t == 7 || nalh.t == 8)
					{
						waiting_for_keyframe_ = false;
					}

					if (!waiting_for_keyframe_)
					{
						rtp_frame_event_.invoke(ssrc_, format_, frame);
						stats_.frames_received++;
					}
					else
					{
						stats_.frames_droped++;
					}
					buf += nal_size + 2;
					size -= nal_size + 2;
				}
			}
			else if (fui.t == 27) //MTAP-B
			{
				int64_t ts = pkt->handle_->header->ts;
				uint16_t donb = payload[1] << 8 | payload[2];
				const uint8_t* buf = payload + 3; // skip fui bite and DON
				int size = payload_size - 3;
				while (size >= 2)
				{
					uint16_t nal_size = 0;
					nal_size = buf[0] << 8 | buf[1];
					uint8_t dond = buf[2];
					int32_t ts_offset = buf[3] << 16 | buf[4] << 8 | buf[5];

					std::string frame_data;
					frame_data.reserve(nal_size + 4);
					frame_data.append(3, 0);
					frame_data.append(1, 1);
					frame_data.append((const char*)buf + 6, nal_size);

					frame.data = (uint8_t*)frame_data.data();
					frame.data_size = frame_data.size();

					frame.pts = ts + ts_offset;
					frame.dts = frame.pts;

					nal_header_t nalh = { 0 };
					nal_header_set(&nalh, frame.data[4]);
					if (nalh.t == 5 || nalh.t == 7 || nalh.t == 8)
					{
						waiting_for_keyframe_ = false;
					}

					if (!waiting_for_keyframe_)
					{
						rtp_frame_event_.invoke(ssrc_, format_, frame);
						stats_.frames_received++;
					}
					else
					{
						stats_.frames_droped++;
					}
					buf += nal_size + 2;
					size -= nal_size + 2;
				}
			}
			else if (fui.t == 28 || fui.t == 29)  //FU-A
			{
				frame.pts = pkt->handle_->header->ts;
				frame.dts = frame.pts;


				fu_header_t fuh = { 0 };
				fu_header_set(&fuh, payload[1]);

				if (fuh.s == 1)
				{
					first_pkt = pkt;

					fu_frame_data = "";
					fu_frame_data.clear();
					fu_frame_data.reserve(MAX_RTP_PAYLOAD_SIZE* pkts.size() + 4);
					fu_frame_data.append(3, 0);
					fu_frame_data.append(1, 1);

					nal_header_t nalh = { 0 };
					nalh.f = fui.f;
					nalh.nri = fui.nri;
					nalh.t = fuh.t;
					uint8_t nalhv = nal_header_get(&nalh);
					fu_frame_data.append(1, nalhv);
				}
				if (!first_pkt)
				{
					continue;
				}

				


				int skip_nal_data = 2;
				if (fui.t == 29) //FU-B
				{
					skip_nal_data += 2;  //skip don;
				}

				const uint8_t* buf = payload + skip_nal_data; //skip fui and fu header (and don for FU-B)
				size_t size = payload_size - skip_nal_data;
				fu_frame_data.append((const char*)buf, size);

			}
			else
			{
				return false;
			}
		}


		if (fu_frame_data.size() > 0)
		{
			av_frame_t frame;
			memset(&frame, 0, sizeof(frame));
			frame.ct = codec_type_h264;
			frame.mt = media_type_video;
			frame.pts = first_pkt->handle_->header->ts;
			frame.dts = frame.pts;
			frame.data = (uint8_t*)fu_frame_data.data();
			frame.data_size = fu_frame_data.size();

			nal_header_t nalh = { 0 };
			nal_header_set(&nalh, frame.data[4]);
			if (nalh.t == 5 || nalh.t == 7 || nalh.t == 8)
			{
				waiting_for_keyframe_ = false;
			}

			if (!waiting_for_keyframe_)
			{
				rtp_frame_event_.invoke(ssrc_, format_, frame);
				stats_.frames_received++;
			}
			else
			{
				stats_.frames_droped++;
			}
		}

		return true;
	}


}
