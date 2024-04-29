/**
 * @file log.h
 * @brief 
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#include "litertp_def.h"

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

	typedef enum log_level_t
	{
		log_level_trace,
		log_level_debug,
		log_level_info,
		log_level_warn,
		log_level_error,
		log_level_fatal,
	}log_level_t;

	LITERTP_API void LITERTP_CALL litertp_log_set_level(log_level_t level);
	LITERTP_API void LITERTP_CALL litertp_log_print(log_level_t level, const char* format, ...);


#define LOGT(...) litertp_log_print(log_level_trace,__VA_ARGS__)
#define LOGD(...) litertp_log_print(log_level_debug,__VA_ARGS__)
#define LOGI(...) litertp_log_print(log_level_info,__VA_ARGS__)
#define LOGW(...) litertp_log_print(log_level_warn,__VA_ARGS__)
#define LOGE(...) litertp_log_print(log_level_error,__VA_ARGS__)
#define LOGF(...) litertp_log_print(log_level_fatal,__VA_ARGS__)



#ifdef __cplusplus
}
#endif //__cplusplus
