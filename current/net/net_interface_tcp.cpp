// net_interface_tcp.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// TCP implementation of net_socket


#include "net/net_interface_tcp.h"
#include "base/tu_timer.h"
#include "base/container.h"

static lfl_string s_proxy;
static int s_proxy_port;

void set_proxy(const char* host, int port)
{
	s_proxy = host ? host : "";
	s_proxy_port = port;
}

int get_proxy_port()
{
	return s_proxy_port;
}

const char* get_proxy()
{
	return s_proxy.c_str();
}

net_socket_tcp::net_socket_tcp(SOCKET sock, const char* client_ip) :
	m_sock(sock),
	m_error(0),
	m_client_ip(client_ip)
{
}

net_socket_tcp::~net_socket_tcp()
{
	closesocket(m_sock);
}

int net_socket_tcp::get_error() const
{
	return m_error;
}

bool net_socket_tcp::is_open() const
{
	return get_error() == 0;
}

bool net_socket_tcp::is_readable() const
// Return true if this socket has incoming data available.
{
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_sock, &fds);
	struct timeval tv = { 0, 0 };

#ifdef WIN32
	// the first arg to select in win32 is ignored.
	// It's included only for compatibility with Berkeley sockets.
	select(1, &fds, NULL, NULL, &tv);
#else
	// It should be the value of the highest numbered FD within any of the passed fd_sets,
	// plus one... Because, the max FD value + 1 == the number of FDs
	// that select() must concern itself with, from within the passed fd_sets...)
	select(m_sock + 1, &fds, NULL, NULL, &tv);
#endif

	if (FD_ISSET(m_sock, &fds))
	{
		// Socket has data available.
		return true;
	}

	return false;
}

int net_socket_tcp::read(void* data, int bytes, float timeout_seconds)
// Try to read the requested number of bytes.  Returns the
// number of bytes actually read.
{
	Uint32 start = tu_timer::get_ticks();

	int total_bytes_read = 0;

	for (;;)
	{
		int bytes_read = recv(m_sock, (char*) data, bytes, 0);

		if (bytes_read == 0)
		{
			break;
		}

		if (bytes_read == SOCKET_ERROR)
		{
			m_error = WSAGetLastError();
			if (m_error == WSAEWOULDBLOCK)
			{
				// No data ready.
				m_error = 0;

				// Timeout?
				Uint32 now = tu_timer::get_ticks();
				double elapsed = (now - start) / 1000.0;	// convert to second

				if (elapsed < timeout_seconds)
				{
					// TODO this spins; fix it by sleeping a bit?
					// if (time_left > 0.010f) { sleep(...); }
					continue;
				}

				// Timed out.
				return total_bytes_read;
			}
			else
			{
				// Presumably something bad.
				fprintf(stderr, "net_socket_tcp::read() error in recv, error code = %d\n",
					WSAGetLastError());
				return total_bytes_read;
			}
		}

		// Got some data.
		total_bytes_read += bytes_read;
		bytes -= bytes_read;
		data = (void*) (((char*) data) + bytes_read);

		assert(bytes >= 0);

		if (bytes == 0)
		{
			// Done.
			break;
		}
	}

	return total_bytes_read;
}


int net_socket_tcp::read_line(lfl_string* str, int maxbytes, float timeout_seconds)
// Try to read a string, up to the next '\n' character.
// Returns the actual bytes read.
//
// If the last character in the string is not '\n', then
// either we read maxbytes, or we timed out or the socket
// closed or something.
{
	assert(str);

	Uint32 start = tu_timer::get_ticks();
	Uint32 timeout = Uint32(timeout_seconds * 1000);	// ticks, ms
	int total_bytes_read = 0;

	for (;;)       
	{
		// Read a byte at a time.  Probably slow!
		char c;
		bool waiting = false;
		int bytes_read = recv(m_sock, &c, 1, 0);
		if (bytes_read == SOCKET_ERROR)
		{
			m_error = WSAGetLastError();
			if (m_error == WSAEWOULDBLOCK)
			{
				// No data ready.
				m_error = 0;
				waiting = true;
			}
			else
			{
				// Presumably something bad.
				fprintf(stderr, "net_socket_tcp::read() error in recv, error code = %d\n",
					WSAGetLastError());
				return 0;
			}
		} 
		else
		if (bytes_read == 0)
		{
			if (is_readable() && recv(m_sock, &c, 1, 0) == 0)
			{
				// Socket must close
				return total_bytes_read ? total_bytes_read : -1;

			}
			waiting = true;
		}

		if (waiting) 
		{
			// Timeout?
			if (tu_timer::get_ticks() - start >= timeout)
			{
				// Timed out.
				return 0;
			}

			// sleep a bit
			if (timeout - (tu_timer::get_ticks() - start) >= 10)
			{
				tu_timer::sleep(10);
			}
			continue;
		}

		assert(bytes_read == 1);
		total_bytes_read++;

		if (c == '\n')
		{
			// Done.
//			printf("recv: '%s'\n", str->c_str());
			return total_bytes_read;
		}

		// '\n'(0x0D) and '\r'(0x0A) are not written into str
		if (c != '\r')
		{
			(*str) += c;
		}

		if (maxbytes > 0 && total_bytes_read >= maxbytes)
		{
			// Caller doesn't want any more bytes.
			return total_bytes_read;
		}

		// else keep reading.
	}

	return 0;
}


