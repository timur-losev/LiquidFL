// net_interface_tcp.h	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

#ifndef NET_INTERFACE_TCP_H
#define NET_INTERFACE_TCP_H

#ifdef _WIN32

#include "Winsock.h"

//#ifndef strdup
//#	define strdup _strdup
//#endif

#else

#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

typedef int SOCKET;

#define closesocket close
#define SOCKET_ERROR -1
#define WSAGetLastError() errno
#define WSAEWOULDBLOCK EAGAIN
#define INVALID_SOCKET ENOTSOCK
#define WSAENOBUFS EAGAIN
#define SOCKADDR_IN sockaddr_in
#define LPSOCKADDR sockaddr*

#endif

#include "net/net_interface.h"

void set_proxy(const char* host, int port);
int get_proxy_port();
const char* get_proxy();
bool net_init();

// server
net_interface* tu_create_net_interface_tcp(int port_number);


struct net_socket_tcp : public net_socket
{
	SOCKET m_sock;
	int m_error;
	lfl_string m_client_ip;
	
	net_socket_tcp(SOCKET sock, const char* client_ip);
	~net_socket_tcp();

	virtual int get_error() const;
	virtual bool is_open() const;
	virtual bool is_readable() const;
	virtual int read(void* data, int bytes, float timeout_seconds);
	virtual int read_line(lfl_string* data, int maxbytes, float timeout_seconds);
	virtual int write(const void* data, int bytes, float timeout_seconds);

	const char* get_ip() const { return m_client_ip.c_str(); };
};

struct net_interface_tcp : public net_interface
{
	int m_port_number;
	SOCKET m_socket;

	~net_interface_tcp();

	//	server
	net_interface_tcp(int port_number);


	// client
	net_interface_tcp();
	net_socket* connect(const char* c_host, int port); 

	void set_nonblock();
	bool is_valid() const;
	net_socket* accept();
};

#endif
