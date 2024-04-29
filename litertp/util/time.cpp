/**
 * @file time.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "time.h"

namespace litertp {
	namespace time_util {

		double cur_time()
		{
			auto tp = std::chrono::system_clock::now();
			auto dur = tp.time_since_epoch();
			typedef std::chrono::duration<double, std::ratio<1, 1>> duration_type;
			auto ts = std::chrono::duration_cast<duration_type>(dur);
			return ts.count();
		}

	}
}