/**
 * @file fmtp.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#include "fmtp.h"


#include <sys2/string_util.h>
#include <sstream>

namespace litertp {

	fmtp::fmtp()
	{
	}

	fmtp::~fmtp()
	{
	}

	void fmtp::parse(const std::string& s)
	{
		auto vec=sys::string_util::split(s, ";");
		map_.clear();
		for (auto& s2 : vec)
		{
			auto vec2 = sys::string_util::split(s2, "=");
			if (vec2.size() >= 2)
			{
				map_[vec2[0]] = vec2[1];
			}
		}
	}

	std::string fmtp::to_string()const
	{
		std::stringstream ss;
		for (auto& kv : map_)
		{
			ss << kv.first << "=" << kv.second<<";";
		}
		if (map_.size() > 0)
		{
			ss.seekp(-1, std::ios_base::end);
		}
		return ss.str();
	}

	std::string fmtp::get(const std::string& key, const std::string& def)
	{
		auto itr=map_.find(key);
		if (itr == map_.end())
		{
			return def;
		}

		return itr->second;
	}

	int64_t fmtp::get(const std::string& key, int64_t def)
	{
		auto itr = map_.find(key);
		if (itr == map_.end())
		{
			return def;
		}

		char* endptr = nullptr;
		return strtoll(itr->second.c_str(),&endptr,0);
	}

	uint64_t fmtp::get(const std::string& key, uint64_t def)
	{
		auto itr = map_.find(key);
		if (itr == map_.end())
		{
			return def;
		}

		char* endptr = nullptr;
		return strtoull(itr->second.c_str(), &endptr, 0);
	}

	uint64_t fmtp::get_hex(const std::string& key, uint64_t def)
	{
		auto itr = map_.find(key);
		if (itr == map_.end())
		{
			return def;
		}

		char* endptr = nullptr;
		return strtoull(itr->second.c_str(), &endptr, 16);
	}

	long double fmtp::get(const std::string& key, long double def)
	{
		auto itr = map_.find(key);
		if (itr == map_.end())
		{
			return def;
		}

		char* endptr = nullptr;
		return strtold(itr->second.c_str(), &endptr);
	}

	void fmtp::set(const std::string& key, const std::string& val)
	{
		map_[key] = val;
	}

	void fmtp::set(const std::string& key, int64_t val)
	{
		map_[key] = std::to_string(val);
	}
	void fmtp::set(const std::string& key, uint64_t val)
	{
		map_[key] = std::to_string(val);
	}

	void fmtp::set(const std::string& key, double val)
	{
		map_[key] = std::to_string(val);
	}
}