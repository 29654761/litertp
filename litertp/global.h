/**
 * @file global.h
 * @brief Global resources for library.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include "dtls/dtls.h"

#include <mutex>

namespace litertp
{
	class global
	{
	public:
		global();
		~global();

		bool init();
		void cleanup();

		cert_ptr get_cert();
		dtls_ptr get_dtls();

	private:
#ifdef LITERTP_SSL
		std::recursive_mutex cert_mutex_;
		cert_ptr cert_;
#endif
	};

	extern global g_instance;

}