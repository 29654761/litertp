
/**
 * @file sdp_pair.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sdp_pair.h"

#include <sstream>
#include "sys2/string_util.h"


namespace litertp {

	sdp_pair::sdp_pair(const std::string& separator)
	{
		separator_ = separator;
	}

	sdp_pair::sdp_pair(const std::string& key, const std::string& val, const std::string& separator)
	{
		key_ = key;
		val_ = val;
		separator_ = separator;
	}

	sdp_pair::~sdp_pair()
	{

	}

	std::string sdp_pair::to_string()const
	{
		if (val_.size() > 0) {
			return key_ + separator_ + val_;
		}
		else {
			return key_;
		}
	}

	bool sdp_pair::parse(const std::string& s)
	{
		std::vector<std::string> vec;
		sys::string_util::split(s, separator_, vec, 2);
		if (vec.size() >= 1) {
			key_ = vec[0];
		}

		if (vec.size() >= 2) {
			val_ = vec[1];
		}
		return true;
	}

}
