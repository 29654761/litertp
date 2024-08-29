/**
 * @file socket.cpp
 * @brief
 * @author Shijie Zhou
 * @copyright 2024 Shijie Zhou
 */


#include "socket.h"
#include <string.h>


namespace sys{


socket::socket(int af, int type, int protocol)
{
	socket_ = ::socket(af, type,protocol);
	if (ready())
	{
		int no = 0;
		setsockopt(socket_, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&no, sizeof(no));
	}
}

socket::socket(SOCKET skt)
{
	socket_ = skt;
}

socket::~socket()
{
	close();
}

int socket::global_init() 
{
#ifdef _WIN32
	WSADATA wd;
	memset(&wd, 0, sizeof(wd));
	WSAStartup(MAKEWORD(2, 2), &wd);
#else

#endif // _WIN32

	return 0;
}


void socket::global_cleanup()
{
#ifdef _WIN32
	WSACleanup();
#else

#endif // _WIN32
}


void socket::addr2ep(const sockaddr* addr, std::string* ip, int* port) 
{
	if (addr->sa_family == AF_INET) {
		char sip[16] = { 0 };
		sockaddr_in* sin = (sockaddr_in*)addr;
		inet_ntop(AF_INET, &sin->sin_addr, sip, 16);
		ip->assign(sip);
		*port = ntohs(sin->sin_port);
	}
	else {
		char sip[64] = { 0 };
		sockaddr_in6* sin = (sockaddr_in6*)addr;
		inet_ntop(AF_INET6, &sin->sin6_addr, sip, 64);
		ip->assign(sip);
		*port = ntohs(sin->sin6_port);
	}
}


void socket::ep2addr(unsigned short family,const char* ip, int port, sockaddr* addr)
{
	if (family== AF_INET) {
		sockaddr_in* sin = (sockaddr_in*)addr;
		sin->sin_family = AF_INET;
		sin->sin_port = htons(port);
		inet_pton(AF_INET, ip?ip:"0.0.0.0", &sin->sin_addr);
	}
	else if(family == AF_INET6){
		sockaddr_in6* sin = (sockaddr_in6*)addr;
		sin->sin6_family = AF_INET6;
		sin->sin6_port = htons(port);

		if (!ip)
		{
			inet_pton(AF_INET6, "::", &sin->sin6_addr);
		}
		else 
		{
			std::string s = ip;
			if (is_ipv4(s))
			{
				s = "::ffff:" + s;
			}
			inet_pton(AF_INET6, s.c_str(), &sin->sin6_addr);
		}
	}
}

void socket::ep2addr(const char* ip, int port, sockaddr* addr)
{
	if (is_ipv4(ip))
	{
		ep2addr(AF_INET, ip, port, addr);
	}
	else if(is_ipv6(ip))
	{
		ep2addr(AF_INET6, ip, port, addr);
	}
}


bool socket::is_big_endian() 
{
	union {
		int i;
		char c[4];
	} v = { 0x00000001 };
	return v.c[3] == 1;
}

uint64_t socket::swap64(uint64_t v)
{
	uint64_t r = 0;
	char* p = (char*)&r;
	p[0] = (char)(v >> 56);
	p[1] = (char)((v & 0x00FF000000000000) >> 48);
	p[2] = (char)((v & 0x0000FF0000000000) >> 40);
	p[3] = (char)((v & 0x000000FF00000000) >> 32);
	p[4] = (char)((v & 0x00000000FF000000) >> 24);
	p[5] = (char)((v & 0x0000000000FF0000) >> 16);
	p[6] = (char)((v & 0x000000000000FF00) >> 8);
	p[7] = (char)((v & 0x00000000000000FF));
	return r;
}

uint32_t socket::swap32(uint32_t v)
{
	uint32_t r = 0;
	char* p = (char*)&r;
	p[0] = (char)(v >> 24);
	p[1] = (char)((v & 0x00FF0000) >> 16);
	p[2] = (char)((v & 0x0000FF00) >> 8);
	p[3] = (char)((v & 0x000000FF));
	return r;
}

uint16_t socket::swap16(uint16_t v)
{
	uint16_t r = 0;
	char* p = (char*)&r;
	p[0] = (char)(v >> 8);
	p[1] = (char)(v & 0x00FF);
	return r;
}

uint64_t socket::hton64(uint64_t v)
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap64(v);
	}
}
uint32_t socket::hton32(uint32_t v)
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap32(v);
	}
}
uint16_t socket::hton16(uint16_t v) 
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap16(v);
	}
}
uint64_t socket::ntoh64(uint64_t v)
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap64(v);
	}
}
uint32_t socket::ntoh32(uint32_t v)
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap32(v);
	}
}
uint16_t socket::ntoh16(uint16_t v)
{
	if (is_big_endian()) {
		return v;
	}
	else {
		return swap16(v);
	}
}

bool socket::is_ipv4(const std::string& ip)
{
	int dotcnt = 0, colon = 0;
	for (int i = 0; i < ip.length(); i++)
	{
		if (ip[i] == '.')
		{
			dotcnt++;
		}
		else if (ip[i] == ':')
		{
			colon++;
		}
	}

	return dotcnt == 3 && colon == 0;
}

bool socket::is_ipv6(const std::string& ip)
{
	int dotcnt = 0, colon = 0;
	for (int i = 0; i < ip.length(); i++)
	{
		if (ip[i] == '.')
		{
			dotcnt++;
		}
		else if (ip[i] == ':')
		{
			colon++;
		}
	}

	return (colon == 7&& dotcnt==0)||(colon == 3 && dotcnt == 3);
}

