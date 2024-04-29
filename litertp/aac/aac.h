/**
 * @file aac.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include <stdint.h>

typedef enum _aac_encode_complexity_t
{
	aac_encode_complexity_main=1,
	aac_encode_complexity_low=2,
}aac_encode_complexity_t;

typedef struct _aac_specific_config_t
{
	aac_encode_complexity_t complexity;
	int samplerate;
	int channels;
}aac_specific_config_t;


void aac_specific_config_deserialize(aac_specific_config_t* cfg, uint16_t v);

void aac_specific_config_serialize(const aac_specific_config_t* cfg, uint16_t* b);


