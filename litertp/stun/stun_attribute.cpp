/**
 * @file stun_attribute.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "stun_attribute.h"
#include <sys2/socket.h>
#include <sys2/security/md5.h>
#include <sys2/security/hmac_sha1.h>
#include <sys2/security/util.h>

stun_attribute::stun_attribute()
{
	type_ = stun_attribute_type::stun_attribute_type_error_code;
}

stun_attribute::stun_attribute(stun_attribute_type type, const std::string& value)
{
	type_ = type;
	value_ = value;
}

stun_attribute::stun_attribute(stun_attribute_type type, const char* value, size_t size)
{
	type_ = type;
	value_.assign(value, size);
}


std::string stun_attribute::serialize()const
{
	std::string s;
	{
		uint16_t v = sys::socket::hton16(type_);
		s.append((const char*)&v, 2);
	}
	{
		uint16_t v = sys::socket::hton16((uint16_t)value_.size());
		s.append((const char*)&v, 2);
	}
	s.append(value_);

	int remainder = value_.size() % 4;
	if (remainder > 0) {
		s.append(4 - remainder, 0);
	}

	return s;
}

int stun_attribute::deserialize(const char* buffer, int size)
{
	if (size < 4) {
		return -1;
	}

	memcpy(&type_, buffer, 2);
	type_ = (stun_attribute_type)sys::socket::ntoh16(type_);

	uint16_t len = 0;
	memcpy(&len, buffer + 2, 2);
	len = sys::socket::ntoh16(len);

	int padding = 0;
	int remained = len % 4;
	if (remained != 0) {
		padding = (4 - remained);
	}

	if (size < len + 4+padding) {
		return -1;
	}

	value_.append(buffer + 4, len);
	return len + 4 + padding;
}














std::string stun_attribute_error_code::serialize()const
{
	std::string s;
	s.append(2, 0);
	uint8_t v = class_;
	s.append(1, v);
	s.append(1, number_);
	s.append(reason_);

	int remainder = reason_.size() % 4;
	if (remainder > 0) {
		s.append(4 - remainder, 0);
	}

	return s;
}

bool stun_attribute_error_code::deserialize(const char* buffer, int size)
{
	if (size < 4) {
		return false;
	}

	uint8_t v = buffer[2];
	class_ = v;
	number_ = (stun_error_code)buffer[3];
	reason_.append(buffer, size - 4);
	return true;
}
















std::string stun_attribute_mapped_address::serialize()const
{
	std::string s;
	s.append(1, 0);
	s.append(1, (char)m_family);

	{
		uint16_t v = sys::socket::hton16(m_port);
		s.append((const char*)&v, 2);
	}

	if (m_family == IPV4) {
		s.append((const char*)m_address, 4);
	}
	else {
		s.append((const char*)m_address, 16);
	}
	return s;
}



bool stun_attribute_mapped_address::deserialize(const char* buffer, int size)
{
	if (size < 4) {
		return false;
	}

	m_family = buffer[1];

	memcpy(&m_port, buffer + 2, 2);
	m_port = sys::socket::ntoh16(m_port);

	memset(m_address, 0, 16);
	if (m_family == IPV4) {
		if (size < 8) {
			return false;
		}
		memcpy(m_address, buffer + 4, 4);
	}
	else if (m_family == IPV6) {
		if (size < 20) {
			return false;
		}
		memcpy(m_address, buffer + 4, 16);
	}
	else {
		return false;
	}

	return true;
}

//0x2112A442
stun_attribute_xor_mapped_address stun_attribute_xor_mapped_address::create(const sockaddr* addr, const char* tid)
{
	stun_attribute_xor_mapped_address attr;

	if (addr->sa_family == AF_INET) {
		attr.m_family = stun_attribute_xor_mapped_address::IPV4;
		const sockaddr_in* sin = (const sockaddr_in*)addr;

		attr.m_port = sys::socket::ntoh16(sin->sin_port);
		attr.m_port ^= 0x2112;


		attr.m_address[0] = ((sin->sin_addr.s_addr & 0xFF000000) >> 24) ^ 0x21;
		attr.m_address[1] = ((sin->sin_addr.s_addr & 0x00FF0000) >> 16) ^ 0x12;
		attr.m_address[2] = ((sin->sin_addr.s_addr & 0x0000FF00) >> 8) ^ 0xA4;
		attr.m_address[3] = ((sin->sin_addr.s_addr & 0x000000FF)) ^ 0x42;

	}
	else if (addr->sa_family == AF_INET6) {
		attr.m_family = stun_attribute_xor_mapped_address::IPV6;
		const sockaddr_in6* sin = (const sockaddr_in6*)addr;

		attr.m_port = sys::socket::ntoh16(sin->sin6_port);
		attr.m_port ^= 0x2112;

		attr.m_address[0] = sin->sin6_addr.s6_addr[0] ^ 0x21;
		attr.m_address[1] = sin->sin6_addr.s6_addr[1] ^ 0x12;
		attr.m_address[2] = sin->sin6_addr.s6_addr[2] ^ 0xA4;
		attr.m_address[3] = sin->sin6_addr.s6_addr[3] ^ 0x42;

		for (int i = 0; i < 12; i++)
		{
			attr.m_address[i + 4] = sin->sin6_addr.s6_addr[i + 4] ^ tid[i];
		}

	}

	return attr;
}


std::string stun_attribute_message_integrity::create_short_turn(const char* data, int data_size,const std::string& password)
{

	sasl_prep((uint8_t*)password.data());
	hmac_sha1 ms;
	BYTE ret[20] = { 0 };
	ms.make_bytes((const BYTE*)data, data_size, (const BYTE*)password.data(), password.size(), ret);

	return std::string((const char*)ret, 20);
}

std::string stun_attribute_message_integrity::create_long_turn(const char* data, int data_size,
	const std::string& username, const std::string& realm, const std::string& password)
{
	std::string key = username;
	key += ":";
	key += realm;
	key += ":";
	key += password;

	MD5 m5 = { 0 };
	md5((const unsigned char*)key.c_str(),key.size(), &m5);

	hmac_sha1 ms;
	BYTE ret[20] = { 0 };
	ms.make_bytes((const BYTE*)data, data_size, m5.bv, 16, ret);

	return std::string((const char*)ret, 20);
}