int net_socket_tcp::write(const void* data, int bytes, float timeout_seconds)
// Try to write the given data buffer.  Return the number of
// bytes actually written.
{
	Uint32 start = tu_timer::get_ticks();

	int total_bytes_written = 0;

	for (;;)
	{
		int bytes_sent = send(m_sock, (const char*) data, bytes, 0);
		if (bytes_sent == SOCKET_ERROR)
		{
			m_error = WSAGetLastError();
			if (m_error == WSAENOBUFS || m_error == WSAEWOULDBLOCK)
			{
				// Non-fatal.
				m_error = 0;

				Uint32 now = tu_timer::get_ticks();
				double elapsed = (now - start) / 1000.0;	// convert to second

				if (elapsed < timeout_seconds)
				{
					// Keep trying.
					// TODO possibly sleep() here
					continue;
				}

				// Timed out.
				return total_bytes_written;
			}

			// write error
			return total_bytes_written;

		}

		total_bytes_written += bytes_sent;
		data = (const void*) (((const char*) data) + bytes_sent);
		bytes -= bytes_sent;

		assert(bytes >= 0);
		if (bytes == 0)
		{
			// Done.
			return total_bytes_written;
		}
		// else keep trying.
	}
}

//	server
net_interface_tcp::net_interface_tcp(int port_number) :
m_port_number(port_number),
m_socket(INVALID_SOCKET)
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		fprintf(stderr, "can't open listen socket\n");

		return;
	}

	// Set the address.
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(m_port_number);

	// bind the address
	int ret = bind(m_socket, (LPSOCKADDR) &saddr, sizeof(saddr));
	if (ret == SOCKET_ERROR)
	{
		fprintf(stderr, "bind failed\n");
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return;
	}

	// gethostname

	// Start listening.
	ret = listen(m_socket, SOMAXCONN);
	if (ret == SOCKET_ERROR)
	{
		fprintf(stderr, "listen() failed\n");
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return;
	}

	// Set non-blocking mode for the socket, so that
	// accept() doesn't block if there's no pending
	// connection.
	set_nonblock();
}

void net_interface_tcp::set_nonblock()
{
#ifdef _WIN32
	int mode = 1;
	ioctlsocket(m_socket, FIONBIO, (u_long FAR*) &mode);
#else
	int mode = fcntl(m_socket, F_GETFL, 0);
	mode |= O_NONBLOCK;
	fcntl(m_socket, F_SETFL, mode);
#endif
}

// client
net_interface_tcp::net_interface_tcp() :
m_socket(INVALID_SOCKET)
{
}

net_socket* net_interface_tcp::connect(const char* c_host, int port)
{
	assert(c_host);
	assert(port > 0);

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET)
	{
		fprintf(stderr, "can't open listen socket\n");
		return NULL;
	}

	// Set the address.
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	m_port_number = get_proxy_port() > 0 ? get_proxy_port() : port;
	saddr.sin_port = htons(m_port_number);


	hostent* he;
	const char* host = get_proxy_port() > 0 ? get_proxy() : c_host;
	if (host[0] >= '0' && host[0] <= '9')	// absolue address ?
	{
		Uint32 addr = inet_addr(host);
		he = gethostbyaddr((char *) &addr, 4, AF_INET);
	}
	else
	{
		he = gethostbyname(host);
	}

	if (he == NULL)
	{
		fprintf(stderr, "can't find '%s'\n", host);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return NULL;
	}

	// get server address
	memcpy(&saddr.sin_addr, he->h_addr, he->h_length);

	//      printf("The IP address is '%d.%d.%d.%d'\n", 
	//				saddr.sin_addr.S_un.S_un_b.s_b1,
	//				saddr.sin_addr.S_un.S_un_b.s_b2,
	//				saddr.sin_addr.S_un.S_un_b.s_b3,
	//				saddr.sin_addr.S_un.S_un_b.s_b4);

	int rc = ::connect(m_socket, (struct sockaddr*) &saddr, sizeof(struct sockaddr));
	if (rc != 0)
	{
		fprintf(stderr, "can't connect to '%s', port %d\n", host, m_port_number);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
		return NULL;
	}

	// Set non-blocking mode for the socket, so that
	// accept() doesn't block if there's no pending
	// connection.
	set_nonblock();

	return new net_socket_tcp(m_socket, NULL);	// fixme: IP address
}

net_interface_tcp::~net_interface_tcp()
{
	closesocket(m_socket);
}

bool net_interface_tcp::is_valid() const
{
	if (m_socket == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

net_socket* net_interface_tcp::accept()
{
	// Accept an incoming request.
	SOCKET	remote_socket;
	struct sockaddr_in client;
	int ln = sizeof(sockaddr_in);
	remote_socket = ::accept(m_socket, (sockaddr*) &client, &ln);
	if (remote_socket == INVALID_SOCKET)
	{
		// No connection pending, or some other error.
		return NULL;
	}

	char buf[16];
	snprintf(buf, 16, "%d.%d.%d.%d",
		client.sin_addr.S_un.S_un_b.s_b1,
		client.sin_addr.S_un.S_un_b.s_b2,
		client.sin_addr.S_un.S_un_b.s_b3,
		client.sin_addr.S_un.S_un_b.s_b4);

	return new net_socket_tcp(remote_socket, buf);
}

bool net_init()
{
#ifdef _WIN32
	WORD version_requested = MAKEWORD(1, 1);
	WSADATA wsa;

	WSAStartup(version_requested, &wsa);

	if (wsa.wVersion != version_requested)
	{	
		// TODO log_error
		fprintf(stderr, "Bad Winsock version %d\n", wsa.wVersion);
		return false;
	}
#endif
	return true;
}

// server
net_interface* tu_create_net_interface_tcp(int port_number)
{
	if (net_init())
	{
		net_interface_tcp* iface = new net_interface_tcp(port_number);
		if (iface == NULL || iface->is_valid() == false)
		{
			delete iface;
			return NULL;
		}
		return iface;
	}
	return NULL;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
