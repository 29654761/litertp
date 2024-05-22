/**
 * @file util.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "util.h"
#include <iomanip>
#include <random>
#include <sstream>
#include <string.h>

namespace sys {

    std::string util::uuid()
    {
        static std::random_device rd;
        static std::uniform_int_distribution<uint64_t> dist(0ULL, 0xFFFFFFFFFFFFFFFFULL);
        uint64_t ab = dist(rd);
        uint64_t cd = dist(rd);
        uint32_t a, b, c, d;
        std::stringstream ss;
        ab = (ab & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
        cd = (cd & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
        a = (ab >> 32U);
        b = (ab & 0xFFFFFFFFU);
        c = (cd >> 32U);
        d = (cd & 0xFFFFFFFFU);
        ss << std::hex << std::nouppercase << std::setfill('0');
        ss << std::setw(8) << (a);//<< '-';
        ss << std::setw(4) << (b >> 16U);//<< '-';
        ss << std::setw(4) << (b & 0xFFFFU);// << '-';
        ss << std::setw(4) << (c >> 16U);//<< '-';
        ss << std::setw(4) << (c & 0xFFFFU);
        ss << std::setw(8) << d;

        return ss.str();
    }

    std::string util::random_string(int len)
    {
        const char* c = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        size_t cs = strlen(c);
        std::string s;
        s.reserve(len);
        for (int i = 0; i < len; i++)
        {
            int r=random_number<int>(0, cs-1);
            s.append(1,c[r]);
        }
        return s;
    }
}
