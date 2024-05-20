#pragma once
#include <string>
#include <vector>

namespace sys {

	class uri
	{
	public:
		uri();
		~uri();

		void add(const std::string& name, const std::string& value);
		void remove(const std::string& name);
		void clear();

		const std::string& get(const std::string& name, const std::string& def_value = "")const;
		void get(const std::string& name, std::vector<std::string>& values)const;
		int64_t get_int64(const std::string& name, int64_t def_value = 0)const;
		uint64_t get_uint64(const std::string& name, uint64_t def_value = 0)const;
		double get_double(const std::string& name, double def_value = 0)const;

		bool parse(const std::string& s);
		std::string to_string()const;

		bool parse_query_string(const std::string& s);
		std::string to_query_string()const;

		static std::string url_encode(const std::string& str);
		static std::string url_decode(const std::string& str);

		void reset();

	private:
		static unsigned char to_hex(unsigned char x);
		static unsigned char from_hex(unsigned char x);
	public:
		std::string protocal_;
		std::string host_;
		int port_;
		bool is_default_port_;
		std::string path_;
		std::vector<std::pair<std::string, std::string>> args_;
	};

}
