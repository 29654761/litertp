
/**
 * @file candidate.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "candidate.h"
#include <sys2/string_util.h>

namespace litertp
{
	candidate::candidate()
	{

	}

	candidate::~candidate()
	{

	}

	bool candidate::parse(const std::string& s)
	{
		std::vector<std::string> ss = sys::string_util::split(s, " ");
		if (ss.size() < 7)
		{
			return false;
		}
		char* endptr = nullptr;
		foundation_ = strtoul(ss[0].c_str(),&endptr,0);
		component_ = strtoul(ss[1].c_str(),&endptr,0);
		protocol_ = ss[2];
		address_ = ss[3];
		port_ = atoi(ss[4].c_str());
		type_ = ss[5];
		return true;
	}

	std::string candidate::to_string() const
	{
		std::string s;
		s += std::to_string(foundation_);
		s += " ";
		s += std::to_string(component_);
		s += " ";
		s += protocol_;
		s += " ";
		s += std::to_string(priority_);
		s += " ";
		s += address_;
		s += " ";
		s += std::to_string(port_);
		s += " ";
		s += "typ " + type_;
		s += " ";
		s += "generation 0";
		s += " ";
		s += "network-id 1";
		s += " ";
		s += "network-cost 10";
		return s;
	}

}