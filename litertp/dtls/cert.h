/**
 * @file cert.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once

#ifdef LITERTP_SSL
#include <openssl/ssl3.h>
#endif


#include <string>
#include <memory>

namespace litertp
{
	class cert
	{
	public:
		cert();
		~cert();

		bool create_cert();
		void destroy_cert();

		std::string fingerprint()const;
		bool is_timeout()const;
#ifdef LITERTP_SSL
		X509* handle() { return cert_; }
		EVP_PKEY* key() { return key_; }
#endif

		bool export_cert(const std::string& file);
		bool export_key(const std::string& file);
	private:
		long expire_at_ = 0;

#ifdef LITERTP_SSL
		X509* cert_ = nullptr;
		EVP_PKEY* key_ = nullptr;
#endif
	};

	typedef std::shared_ptr<cert> cert_ptr;
}




