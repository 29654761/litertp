/**
 * @file receiver_video_vp8.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once

#include "receiver.h"



namespace litertp
{

	class receiver_video_vp8 :public receiver
	{
	public:
		receiver_video_vp8(int ssrc, media_type_t mt, const sdp_format& fmt);
		virtual ~receiver_video_vp8();

		virtual bool insert_packet(packet_ptr pkt);
	private:
		bool find_a_frame(std::vector<packet_ptr>& pkts);

		//check and drop the broken frame;
		void check_for_drop();



		bool combin_frame(const std::vector<packet_ptr>& pkts);

	

		
	};


}