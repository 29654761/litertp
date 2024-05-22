/**
 * @file sdp.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "sdp.h"

#include <sstream>
#include "sys2/string_util.h"


namespace litertp {




	std::string sdp::to_string()const
	{
		std::stringstream ss;
		ss << "v=" << v_ << std::endl;
		ss << "o=" << o_ << std::endl;
		ss << "s=" << s_ << std::endl;
		if (i_.size() > 0) {
			ss << "i=" << i_ << std::endl;
		}
		if (u_.size() > 0) {
			ss << "u=" << u_ << std::endl;
		}
		if (e_.size() > 0) {
			ss << "e=" << e_ << std::endl;
		}
		if (p_.size() > 0) {
			ss << "p=" << p_ << std::endl;
		}
		if (c_.size() > 0) {
			ss << "c=" << c_ << std::endl;
		}
		if (b_.size() > 0) {
			ss << "b=" << b_ << std::endl;
		}
		if (z_.size() > 0) {
			ss << "z=" << z_ << std::endl;
		}
		if (k_.size() > 0) {
			ss << "k=" << k_ << std::endl;
		}
		if (t_.size() > 0) {
			ss << "t=" << t_ << std::endl;
		}
		if (r_.size() > 0) {
			ss << "r=" << r_ << std::endl;
		}

		for (auto& attr : attrs_)
		{
			ss << "a=" << attr.to_string() << std::endl;
		}

		if (bundle_)
		{
			ss << "a=group:BUNDLE";
			for (auto m : medias_)
			{
				ss << " " << m.mid_;
			}
			ss << std::endl;
		}

		for (auto a : attrs_)
		{
			ss << "a=" << a.to_string() << std::endl;
		}

		for (auto m : medias_)
		{
			m.to_string(ss);
		}

		return ss.str();
	}

	bool sdp::parse(const std::string& s)
	{
		std::vector<std::string> lines;
		sys::string_util::split(s, "\n", lines);

		attrs_.clear();
		medias_.clear();
		c_.clear();
		
		for (auto line : lines)
		{
			std::vector<std::string> vec;
			sys::string_util::split(line, "=", vec, 2);
			if (vec.size() < 2)
				continue;
			sys::string_util::trim(vec[1]);
			const std::string& key = vec[0];
			const std::string& val = vec[1];

			if (key == "v")
			{
				v_ = val;
			}
			else if (key == "o")
			{
				o_ = val;
			}
			else if (key == "s")
			{
				s_ = val;
			}
			else if (key == "t")
			{
				t_ = val;
			}
			else if (key == "m")
			{
				sdp_media m;
				m.parse_header(val);
				medias_.push_back(m);
			}
			else if(key=="a")
			{
				auto iter = this->medias_.rbegin();
				if (iter != medias_.rend())
				{
					iter->parse_a(val);
				}
				else
				{
					sdp_pair attr(":");
					if (attr.parse(val)) 
					{
						if (attr.key_ == "group")
						{
							std::vector<std::string> vec=sys::string_util::split(attr.val_, " ");
							if (vec.size() >= 2) 
							{
								if (vec[0] == "BUNDLE")
								{
									bundle_ = true;
								}
							}
						}

						this->attrs_.push_back(attr);
					}
				}
			}
			else if (key == "c")
			{
				auto iter = this->medias_.rbegin();
				if (iter != medias_.rend())
				{
					iter->parse_c(val);
				}
			}


		}

		return true;
	}


}
