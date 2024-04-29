/**
 * @file nal_header.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include <string>
#include <vector>
#include "nal_header.h"

void fu_header_set(fu_header_t* fu, uint8_t v)
{
	fu->s = (v & 0x80) >> 7;
	fu->e = (v & 0x40) >> 6;
	fu->r = (v & 0x20) >> 5;
	fu->t = v & 0x1f;
}

uint8_t fu_header_get(const fu_header_t* fu)
{
	uint8_t v = 0;
	v |= fu->s << 7;
	v |= fu->e << 6;
	v |= fu->r << 5;
	v |= fu->t;
	return v;
}

void nal_header_set(nal_header_t* nal, uint8_t v)
{
	nal->f = (v & 0x80) >> 7;
	nal->nri = (v & 0x60) >> 5;
	nal->t = v & 0x1f;
}

uint8_t nal_header_get(const nal_header_t* nal)
{
	uint8_t v = 0;
	v |= nal->f << 7;
	v |= nal->nri << 5;
	v |= nal->t;
	return v;
}

