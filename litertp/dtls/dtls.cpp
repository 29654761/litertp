/**
 * @file dtls.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "dtls.h"

#include <algorithm>
#include <string.h>
#include <assert.h>

#define ONE_YEAR 60*60*24*365

namespace litertp
{
	dtls::dtls(cert_ptr cert)
	{
		cert_ = cert;
	}

	dtls::~dtls()
	{
		close();
	}

#ifdef LITERTP_SSL
	static int dtls_verify_callback(int preverify_ok, X509_STORE_CTX* ctx) {
		/* We just use the verify_callback to request a certificate from the client */
		return 1;
	}
#endif

	bool dtls::open()
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);

#ifdef LITERTP_SSL
		SSL_load_error_strings();
		if(!SSL_library_init()){
			close();
			return false;
		}

		ctx_ = SSL_CTX_new(DTLS_method());
		if(!ctx_) {
			close();
			return false;
		}
		SSL_CTX_set_verify(ctx_, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, dtls_verify_callback);
		SSL_CTX_set_tlsext_use_srtp(ctx_, "SRTP_AES128_CM_SHA1_32:SRTP_AES128_CM_SHA1_80");


		SSL_CTX_use_certificate(ctx_, cert_->handle());
		SSL_CTX_use_PrivateKey(ctx_, cert_->key());
		if (!SSL_CTX_check_private_key(ctx_))
		{
			close();
			return false;
		}

		SSL_CTX_set_read_ahead(ctx_, 1);
		SSL_CTX_set_ecdh_auto(ctx_, true);


		SSL_CTX_set_cipher_list(ctx_, "HIGH:!aNULL:!MD5:!RC4");
		

		ssl_ = SSL_new(ctx_);


		//SSL_set_ex_data(ssl_, 0, dtls);
		//SSL_set_info_callback(ssl_, janus_dtls_callback);


		BIO* rbio = BIO_new(BIO_s_mem());
		BIO* wbio = BIO_new(BIO_s_mem());
		BIO_set_mem_eof_return(rbio, -1);
		BIO_set_mem_eof_return(wbio, -1);
		SSL_set_bio(ssl_, rbio, wbio);


		EC_KEY* ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
		if (ecdh)
		{
			SSL_set_options(ssl_, SSL_OP_SINGLE_ECDH_USE);
			SSL_set_tmp_ecdh(ssl_, ecdh);
			EC_KEY_free(ecdh);
		}
		established_ = false;
		return true;
#else
		return false;
#endif
	}

	void dtls::close()
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);

#ifdef LITERTP_SSL
		if (ssl_) {
			SSL_shutdown(ssl_);
			SSL_free(ssl_);
			ssl_ = nullptr;
		}

		if (ctx_) {
			SSL_CTX_free(ctx_);
			ctx_ = nullptr;
		}
#endif
		established_ = false;
	}



	std::string dtls::write(const uint8_t* data, size_t size)
	{
#ifdef LITERTP_SSL
		std::unique_lock<std::recursive_mutex> lk(mutex_);
		assert(ssl_);
		BIO* wbio = SSL_get_wbio(ssl_);
		assert(wbio);

		SSL_write(ssl_, data, size);

		size_t buf_size = BIO_ctrl_pending(wbio);
		std::string ret;
		ret.reserve(buf_size);
		char* buf = (char*)calloc(1, buf_size);
		if (buf)
		{
			BIO_read(wbio, buf, buf_size);
			ret.assign(buf, buf_size);
			free(buf);
		}
		return ret;
#else
		return "";
#endif
	}

	std::string dtls::read(const uint8_t* data, size_t size)
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);
#ifdef LITERTP_SSL
		assert(ssl_);
		BIO* rbio = SSL_get_rbio(ssl_);
		assert(rbio);

		BIO_write(rbio, data, size);
		std::string ret;
		ret.reserve(size);
		char* buf = (char*)calloc(1, size);
		if (buf)
		{
			int r = SSL_read(ssl_, buf, size);
			if (r < 0 && SSL_get_error(ssl_,r) == SSL_ERROR_SSL) {
				free(buf);
				return ret;
			}
			ret.assign(buf, size);
			free(buf);
		}
		return ret;
#else
		return "";
#endif
	}

	std::string dtls::accept(const uint8_t* data, size_t size)
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);

