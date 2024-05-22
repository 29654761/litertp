#include "uri.h"
#include "string_util.h"
#include <sstream>
#include <algorithm>

namespace sys {

	uri::uri()
	{
	}

	uri::~uri()
	{
	}

	void uri::add(const std::string& name, const std::string& value)
	{
		args_.push_back(std::make_pair(name, value));
	}

	void uri::remove(const std::string& name)
	{
		auto itr = std::find_if(args_.begin(), args_.end(), [&name](const std::pair<std::string, std::string>& kv) {
			return kv.first == name;
			});

		if (itr != args_.end())
		{
			args_.erase(itr);
		}
	}

	void uri::clear()
	{
		args_.clear();
	}


	const std::string& uri::get(const std::string& name, const std::string& def_value)const
	{
		auto itr = std::find_if(args_.begin(), args_.end(), [&name](const std::pair<std::string, std::string>& kv) {
			return kv.first == name;
			});

		if (itr == args_.end())
		{
			return def_value;
		}
		return itr->second;
	}

	void uri::get(const std::string& name, std::vector<std::string>& values)const
	{
		values.clear();
		for (auto itr = args_.begin(); itr != args_.end(); itr++)
		{
			if (itr->first == name)
			{
				values.push_back(itr->second);
			}
		}
	}

	int64_t uri::get_int64(const std::string& name, int64_t def_value)const
	{
		auto itr = std::find_if(args_.begin(), args_.end(), [&name](const std::pair<std::string, std::string>& kv) {
			return kv.first == name;
			});

		if (itr == args_.end())
		{
			return def_value;
		}

		char* endptr = nullptr;
		return strtoll(itr->second.c_str(), &endptr, 0);
	}

	uint64_t uri::get_uint64(const std::string& name, uint64_t def_value)const
	{
		auto itr = std::find_if(args_.begin(), args_.end(), [&name](const std::pair<std::string, std::string>& kv) {
			return kv.first == name;
			});

		if (itr == args_.end())
		{
			return def_value;
		}

		char* endptr = nullptr;
		return strtoull(itr->second.c_str(), &endptr, 0);
	}

	double uri::get_double(const std::string& name, double def_value)const
	{
		auto itr = std::find_if(args_.begin(), args_.end(), [&name](const std::pair<std::string, std::string>& kv) {
			return kv.first == name;
			});

		if (itr == args_.end())
		{
			return def_value;
		}

		char* endptr = nullptr;
		return strtod(itr->second.c_str(), &endptr);
	}


	bool uri::parse(const std::string& s)
	{
		reset();

		auto vec = sys::string_util::split(s, "://", 2);
		if (vec.size() < 2)
		{
			return false;
		}

		protocal_ = vec[0];

		auto vec_hp_q = sys::string_util::split(vec[1], "?", 2);
		if (vec_hp_q.size() > 1)
		{
			if (!parse_query_string(vec_hp_q[1]))
			{
				return false;
			}
		}

		auto vec_host_path = sys::string_util::split(vec_hp_q[0], "/", 2);
		if (vec_host_path.size() > 1)
		{
			path_ = vec_host_path[1];
		}

		auto vec_host_port = sys::string_util::split(vec_host_path[0], ":", 2);
		host_ = vec_host_port[0];
		if (vec_host_port.size() > 1)
		{
			char* endptr = nullptr;
			port_ = strtol(vec_host_port[1].c_str(), &endptr, 10);
			is_default_port_ = false;
		}
		else {
			port_ = 0;
			is_default_port_ = true;
		}


		return true;
	}

	std::string uri::to_string()const
	{
		std::stringstream ss;
		ss << protocal_ << "://" << host_;
		if (!is_default_port_)
		{
			ss << ":" << port_;
		}
		if (path_.size() > 0)
		{
			if (path_[0] != '/')
			{
				ss << "/";
			}
			ss << path_;
		}
		if (args_.size() > 0)
		{
			ss << "?" << to_query_string();
		}

		return ss.str();
	}


	bool uri::parse_query_string(const std::string& s)
	{
		clear();
		auto vec = sys::string_util::split(s, "&");
		for (auto itr = vec.begin(); itr != vec.end(); itr++)
		{
			auto vec2 = sys::string_util::split(*itr, "=", 2);
			if (vec2.size() >= 2)
			{
				add(vec2[0], url_decode(vec2[1]));
			}
		}

		return true;
	}

	std::string uri::to_query_string()const
	{
		std::stringstream ss;
		for (auto itr = args_.begin(); itr != args_.end(); itr++)
		{
			ss << itr->first << "=" << url_encode(itr->second) << "&";
		}

		if (args_.size() > 0)
		{
			ss.seekp(-1, std::ios::end);
			ss.put(0);
		}

		return ss.str();
	}

	void uri::reset()
	{
		protocal_ = "";
		host_ = "";
		port_ = 0;
		is_default_port_ = false;
		path_ = "";
		args_.clear();
	}

	std::string uri::url_encode(const std::string& str)
	{
		std::string str_temp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (isalnum((unsigned char)str[i]) ||
				(str[i] == '-') ||
				(str[i] == '_') ||
				(str[i] == '.') ||
				(str[i] == '~'))
				str_temp += str[i];
			else if (str[i] == ' ')
				str_temp += "+";
			else
			{
				str_temp += '%';
				str_temp += to_hex((unsigned char)str[i] >> 4);
				str_temp += to_hex((unsigned char)str[i] % 16);
			}
		}
		return str_temp;
	}

	std::string uri::url_decode(const std::string& str)
	{
		std::string str_temp = "";
		size_t length = str.length();
		for (size_t i = 0; i < length; i++)
		{
			if (str[i] == '+') str_temp += ' ';
			else if (str[i] == '%')
			{
				unsigned char high = from_hex((unsigned char)str[++i]);
				unsigned char low = from_hex((unsigned char)str[++i]);
				str_temp += high * 16 + low;
			}
			else str_temp += str[i];
		}
		return str_temp;
	}






	unsigned char uri::to_hex(unsigned char x)
	{
		return  x > 9 ? x + 55 : x + 48;
	}

	unsigned char uri::from_hex(unsigned char x)
	{
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
		else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
		else if (x >= '0' && x <= '9') y = x - '0';
		else;
		return y;
	}

}
