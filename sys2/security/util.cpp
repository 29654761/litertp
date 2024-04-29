/**
 * @file util.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "util.h"


int sasl_prep(uint8_t* s) {
    if (s) {
        uint8_t* strin = s;
        uint8_t* strout = s;
        for (;;) {
            uint8_t c = *strin;
            if (!c) {
                *strout = 0;
                break;
            }

            switch (c) {
            case 0xAD:
                ++strin;
                break;
            case 0xA0:
            case 0x20:
                *strout = 0x20;
                ++strout;
                ++strin;
                break;
            case 0x7F:
                return -1;
            default:
                if (c < 0x1F)
                    return -1;
                if (c >= 0x80 && c <= 0x9F)
                    return -1;
                *strout = c;
                ++strout;
                ++strin;
            };
        }
    }

    return 0;
}

