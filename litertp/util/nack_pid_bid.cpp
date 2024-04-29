/**
 * @file nack_pid_bid.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "nack_pid_bid.h"

namespace litertp
{
	nack_pid_bid::nack_pid_bid()
	{
	}

	nack_pid_bid::~nack_pid_bid()
	{
	}

	bool nack_pid_bid::add(uint16_t seq)
	{
		if (!has_pid_)
		{
			pid_ = seq;
			has_pid_=true;
			return true;
		}
		else
		{
			uint16_t pos = seq - pid_;
			if (pos >= 16)
			{
				return false;
			}

			bid_ |= (0x0001 << pos);
			return true;
		}
	}
}