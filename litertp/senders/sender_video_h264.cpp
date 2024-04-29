/**
 * @file sender_video_h264.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#include "sender_video_h264.h"
#include "../h264/nal_header.h"

namespace litertp
{
	sender_video_h264::sender_video_h264(uint32_t ssrc, media_type_t mt, const sdp_format& fmt)
		:sender(ssrc,mt,fmt)
	{
	}

	sender_video_h264::~sender_video_h264()
	{
	}


	bool sender_video_h264::send_frame(const uint8_t* frame, uint32_t size, uint32_t duration)
	{
        std::unique_lock<std::shared_mutex>lk(mutex_);
        int offset = 0;
        int nal_start = 0, nal_size = 0;
        bool islast = false;
        while (find_next_nal(frame, size, offset, nal_start, nal_size, islast))
        {
            send_nal(duration, frame + nal_start, nal_size, islast);
        }
        return true;
	}

    void sender_video_h264::send_nal(uint32_t duration, const uint8_t* nal, uint32_t nal_size, bool islast)
    {
        if (nal_size <=MAX_RTP_PAYLOAD_SIZE)
        {
            // Send as Single-Time Aggregation Packet (STAP-A).
            packet_ptr pkt = std::make_shared<packet>(format_.payload_type_, ssrc_, seq_, timestamp_);
            pkt->handle_->header->m = islast ? 1 : 0;
            pkt->set_payload(nal, nal_size);
            this->send_packet(pkt);
        }
        else // Send as Fragmentation Unit A (FU-A):
        {
            uint8_t nal0 = nal[0];
            nal_header_t nal_header;
            nal_header_set(&nal_header, nal0);
            int pos = 1; //skip nal0

            nal_header_t fu_ind;
            fu_ind.f = nal_header.f;
            fu_ind.nri = nal_header.nri;
            fu_ind.t = 28;  //FU-A
            uint8_t fu_ind_v = nal_header_get(&fu_ind);


            for (int index = 0; index * MAX_RTP_PAYLOAD_SIZE < nal_size; index++)
            {
                int offset = index * MAX_RTP_PAYLOAD_SIZE;
                int payload_length = ((index + 1) * MAX_RTP_PAYLOAD_SIZE < nal_size) ? MAX_RTP_PAYLOAD_SIZE : nal_size - index * MAX_RTP_PAYLOAD_SIZE;

                bool is_first_packet = index == 0;
                bool is_final_packet = (index + 1) * MAX_RTP_PAYLOAD_SIZE >= nal_size;

                packet_ptr pkt = std::make_shared<packet>(format_.payload_type_, ssrc_, seq_, timestamp_);
                pkt->handle_->header->m= (islast && is_final_packet) ? 1 : 0;

                fu_header_t fu;
                fu.t = nal_header.t;
                if (is_first_packet) {
                    fu.s = 1;
                    fu.e = 0;
                    fu.r = 0;
                }
                else if (is_final_packet) {
                    fu.s = 0;
                    fu.e = 1;
                    fu.r = 0;
                }
                else {
                    fu.s = 0;
                    fu.e = 0;
                    fu.r = 0;
                }

                std::string s;
                s.reserve(MAX_RTP_PAYLOAD_SIZE);
                s.append(1, fu_ind_v);
                s.append(1, fu_header_get(&fu));
                s.append((const char*)nal + pos, payload_length);
                pos += payload_length;
                pkt->set_payload((const uint8_t*)s.data(), s.size());

                this->send_packet(pkt);

            }
        }

        if (islast)
        {
            timestamp_ += duration;
        }
    }

	bool sender_video_h264::find_next_nal(const uint8_t* buffer, int size, int& offset, int& nal_start, int& nal_size,bool& islast)
	{
        int zeroes = 0;
        // Parse NALs from H264 access unit, encoded as an Annex B bitstream.
        // NALs are delimited by 0x000001 or 0x00000001.
        int cur_pos = offset;
        nal_start = 0;
        nal_size = 0;
        islast = false;
        for (int i = offset; i < size; i++)
        {
            if (buffer[i] == 0x00)
            {
                zeroes++;
            }
            else if (buffer[i] == 0x01 && zeroes >= 2)
            {
                // This is a NAL start sequence.
                int nal_start2 = i + 1;
                if (nal_start > 0)
                {
                    int end_pos = nal_start2 - (zeroes == 2 ? 3 : 4);
                    nal_size = end_pos - cur_pos;
                    islast = cur_pos + nal_size == size;
                    offset = end_pos;
                    return true;
                }
                else
                {
                    nal_start = nal_start2;
                }

                cur_pos = nal_start;
            }
            else
            {
                zeroes = 0;
            }
        }

        if (cur_pos < size)
        {
            nal_start = cur_pos;
            nal_size = size - nal_start;
            offset = size;
            islast = true;
            return true;
        }

        return false;
	}

}
