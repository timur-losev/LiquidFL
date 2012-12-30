// http_client.h	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// HTTP implementation of tu_file

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#define HTTP_SERVER_PORT 80
#define HTTP_TIMEOUT 10	// sec

#include "base/container.h"
#include "net/net_interface_tcp.h"

// this is used for HTTP connection
// if connection is going through proxy server
void set_proxy(const char* host, int port);

// HTTP client interface.
struct http_connection : public net_interface_tcp
{
	http_connection();
	~http_connection();
	bool is_open();
	net_socket* connect(const char* url, int port);
	int read(void* data, int bytes);
	int read_line(lfl_string* data, int maxbytes);
	int write(const void* data, int bytes);
	int write_string(const char* str);

private:

	bool read_response();

	net_interface* m_iface;
	net_socket* m_ns;

};

#endif

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
