/**
 * @file cert.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "cert.h"



#include "../util/time.h"
#include <assert.h>

#define ONE_YEAR 60*60*24*365
#define RENEW_ONE_YEAR 60*60*24*360

namespace litertp
{
	cert::cert()
	{
	}

	cert::~cert()
	{
		destroy_cert();
	}


	bool cert::create_cert()
	{
#ifdef LITERTP_SSL
		static const int num_bits = 2048;
		BIGNUM* bne = BN_new();
		BN_set_word(bne, RSA_F4);
		RSA* rsa_key = RSA_new();
		RSA_generate_key_ex(rsa_key, num_bits, bne, NULL);

		key_ = EVP_PKEY_new();
		EVP_PKEY_assign_RSA(key_, rsa_key);
		cert_ = X509_new();
		X509_set_version(cert_, 2);

		ASN1_INTEGER_set(X509_get_serialNumber(cert_), 1000); // TODO
		X509_gmtime_adj(X509_get_notBefore(cert_), -1 * ONE_YEAR);
		X509_gmtime_adj(X509_get_notAfter(cert_), ONE_YEAR);
		if (!X509_set_pubkey(cert_, key_)) {
			destroy_cert();
			return false;
		}

		X509_NAME* cert_name = X509_get_subject_name(cert_);

		const char* name = "litertp";
		X509_NAME_add_entry_by_txt(cert_name, "O", MBSTRING_ASC, (const char unsigned*)name, -1, -1, 0);
		X509_NAME_add_entry_by_txt(cert_name, "CN", MBSTRING_ASC, (const char unsigned*)name, -1, -1, 0);

		if (!X509_set_issuer_name(cert_, cert_name)) {
			destroy_cert();
			return false;
		}

		if (!X509_sign(cert_, key_, EVP_sha1())) {
			destroy_cert();
			return false;
		}

		BN_free(bne);

		expire_at_= time_util::cur_time();
		return true;
#else
		return false;
#endif
	}

	void cert::destroy_cert()
	{
#ifdef LITERTP_SSL
		if (cert_)
		{
			X509_free(cert_);
			cert_ = nullptr;
		}

		if (key_) {
			EVP_PKEY_free(key_);
			key_ = nullptr;
		}
#else
		expire_at_ = 0;
#endif
	}

	std::string cert::fingerprint()const
	{
#ifdef LITERTP_SSL
		assert(cert_);
		std::string ret;
		ret.reserve(160);
		unsigned int size;
		unsigned char fingerprint[EVP_MAX_MD_SIZE];
		if (X509_digest(cert_, EVP_sha256(), (unsigned char*)fingerprint, &size) == 0) {
			return ret;
		}

		for (unsigned int i = 0; i < size; i++) {
			char c[5] = { 0 };
			sprintf(c, "%.2X:", fingerprint[i]);
			ret.append(c);
		}
		if (size > 0) {
			ret.erase(ret.end() - 1);
		}
		return ret;
#else
		return "";
#endif
	}

	bool cert::is_timeout()const
	{
		long now = time_util::cur_time();
		if (now >= expire_at_)
		{
			return false;
		}
		return true;
	}
}
