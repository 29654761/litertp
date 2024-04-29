
/**
 * @file stun_attribute.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */



#pragma once
#include <string>




enum stun_attribute_type
{
	stun_attribute_type_none = 0x0000,
	stun_attribute_type_mapped_address = 0x0001,
	stun_attribute_type_username = 0x0006,
	stun_attribute_type_message_integrity = 0x0008,
	stun_attribute_type_error_code = 0x0009,
	stun_attribute_type_unknown_attributes = 0x000a,
	stun_attribute_type_realm = 0x0014,
	stun_attribute_type_nonce = 0x0015,
	stun_attribute_type_priority = 0x0024,
	stun_attribute_type_use_candidate = 0x0025,
	stun_attribute_type_xor_mapped_address = 0x0020,
	stun_attribute_type_software = 0x8022,
	stun_attribute_type_alternate_server = 0x8023,
	stun_attribute_type_fingerprint = 0x8028,
	stun_attribute_type_origin = 0x802f,
	stun_attribute_type_retransmit_count = 0xff00,

	stun_attribute_type_goog_network_info = 0xc057,
	stun_attribute_type_ice_controlled = 0x8029,
	stun_attribute_type_ice_controlling = 0x802a,
};

enum stun_error_code
{
	stun_error_code_try_alternate = 300,
	stun_error_code_bad_request = 400,
	stun_error_code_unauthorized = 401,
	stun_error_code_unknown_attribute = 420,
	stun_error_code_stale_credentials = 430,
	stun_error_code_stale_nonce = 438,
	stun_error_code_server_error = 500,
	stun_error_code_global_failure = 600,
};


class stun_attribute
{
public:
	stun_attribute();
	stun_attribute(stun_attribute_type type, const std::string& value);
	stun_attribute(stun_attribute_type type, const char* value, size_t size);
	~stun_attribute() {
		int a = 0;
	}
	std::string serialize()const;
	int deserialize(const char* buffer, int size);



public:
	stun_attribute_type type_;
	std::string value_;
};




class stun_attribute_error_code
{
public:
	stun_attribute_error_code() {
		class_ = 0;
		number_ = stun_error_code::stun_error_code_bad_request;
	}
	std::string serialize()const;
	bool deserialize(const char* buffer, int size);

	uint8_t class_ : 3;
	stun_error_code number_;
	std::string reason_;


};



class stun_attribute_mapped_address
{
public:
	static const uint8_t IPV4 = 0x01;
	static const uint8_t IPV6 = 0x02;

	uint8_t m_family = IPV4;
	uint16_t m_port = 0;
	uint8_t m_address[16] = { 0 };

	std::string serialize()const;
	bool deserialize(const char* buffer, int size);
};

struct sockaddr;
class stun_attribute_xor_mapped_address :public stun_attribute_mapped_address
{
public:
	static stun_attribute_xor_mapped_address create(const sockaddr* addr, const char* tid);
};


class stun_attribute_message_integrity
{
public:
	static std::string create_short_turn(const char* data, int data_size, const std::string& password);
	static std::string create_long_turn(const char* data, int data_size,
		const std::string& username, const std::string& realm, const std::string& password);

};

