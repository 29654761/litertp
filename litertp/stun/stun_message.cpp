
/**
 * @file stun_message.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "stun_message.h"
#include <sys2/socket.h>
#include <sys2/security/crc32.h>
#include <random>
#include <string.h>

stun_message::stun_message()
{
}

stun_message::~stun_message()
{
}


std::string stun_message::serialize(int placeholder)const
{
	std::string data;
	for (auto attr : attributes_) {
		data.append(attr.serialize());
	}

	std::string s;
	s.reserve(20 + data.size());

	uint16_t type = sys::socket::hton16(type_) & 0x3FFF;
	s.append((const char*)&type, 2);

	uint16_t len = sys::socket::hton16((uint16_t)(data.size() + placeholder));
	s.append((const char*)&len, 2);

	uint32_t magic_cookie = sys::socket::hton32(magic_cookie_);
	s.append((const char*)&magic_cookie, 4);

	s.append((const char*)transaction_id_, 12);
	s.append(data);
	return s;
}

std::string stun_message::serialize(const std::string& password)const
{
	std::string s = serialize(24);
	add_message_integrity_short_turn(s, password);

	uint16_t len = s.size() - 20 + 8;
	//len = sys::socket::hton16(len);
	s[2] = len >> 8;
	s[3] = len & 0x00FF;

	add_fingerprint(s);
	return s;
}

std::string stun_message::serialize(const std::string& username, const std::string& realm, const std::string& password)const
{
	std::string s = serialize(24);
	add_message_integrity_short_turn(s, password);
	add_fingerprint(s);
	return s;
}

bool stun_message::deserialize(const char* buffer, int size, uint32_t& fingerprint)
{
	if (size < 20) {
		return false;
	}

	fingerprint = calculate_fingerprint(buffer, size - 8);

	uint16_t type = 0;
	memcpy(&type, buffer, 2);
	type_ = (stun_message_type)sys::socket::ntoh16(type);

	uint16_t len = 0;
	memcpy(&len, buffer + 2, 2);
	len = sys::socket::ntoh16(len);

	if (size < len + 20) {
		return false;
	}


	memcpy(&magic_cookie_, buffer + 4, 4);
	magic_cookie_ = sys::socket::ntoh32(magic_cookie_);

	memcpy(transaction_id_, buffer + 8, 12);

	int offset = 0;
	const char* buf = buffer + 20;
	int buf_size = size - 20;
	attributes_.clear();
	do {
		stun_attribute attr;
		offset = attr.deserialize(buf, buf_size);
		if (offset <= 0) {
			break;
		}
		buf += offset;
		buf_size -= offset;
		attributes_.push_back(attr);
	} while (true);



	return true;
}

std::string stun_message::calculate_message_integrity(const std::string& password)
{
	stun_message msg = *this;
	msg.attributes_.erase(msg.attributes_.end() - 1);  //Remove fingerprint
	std::string s = msg.serialize();
	return stun_attribute_message_integrity::create_short_turn(s.data(), s.size() - 24, password);
}

std::string stun_message::calculate_message_integrity(const std::string& username, const std::string& realm, const std::string& password)
{
	stun_message msg = *this;
	msg.attributes_.erase(msg.attributes_.end() - 1);//Remove fingerprint
	std::string s = msg.serialize();
	return stun_attribute_message_integrity::create_long_turn(s.data(), s.size() - 24,username,realm, password);
}

void stun_message::add_message_integrity_short_turn(std::string& data, const std::string& password)
{
	std::string code = stun_attribute_message_integrity::create_short_turn(data.c_str(), data.size(), password);
	stun_attribute mi_attr(stun_attribute_type_message_integrity, code);
	std::string mi = mi_attr.serialize();
	data.append(mi);
	
}

void stun_message::add_message_integrity_long_turn(std::string& data, const std::string& username, const std::string& realm, const std::string& password)
{
	std::string code = stun_attribute_message_integrity::create_long_turn(data.c_str(), data.size(), username,realm, password);
	stun_attribute mi_attr(stun_attribute_type_message_integrity, code);
	std::string mi = mi_attr.serialize();
	data.append(mi);
}

void stun_message::add_fingerprint(std::string& data)
{
	uint32_t crc = calculate_fingerprint(data.c_str(), data.size());
	crc = sys::socket::hton32(crc);
	stun_attribute fp_attr(stun_attribute_type_fingerprint, (const char*)&crc, 4);

	std::string fp = fp_attr.serialize();
	data.append(fp);

	
}

uint32_t stun_message::calculate_fingerprint(const char* data, int data_size)
{
	uint32_t crc = cyg_ether_crc32((const unsigned char*)data, data_size);
	crc ^= 0x5354554e;
	return crc;
}

void stun_message::make_transaction_id()
{
	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_int_distribution<uint32_t> distr(0, 0xFF);
	for (int i = 0; i < 12; i++) {
		transaction_id_[i] = (uint8_t)distr(eng);
	}
}

std::string stun_message::get_attribute(stun_attribute_type type)const
{
	std::string ret;
	for (auto& attr : attributes_)
	{
		if (attr.type_ == type)
		{
			ret = attr.value_;
			break;
		}
	}

	return ret;
}