#ifdef LITERTP_SSL
		assert(ssl_);
		BIO* rbio = SSL_get_rbio(ssl_);
		assert(rbio);
		BIO_write(rbio, data, size);

		std::string ret;
		if (!SSL_accept(ssl_)) {
			return ret;
		}

		BIO* wbio = SSL_get_wbio(ssl_);
		assert(wbio);

		size_t rsp_size = BIO_ctrl_pending(wbio);
		char* rsp = (char*)calloc(1, rsp_size);
		if (!rsp) {
			return ret;
		}
		BIO_read(wbio, rsp, rsp_size);
		ret.assign(rsp, rsp_size);
		free(rsp);

		established_ = true;
		return ret;
#else
		return "";
#endif
	}

	std::string dtls::connect()
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);
#ifdef LITERTP_SSL
		assert(ssl_);


		SSL_connect(ssl_);
		
		std::string ret;
		BIO* wbio = SSL_get_wbio(ssl_);
		assert(wbio);
		size_t rsp_size = BIO_ctrl_pending(wbio);
		char* rsp = (char*)calloc(1, rsp_size);
		if (!rsp) {
			return ret;
		}
		BIO_read(wbio, rsp, rsp_size);
		ret.assign(rsp, rsp_size);
		free(rsp);

		established_ = true;
		return ret;
#else
		return "";
#endif
	}


	std::string dtls::fingerprint()
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);
		return cert_->fingerprint();
	}

	bool dtls::is_init_finished() {
		std::unique_lock<std::recursive_mutex> lk(mutex_);
#ifdef LITERTP_SSL
		return SSL_is_init_finished(ssl_);
#else
		return false;
#endif
	}
	bool dtls::is_established() {
		std::unique_lock<std::recursive_mutex> lk(mutex_);
#ifdef LITERTP_SSL
		return established_;
#else
		return false;
#endif
	}

	bool dtls::export_key_material(dtls_info_t* info)
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);

#ifdef LITERTP_SSL
		const char* label = "EXTRACTOR-dtls_srtp";
		unsigned char material[SRTP_MASTER_LENGTH *2] = { 0 };
		if (!SSL_export_keying_material(ssl_, material, sizeof(material), label, strlen(label), NULL, 0, 0))
		{
			return false;
		}

		info->key_length = SRTP_MASTER_LENGTH;
		int offset = 0;
		memcpy(info->client_key, material+ offset, SRTP_MASTER_KEY_LENGTH);
		offset += SRTP_MASTER_KEY_LENGTH;
		memcpy(info->server_key, material +offset, SRTP_MASTER_KEY_LENGTH);
		offset += SRTP_MASTER_KEY_LENGTH;
		memcpy(info->client_key+SRTP_MASTER_KEY_LENGTH, material +offset, SRTP_MASTER_SALT_LENGTH);
		offset += SRTP_MASTER_SALT_LENGTH;
		memcpy(info->server_key+SRTP_MASTER_KEY_LENGTH, material +offset, SRTP_MASTER_SALT_LENGTH);

		auto profile=SSL_get_selected_srtp_profile(ssl_);
		if (!profile) {
			return false;
		}

		/*
		switch (profile->id) {
		case SRTP_AES128_CM_SHA1_80:
			memcpy(&info->profile, "SRTP_AES128_CM_SHA1_80", strlen("SRTP_AES128_CM_SHA1_80"));
			break;
		case SRTP_AES128_CM_SHA1_32:
			memcpy(&info->profile, "SRTP_AES128_CM_SHA1_32", strlen("SRTP_AES128_CM_SHA1_32"));
			break;
		}
		*/
		info->profile_id = profile->id;
#else
		return false;
#endif
		return true;
	}

	std::string dtls::flush()
	{
		std::unique_lock<std::recursive_mutex> lk(mutex_);

#ifdef LITERTP_SSL
		assert(ssl_);
		BIO* wbio = SSL_get_wbio(ssl_);
		assert(wbio);

		std::string ret;
		int pending = BIO_ctrl_pending(wbio);
		while (pending>0)
		{
			char* buf=(char*)calloc(1,pending);
			if (buf)
			{
				int out = BIO_read(wbio, buf, pending);
				if (out > 0)
				{
					ret.append(buf, out);
				}
				free(buf);
			}
			pending = BIO_ctrl_pending(wbio);
		}
		return ret;
#else
		return "";
#endif
	}
}
