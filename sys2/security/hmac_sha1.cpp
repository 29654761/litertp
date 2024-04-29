/**
 * @file hmac_sha1.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "hmac_sha1.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <iomanip>

void hmac_sha1::make_bytes(const BYTE* text, int text_len, const BYTE* key, int key_len, BYTE* digest)
{
    memset(SHA1_Key, 0, SHA1_BLOCK_SIZE);

    /* repeated 64 times for values in ipad and opad */
    memset(m_ipad, 0x36, sizeof(m_ipad));
    memset(m_opad, 0x5c, sizeof(m_opad));

    /* STEP 1 */
    if (key_len > SHA1_BLOCK_SIZE)  //´óÓÚ64Î»
    {
        sha1::reset();
        sha1::update((UINT_8*)key, key_len);
        sha1::final();

        sha1::get_hash((UINT_8*)SHA1_Key);//20
    }
    else
        memcpy(SHA1_Key, key, key_len);

    /* STEP 2 */
    for (int i = 0; i < sizeof(m_ipad); i++)
    {
        m_ipad[i] ^= SHA1_Key[i];
    }

    /* STEP 3 */
    memcpy(AppendBuf1, m_ipad, sizeof(m_ipad));
    memcpy(AppendBuf1 + sizeof(m_ipad), text, text_len);

    /* STEP 4 */
    sha1::reset();
    sha1::update((UINT_8*)AppendBuf1, sizeof(m_ipad) + text_len);
    sha1::final();

    sha1::get_hash((UINT_8*)szReport);

    /* STEP 5 */
    for (int j = 0; j < sizeof(m_opad); j++)
    {
        m_opad[j] ^= SHA1_Key[j];
    }

    /* STEP 6 */
    memcpy(AppendBuf2, m_opad, sizeof(m_opad));
    memcpy(AppendBuf2 + sizeof(m_opad), szReport, SHA1_DIGEST_LENGTH);

    /*STEP 7 */
    sha1::reset();
    sha1::update((UINT_8*)AppendBuf2, sizeof(m_opad) + SHA1_DIGEST_LENGTH);
    sha1::final();

    sha1::get_hash((UINT_8*)digest);


    //    char * mu;
    //    CSHA1::ReportHash(mu,REPORT_HEX);

}

std::string hmac_sha1::make_string(const std::string& text, const std::string& key)
{
    BYTE ret[20] = { 0 };
    make_bytes((const BYTE*)text.c_str(), (int)text.size(),(const BYTE*)key.c_str(), (int)key.size(), ret);
    return hex_string(ret, 20);
}

std::string hmac_sha1::hex_string(BYTE* hex, int size)
{
    std::stringstream ss;
    for (int i = 0; i < size; i++) {
        ss << std::hex <<std::nouppercase<<std::setfill('0')<<std::setw(2)<<(int)hex[i];
    }
    return ss.str();
}