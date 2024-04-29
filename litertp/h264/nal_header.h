/**
 * @file nal_header.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once
#include <string>
#include <vector>

typedef struct _fu_header_t
{
	uint8_t s : 1;
	uint8_t e : 1;
	uint8_t r : 1;
	uint8_t t : 5;
}fu_header_t;

typedef struct _nal_header_t
{
	uint8_t f : 1;
	uint8_t nri : 2;
	uint8_t t : 5;
}nal_header_t;



void fu_header_set(fu_header_t* fu, uint8_t v);

uint8_t fu_header_get(const fu_header_t* fu);

void nal_header_set(nal_header_t* nal, uint8_t v);

uint8_t nal_header_get(const nal_header_t* nal);
