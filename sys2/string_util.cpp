/**
 * @file string_util.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "string_util.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <iconv.h>
#endif // _WIN32


namespace sys {


void string_util::split(const std::string& src, const std::string& pattern, std::vector<std::string>& result,int max,bool removeEmpty)
{
	std::string::size_type pos = 0;
	std::string::size_type size = src.size();

	if (max == 0||size==0||pattern.size()==0)
		return;

	
	while (pos <= size)
	{
		std::string::size_type end = src.find(pattern, pos);
		std::string s;

		if ((max >= 0&& result.size() >= max-1)||end==std::string::npos)
		{
			s = src.substr(pos, std::string::npos);
			pos = std::string::npos;
		}
		else
		{
			s = src.substr(pos, end-pos);
			pos = end;
		}
		

		if (removeEmpty) {
			if (!s.empty()) {
				result.push_back(s);
			}
		}
		else {
			result.push_back(s);
		}

		if (pos < size) {
			pos += pattern.size();
		}
	}
}

std::vector<std::string> string_util::split(const std::string& src, const std::string& pattern, int max, bool removeEmpty)
{
	std::vector<std::string> result;
	std::string::size_type pos = 0;
	std::string::size_type size = src.size();

	if (max == 0 || size == 0 || pattern.size() == 0)
		return result;


	while (pos <= size)
	{
		std::string::size_type end = src.find(pattern, pos);
		std::string s;

		if ((max >= 0 && result.size() >= max - 1) || end == std::string::npos)
		{
			s = src.substr(pos, std::string::npos);
			pos = std::string::npos;
		}
		else
		{
			s = src.substr(pos, end - pos);
			pos = end;
		}


		if (removeEmpty) {
			if (!s.empty()) {
				result.push_back(s);
			}
		}
		else {
			result.push_back(s);
		}

		if (pos < size) {
			pos += pattern.size();
		}
	}

	return result;
}


std::string string_util::gb2312_to_utf8(const std::string& gb2312)
{
	std::vector<wchar_t> buff(gb2312.size());
#ifdef _MSC_VER
	std::locale loc("zh-CN");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	wchar_t* pwsz_next = nullptr;
	const char* psz_next = nullptr;
	mbstate_t state = {};
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).in(state,
			gb2312.data(), gb2312.data() + gb2312.size(), psz_next,
			buff.data(), buff.data() + buff.size(), pwsz_next);

	if (std::codecvt_base::ok == res)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
		return cutf8.to_bytes(std::wstring(buff.data(), pwsz_next));
	}

	return "";

}

std::string string_util::utf8_to_gb2312(const std::string& utf8)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> cutf8;
	std::wstring wtemp = cutf8.from_bytes(utf8);
#ifdef _MSC_VER
	std::locale loc("zh_CN");
#else
	std::locale loc("zh_CN.GB18030");
#endif
	const wchar_t* pwsz_next = nullptr;
	char* psz_next = nullptr;
	mbstate_t state = {};

	std::vector<char> buff(wtemp.size() * 2);
	int res = std::use_facet<std::codecvt<wchar_t, char, mbstate_t> >
		(loc).out(state,
			wtemp.data(), wtemp.data() + wtemp.size(), pwsz_next,
			buff.data(), buff.data() + buff.size(), psz_next);

	if (std::codecvt_base::ok == res)
	{
		return std::string(buff.data(), psz_next);
	}
	return "";
}


std::string string_util::w2s(const std::wstring& wstr)
{
#ifdef _WIN32
	std::string res;
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		return res;
	}
	char* buffer = new char[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, nullptr, nullptr);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;

#else
	setlocale(LC_ALL, "en_US.UTF-8");
	const wchar_t* wchSrc = wstr.c_str();
	size_t nWStr = wcstombs(NULL, wchSrc, 0) + 1;
	char* chDest = new char[nWStr];
	memset(chDest, 0, nWStr);
	wcstombs(chDest, wchSrc, nWStr);
	std::string strRes = chDest;
	delete[] chDest;

	return strRes;

#endif
}

std::wstring string_util::s2w(const std::string& str)
{
#ifdef _WIN32
	std::wstring res;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), nullptr, 0);
	if (len < 0) {
		return res;
	}
	wchar_t* buffer = new wchar_t[len + 1];
	if (buffer == nullptr) {
		return res;
	}
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
	buffer[len] = '\0';
	res.append(buffer);
	delete[] buffer;
	return res;
#else
	std::string strUtf8 = str;
	setlocale(LC_ALL, "en_US.UTF-8");
	const char* chSrc = strUtf8.c_str();
	size_t nStr = mbstowcs(NULL, chSrc, 0) + 1;
	wchar_t* wchDest = new wchar_t[nStr];
	memset(wchDest, 0, nStr);
	mbstowcs(wchDest, chSrc, nStr);
	std::wstring wStrRes = wchDest;
	delete[] wchDest;
	return wStrRes;

#endif
}

char* string_util::alloc(const std::string& s) {
	char* ret = nullptr;
	size_t len = s.size() + 1;
	if (len <= 0) {
		return ret;
	}
	ret = (char*)malloc(len);
	memset(ret, 0, len);
	s.copy(ret, len);
	return ret;
}

void string_util::free(char** p) {
	if (p && (*p)) {
		::free(*p);
		*p = NULL;
	}
}

std::string string_util::join_path(std::string& path1, std::string& path2, const std::string& separator)
{
	auto itr = path1.rbegin();
	if (itr != path1.rend()) {
		if (*itr == '/' || *itr == '\\') {
			path1.erase((itr + 1).base());
		}
	}
	else {
		path1 = path2;
		return path1;
	}

	auto itr2 = path2.begin();
	if (itr2 != path2.end()) {
		if (*itr2 == '/' || *itr2 == '\\') {
			path2.erase(itr2);
		}
	}

	return path1.append(separator).append(path2);
}

// trim from start (in place)
void string_util::ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
void string_util::rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
void string_util::trim(std::string& s) {
	rtrim(s);
	ltrim(s);
}

bool string_util::icasecompare(const std::string& a, const std::string& b)
{
	return std::equal(a.begin(), a.end(), b.begin(), b.end(),
		[](char c1, char c2) {
				return std::tolower(c1) == std::tolower(c2);
			}
	);
}

}
