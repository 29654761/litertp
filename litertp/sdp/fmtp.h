/**
 * @file fmtp.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <string>
#include <map>

namespace litertp {

	class fmtp
	{
	public:
		fmtp();
		~fmtp();

		void parse(const std::string& s);
		std::string to_string()const;

		std::string get(const std::string& key, const std::string& def = "");
		int64_t get(const std::string& key, int64_t def = 0);
		uint64_t get(const std::string& key, uint64_t def = 0);
		uint64_t get_hex(const std::string& key, uint64_t def = 0);
		long double get(const std::string& key, long double def = 0);

		void set(const std::string& key, const std::string& val);
		void set(const std::string& key, int64_t val);
		void set(const std::string& key, uint64_t val);
		void set(const std::string& key, double val);

	private:
		std::map<std::string, std::string> map_;
	};


}