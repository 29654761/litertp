/**
 * @file socket.h
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#pragma once

#include <string>
#include <vector>
#include <memory>


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include<WinSock2.h>
#include<WS2tcpip.h> 
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
typedef int SOCKET;
#endif

namespace sys {



class socket
{
private:
	SOCKET m_socket = 0;
	sockaddr_storage m_remote_addr = {0};
public:
	socket();
	socket(SOCKET skt);
	~socket();



	static int global_init();
	static void global_uninit();

	static void addr2ep(const sockaddr* addr, std::string* ip, int* port);
	static void ep2addr(unsigned short family, const char* ip, int port, sockaddr* addr);
	static void ep2addr(const char* ip, int port, sockaddr* addr);

	static bool is_big_endian();

	static uint64_t swap64(uint64_t v);
	static uint32_t swap32(uint32_t v);
	static uint16_t swap16(uint16_t v);

	static uint64_t hton64(uint64_t v);
	static uint32_t hton32(uint32_t v);
	static uint16_t hton16(uint16_t v);
	static uint64_t ntoh64(uint64_t v);
	static uint32_t ntoh32(uint32_t v);
	static uint16_t ntoh16(uint16_t v);

	static bool is_ipv4(const std::string& ip);
	static bool is_ipv6(const std::string& ip);

	static bool get_addresses_byhost(const char* host, std::vector<sockaddr_storage>& addresses);

	bool create_udp_socket(int family);
	bool create_tcp_socket(int family);
	void close();

	SOCKET handle()const { return m_socket; }

	bool connect(const sockaddr* addr, socklen_t addr_size);
	bool bind(const sockaddr* addr, socklen_t addr_size);
	bool listen(int q);

	std::shared_ptr<socket> accept();

	int send(const char* buffer, int size);
	int sendto(const char* buffer, int size, const sockaddr* addr, socklen_t addr_size);

	int recv(char* buffer, int size);
	int recvfrom(char* buffer, int size, sockaddr* addr, socklen_t* addrSize);

	const sockaddr* remote_addr()const;
	socklen_t remote_addr_size()const;
	void remote_addr(std::string& addr, int& port);
	void set_remote_addr(const sockaddr* addr, socklen_t addr_size);
public:
	bool set_reuseaddr(bool enable);
	bool get_reuseaddr();
	bool set_sendbuf_size(int size);
	int get_sendbuf_size();
	bool set_recvbuf_size(int size);
	int get_recvbuf_size();

	bool set_timeout(int ms);
};



class socket_selector
{
private:
	fd_set m_read_set;
	fd_set m_write_set;
	fd_set m_error_set;
	int32_t m_maxfd;
public:
	socket_selector();
	~socket_selector();

	void reset();
	void add_to_read(SOCKET skt);
	void add_to_write(SOCKET skt);

	bool is_readable(SOCKET skt);
	bool is_writeable(SOCKET skt);

	int wait(int32_t sec = -1);
};

}
