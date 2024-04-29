/**
 * @file aac.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "aac.h"




void aac_specific_config_deserialize(aac_specific_config_t* cfg, uint16_t v)
{
    cfg->complexity = (_aac_encode_complexity_t)(uint16_t)((v & 0xF800) >> 11);

    int samplerate = (uint16_t)((v & 0x0780) >> 7);  //4bits samplerate 0-96000, 1-88200, 2-64000, 3-48000,4-44100,5-32000,6-24000,7-22050,8-16000

    if (samplerate == 0)
        cfg->samplerate = 96000;
    else if (samplerate == 1)
        cfg->samplerate = 88200;
    else if (samplerate == 2)
        cfg->samplerate = 64000;
    else if (samplerate == 3)
        cfg->samplerate = 48000;
    else if (samplerate == 4)
        cfg->samplerate = 44100;
    else if (samplerate == 5)
        cfg->samplerate = 32000;
    else if (samplerate == 6)
        cfg->samplerate = 24000;
    else if (samplerate == 7)
        cfg->samplerate = 22050;
    else if (samplerate == 8)
        cfg->samplerate = 16000;
    else if (samplerate == 9)
        cfg->samplerate = 8000;

    cfg->channels = (uint16_t)((v & 0x078) >> 3);     //4bits channels

    //3bits fixed 0
}

void aac_specific_config_serialize(const aac_specific_config_t* cfg, uint16_t* b)
{
    uint16_t v = 0;
    uint16_t complexity = (uint16_t)cfg->complexity;
    v |= (uint16_t)(complexity << 11);

    uint16_t samplerate = 0;
    if (cfg->samplerate == 96000)
        samplerate = 0;
    else if (cfg->samplerate == 88200)
        samplerate = 1;
    else if (cfg->samplerate == 64000)
        samplerate = 2;
    else if (cfg->samplerate == 48000)
        samplerate = 3;
    else if (cfg->samplerate == 44100)
        samplerate = 4;
    else if (cfg->samplerate == 32000)
        samplerate = 5;
    else if (cfg->samplerate == 24000)
        samplerate = 6;
    else if (cfg->samplerate == 22050)
        samplerate = 7;
    else if (cfg->samplerate == 16000)
        samplerate = 8;
    else if (cfg->samplerate == 8000)
        samplerate = 9;

    v |= (uint16_t)(samplerate << 7);
    v |= (uint16_t)(cfg->channels << 3);

    *b = v;

}