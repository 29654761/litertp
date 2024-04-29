/**
 * @file dtls.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */

#pragma once


#include "cert.h"

#include <string>
#include <mutex>

#ifdef LITERTP_SSL
#include <openssl/ssl3.h>
#include <srtp2/srtp.h>
#endif

#define PROFILE_STRING_LENGTH 23
#define SRTP_MASTER_KEY_LENGTH 16
#define SRTP_MASTER_SALT_LENGTH 14
#define SRTP_MASTER_LENGTH (SRTP_MASTER_KEY_LENGTH + SRTP_MASTER_SALT_LENGTH)

namespace litertp
{


	typedef struct dtls_info_t {
		unsigned char client_key[SRTP_MASTER_KEY_LENGTH + SRTP_MASTER_SALT_LENGTH];
		unsigned char server_key[SRTP_MASTER_KEY_LENGTH + SRTP_MASTER_SALT_LENGTH];
		//unsigned char profile[PROFILE_STRING_LENGTH];
		unsigned long profile_id;
		int key_length;
	} dtls_info_t;


	class dtls
	{
	public:
		dtls(cert_ptr cert);
		~dtls();

		bool open();
		void close();

		std::string write(const uint8_t* data, size_t size);
		std::string read(const uint8_t* data, size_t size);
		std::string flush();

		std::string accept(const uint8_t* data,size_t size);
		std::string connect();

		std::string fingerprint();
		bool export_key_material(dtls_info_t* info);
		bool is_init_finished();
		bool is_established();

	private:
		std::recursive_mutex mutex_;

#ifdef LITERTP_SSL
		SSL_CTX* ctx_ = nullptr;
		SSL* ssl_ = nullptr;
#endif
		cert_ptr cert_;
		bool established_ = false;
	};

	typedef std::shared_ptr<dtls> dtls_ptr;
}

