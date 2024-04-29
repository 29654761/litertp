/**
 * @file nack_pid_bid.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <stdint.h>

namespace litertp
{
	class nack_pid_bid
	{
	public:
		nack_pid_bid();
		~nack_pid_bid();

		bool add(uint16_t seq);
		void reset() {
			pid_ = 0;
			bid_ = 0;
			has_pid_ = false;
		}
	public:
		uint16_t pid_ = 0;
		uint16_t bid_ = 0;

		bool has_pid_ = false;
	};


}