bool socket::get_addresses_byhost(const char* host, std::vector<sockaddr_storage>& addresses)
{
	hostent* ent = gethostbyname(host);
	if (!ent) {
		return false;
	}

	int i = 0;
	while (ent->h_addr_list[i])
	{
		sockaddr_storage ss = { 0 };
		memcpy(&ss, ent->h_addr_list[i], ent->h_length);
		addresses.push_back(ss);
		i++;
	}
	return true;
}

bool socket::shutdown(shutdown_how_t how)
{
#ifdef _WIN32
	int r=::shutdown(socket_, (int)how);
	return r >= 0;
#else

#endif
}

void socket::close()
{
#ifdef _WIN32
	closesocket(socket_);
#else
	shutdown(shutdown_how_t::sd_both);
	::close(socket_);
#endif
}

bool socket::ready()
{
#ifdef _WIN32
	return socket_ != INVALID_SOCKET;
#else
	return socket_ >= 0;
#endif
}

bool socket::connect(const sockaddr* addr, socklen_t addr_size)
{
	if (!ready())
		return false;

	set_remote_addr(addr, addr_size);
	int r = ::connect(socket_, addr, addr_size);
	if (r != 0) {
		return false;
	}

	return true;
}
bool socket::bind(const sockaddr* addr, socklen_t addr_size)
{
	int r = ::bind(socket_, addr, addr_size);
	if (r < 0)
		return false;

	return true;
}
bool socket::listen(int q) 
{
	int r = ::listen(socket_, q);
	if (r < 0)
		return false;

	return true;
}

socket_ptr socket::accept()
{
	sockaddr_storage addr = { 0 };
	socklen_t addrlen = sizeof(addr);
	SOCKET skt = ::accept(socket_, (sockaddr*)&addr, &addrlen);
	auto ptr = std::make_shared<socket>(skt);
	if (!ptr->ready())
	{
		return nullptr;
	}
	ptr->set_remote_addr((const sockaddr*)&addr, addrlen);
	return ptr;
}


int socket::send(const char* buffer, int size)
{
	return ::send(socket_, buffer, size, 0);
}
int socket::sendto(const char* buffer, int size, const sockaddr* addr, socklen_t addr_size)
{
	//std::string ip;
	//int port = 0;
	//addr2ep(addr, &ip, &port);
	return ::sendto(socket_, buffer, size, 0, addr, addr_size);
}

int socket::recv(char* buffer, int size) 
{
	return ::recv(socket_, buffer, size, 0);
}
int socket::recvfrom(char* buffer, int size, sockaddr* addr, socklen_t* addrSize) 
{
	return ::recvfrom(socket_, buffer, size, 0, addr, addrSize);
}

const sockaddr* socket::remote_addr()const
{
	return (const sockaddr*)&remote_addr_;
}

socklen_t socket::remote_addr_size()const
{
	return sizeof(remote_addr_);
}

void socket::remote_addr(std::string& addr, int& port)
{
	addr2ep((const sockaddr*)&remote_addr_, &addr, &port);
}
void socket::set_remote_addr(const sockaddr* addr, socklen_t addr_size)
{
	memcpy(&remote_addr_, addr, addr_size);
}

bool socket::set_reuseaddr(bool enable)
{
	int v = enable ? 1 : 0;
	if (setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&v, sizeof(int)) < 0) {
		return false;
	}
	return true;
}

bool socket::get_reuseaddr()
{
	int v = 0;
	socklen_t s = sizeof(v);
	getsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)&v, &s);
	return !!v;
}

bool socket::set_sendbuf_size(int size)
{
	if (setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (const char*)&size, (socklen_t)sizeof(size)) < 0) {
		return false;
	}
	return true;
}

int socket::get_sendbuf_size()
{
	int v = 0;
	socklen_t s = sizeof(v);
	getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, (char*)&v, &s);


	return v;
}

bool socket::set_recvbuf_size(int size)
{
	if (setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) < 0) {
		return false;
	}
	return true;
}

int socket::get_recvbuf_size()
{
	int v = 0;
	socklen_t s = sizeof(v); 
	getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, (char*)&v, &s);
	return v;
}

bool socket::set_timeout(int ms)
{
#ifdef _WIN32
	DWORD timeout = ms;
	int r=setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
	return r == 0 ? true : false;


#else
	struct timeval tv;
	tv.tv_sec = ms / 1000;
	tv.tv_usec = (ms % 1000) * 1000;
	int r=setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	return r == 0 ? true : false;
#endif
}


socket_selector::socket_selector()
{
	reset();
}

socket_selector::~socket_selector()
{
}

void socket_selector::reset()
{
	FD_ZERO(&read_set_);
	FD_ZERO(&write_set_);
	FD_ZERO(&error_set_);
	maxfd_ = 0;
}

void socket_selector::add_to_read(SOCKET skt)
{
	FD_SET(skt, &read_set_);
	if (skt > maxfd_)
		maxfd_ = skt;
}

void socket_selector::add_to_write(SOCKET skt)
{
	FD_SET(skt, &write_set_);
	if (skt > maxfd_)
		maxfd_ = skt;
}

bool socket_selector::is_readable(SOCKET skt)
{
	return FD_ISSET(skt, &read_set_);
}

bool socket_selector::is_writeable(SOCKET skt)
{
	return FD_ISSET(skt, &write_set_);
}

int socket_selector::wait(int32_t sec)
{
	timeval tv = { sec,0 };
	return select(maxfd_ + 1, &read_set_, &write_set_, &error_set_, sec >= 0 ? (&tv) : nullptr);
}


}