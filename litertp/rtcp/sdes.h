/**
 * @file sdes.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <string>
#include <vector>

#include "litertp/proto/rtcp_sdes.h"

namespace litertp {
	namespace rtcp {



		class sdes
		{
		public:
			sdes();
			~sdes();

			size_t size()const;
			bool add_item(uint32_t src, rtcp_sdes_type type, const char* data, size_t size);
			void remove_item(uint32_t src, rtcp_sdes_type type);
			void remove_entry(uint32_t src);
			bool get_item(uint32_t src, rtcp_sdes_type type, rtcp_sdes_item& item);


			bool serialize(std::string& buffer);
			bool parse(const uint8_t* buffer, size_t size);

		public:
			rtcp_sdes* packet_;
		};



	}
}
