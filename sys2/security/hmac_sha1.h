
/**
 * @file hmac_sha1.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#ifndef __HMAC_SHA1_H__
#define __HMAC_SHA1_H__

#include "sha1.h"
#include <string>

typedef unsigned char BYTE;

class hmac_sha1 : public sha1
{
private:
    BYTE m_ipad[64];
    BYTE m_opad[64];

    char* szReport;
    char* SHA1_Key;
    char* AppendBuf1;
    char* AppendBuf2;


public:

    enum {
        SHA1_DIGEST_LENGTH = 20,
        SHA1_BLOCK_SIZE = 64,
        HMAC_BUF_LEN = 4096
    };

    hmac_sha1()
        :szReport(new char[HMAC_BUF_LEN]),
        AppendBuf1(new char[HMAC_BUF_LEN]),
        AppendBuf2(new char[HMAC_BUF_LEN]),
        SHA1_Key(new char[HMAC_BUF_LEN])
    {}

    ~hmac_sha1()
    {
        delete[] szReport;
        delete[] AppendBuf1;
        delete[] AppendBuf2;
        delete[] SHA1_Key;
    }

    void make_bytes(const BYTE* text, int text_len, const BYTE* key, int key_len, BYTE* digest);
    std::string make_string(const std::string& text,const std::string& key);

    static std::string hex_string(BYTE* hex, int size);
};


#endif /* __HMAC_SHA1_H__ */