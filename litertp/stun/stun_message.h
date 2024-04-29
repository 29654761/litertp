/**
 * @file stun_message.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once

#include <vector>
#include "stun_attribute.h"

enum stun_message_type
{
	stun_message_type_binding_request = 0x0001,
	stun_message_type_binding_response = 0x0101,
	stun_message_type_binding_error_response = 0x111,
	stun_message_type_binding_indication = 0x0011,

	stun_message_type_shared_request = 0x0002,
	stun_message_type_shared_response = 0x0102,
	stun_message_type_shared_error_response = 0x112,
	stun_message_type_shared_indication = 0x0012,
};

#define MAGIC_COOKIE 0x2112A442

class stun_message
{
public:
	stun_message();
	~stun_message();



	
	std::string serialize(const std::string& password)const;
	std::string serialize(const std::string& username, const std::string& realm, const std::string& password)const;

	bool deserialize(const char* buffer, int size,uint32_t& fingerprint);

	
	std::string calculate_message_integrity(const std::string& password);
	std::string calculate_message_integrity(const std::string& username, const std::string& realm, const std::string& password);

	void make_transaction_id();

	std::string get_attribute(stun_attribute_type type)const;
private:
	std::string serialize(int placeholder = 0)const;
	static void add_message_integrity_short_turn(std::string& data, const std::string& password);
	static void add_message_integrity_long_turn(std::string& data, const std::string& username, const std::string& realm, const std::string& password);
	static void add_fingerprint(std::string& data);
	static uint32_t calculate_fingerprint(const char* data,int data_size);
public:
	stun_message_type type_ : 14;
	uint32_t magic_cookie_ = MAGIC_COOKIE;
	uint8_t transaction_id_[12] = { 0 };

	std::vector<stun_attribute> attributes_;

};
