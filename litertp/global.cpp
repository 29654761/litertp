/**
 * @file global.cpp
 * @brief Global resources for library.
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "global.h"
#include "log.h"

#include <sys2/socket.h>

namespace litertp
{
	global::global()
	{
	}

	global::~global()
	{
	}

	bool global::init()
	{
		int r=sys::socket::global_init();
		if (r != 0)
		{
			cleanup();
			return false;
		}

#ifdef LITERTP_SSL
		srtp_err_status_t srtp = srtp_init();
		if (srtp != srtp_err_status_ok)
		{
			cleanup();
			return false;
		}

		cert_ = std::make_shared<cert>();
		if (!cert_->create_cert())
		{
			cleanup();
			return false;
		}

		//cert_->export_cert("E:\\cacert.pem");
		//cert_->export_key("E:\\privkey.pem");
#endif
		return true;
	}

	void global::cleanup()
	{
		sys::socket::global_cleanup();

#ifdef LITERTP_SSL
		srtp_shutdown();
		if (cert_)
		{
			cert_.reset();
		}
#endif
	}

	cert_ptr global::get_cert()
	{
#ifdef LITERTP_SSL
		std::unique_lock<std::recursive_mutex> lk(cert_mutex_);
		if (!cert_||cert_->is_timeout())
		{
			cert_ = std::make_shared<cert>();
			if (!cert_->create_cert())
			{
				cert_.reset();
				return nullptr;
			}
		}
		return cert_;
#else
		return nullptr;
#endif
	}

	dtls_ptr global::get_dtls()
	{
#ifdef LITERTP_SSL
		cert_ptr cert = get_cert();
		if (!cert)
		{
			return nullptr;
		}
		dtls_ptr dtls = std::make_shared<litertp::dtls>(cert);
		if (!dtls->open())
		{
			return nullptr;
		}

		return dtls;
#else
		return nullptr;
#endif
	}

	global g_instance;
}
