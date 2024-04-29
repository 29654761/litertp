/**
 * @file util.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include <string>
#include <random>

namespace sys{

class util
{
public:

    static std::string uuid();

    template<class TNumber>
    static TNumber random_number(TNumber min,TNumber max)
    {
        static std::random_device rd;
        static std::uniform_int_distribution<TNumber> dist(min, max);
        return dist(rd);
    }

    
    static std::string random_string(int len);
};

}


