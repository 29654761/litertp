/**
 * @file sn_test.hpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <stdint.h>
#include <stdio.h>

#include "sn.hpp"



void test_sn()
{
	bool b = rtp::sn::ahead_of<uint16_t>(511, 1);
}