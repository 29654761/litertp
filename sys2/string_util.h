
/**
 * @file string_util.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once
#include <string>
#include <vector>
#include <codecvt>
#include <locale>

namespace sys {

class string_util
{
public:
	static void split(const std::string& src, const std::string& pattern, std::vector<std::string>& result, int max=-1, bool removeEmpty=true);
	static std::vector<std::string> split(const std::string& src, const std::string& pattern, int max=-1, bool removeEmpty=true);
	//Methods for utf8 are unsported on Adnroid NDK
	static std::string gb2312_to_utf8(const std::string& gb2312);
	static std::string utf8_to_gb2312(const std::string& utf8);

	static std::string w2s(const std::wstring& w);
	static std::wstring s2w(const std::string& s);

	static char* alloc(const std::string& s);
	static void free(char** p);

	static std::string join_path(std::string& path1, std::string& path2, const std::string& separator);

	static void ltrim(std::string& s);
	static void rtrim(std::string& s);
	static void trim(std::string& s);

	static bool icasecompare(const std::string& a, const std::string& b);
};

}

