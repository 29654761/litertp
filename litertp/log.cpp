/**
 * @file log.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <atomic>

static std::atomic<log_level_t> s_log_level = log_level_debug;

LITERTP_API void LITERTP_CALL litertp_log_set_level(log_level_t level)
{
	s_log_level = level;
}

LITERTP_API void LITERTP_CALL litertp_log_print(log_level_t level, const char* format, ...)
{
	if (level >= s_log_level)
	{
		va_list argptr;
		va_start(argptr, format);
		vfprintf(stdout, format, argptr);
		va_end(argptr);
	}
}
