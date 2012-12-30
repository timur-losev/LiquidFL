// http_client.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// HTTP implementation of tu_file

#include "base/utility.h"
#include "base/tu_file.h"
#include "net/http_client.h"
#include "net/net_interface_tcp.h"

#ifdef _WIN32
#define snprintf _snprintf
#endif // _WIN32


http_connection::http_connection() :
	m_iface(NULL),
	m_ns(NULL)
{
}

http_connection::~http_connection()
{
	delete m_ns;
	delete m_iface;
}

int http_connection::read(void* data, int bytes)
{
	return m_ns->read(data, bytes, HTTP_TIMEOUT);
}

int http_connection::read_line(lfl_string* data, int maxbytes)
{
	return m_ns->read_line(data, maxbytes, HTTP_TIMEOUT);
}

int http_connection::write(const void* data, int bytes)
{
	return m_ns->write(data, bytes, HTTP_TIMEOUT);
}

int http_connection::write_string(const char* str)
{
	return m_ns->write_string(str, HTTP_TIMEOUT);
}

bool http_connection::is_open()
{
	return m_ns != NULL && m_iface != NULL;
}

net_socket* http_connection::connect(const char* c_url, int port)
{
	char* url = strdup(c_url);

	// get host name from url
	// find the first '/'
	int i, n;
	for (i = 0, n = (int) strlen(url); url[i] != '/' && i < n; i++);
	if (i == n)
	{
		// '/' is not found
		fprintf(stderr, "invalid url '%s'\n", url);
		free(url);
		return NULL;
	}

	lfl_string uri = url + i;
	url[i] = 0;
	lfl_string host = url;
	free(url);
	url = NULL;

	if (net_init() == false)
	{
		return NULL;
	}

	m_iface = new net_interface_tcp();

	// Open a socket to receive connections on.
	m_ns = m_iface->connect(host.c_str(), port);

	if (m_ns == NULL)
	{
		fprintf(stderr, "Couldn't open net interface\n");
		delete m_iface;
		m_iface = NULL;
		return NULL;
	}

	// connect to HTTP server
	if (get_proxy_port() > 0)
	{
		char buf[512];
		snprintf(buf, 512, "CONNECT %s:%d HTTP/1.0\r\n", host.c_str(), port);
		m_ns->write_string(buf, HTTP_TIMEOUT);
		m_ns->write_string("User-Agent:gameswf\r\n", HTTP_TIMEOUT);
		m_ns->write_string("Connection:Keep-Alive\r\n", HTTP_TIMEOUT);
		m_ns->write_string("\r\n", HTTP_TIMEOUT);
		if (read_response() == false)
		{
			fprintf(stderr, "Couldn't connect to '%s' through proxy '%s'\n", 
				host.c_str(), get_proxy());
			delete m_ns;
			m_ns = NULL;
			return NULL;
		}
	}

	// We use HTTP/1.0 because we do not wont get chunked encoding data
	m_ns->write_string(lfl_string("GET ") + uri + lfl_string(" HTTP/1.0\r\n"), HTTP_TIMEOUT);
	m_ns->write_string(lfl_string("Host:") + host + lfl_string("\r\n"), HTTP_TIMEOUT);
	//	m_ns->write_string("Accept:*\r\n", HTTP_TIMEOUT);
	//	m_ns->write_string("Accept-Language: en\r\n", HTTP_TIMEOUT);
	//	m_ns->write_string("Accept-Encoding: gzip, deflate, chunked\r\n", HTTP_TIMEOUT);
	//	m_ns->write_string("Accept-Encoding: *\r\n", HTTP_TIMEOUT);
	//	m_ns->write_string("Proxy-Authenticate:prg\r\n", HTTP_TIMEOUT);
	//	m_ns->write_string("Proxy-Authorization:123\r\n", HTTP_TIMEOUT);
	m_ns->write_string("User-Agent:gameswf\r\n", HTTP_TIMEOUT);
	m_ns->write_string("Connection:Close\r\n", HTTP_TIMEOUT);

	m_ns->write_string("\r\n", HTTP_TIMEOUT);

	if (read_response() == false)
	{
		fprintf(stderr, "Couldn't find resource '%s', host '%s'\n", 
			uri.c_str(), host.c_str());
		delete m_ns;
		m_ns = NULL;
		return NULL;
	}

	return m_ns;
}

// read http response
bool http_connection::read_response()
{
	bool rc = false;
	while (true)
	{
		lfl_string s;
		int n = m_ns->read_line(&s, 256, HTTP_TIMEOUT);

		// end of header
		if (n <= 2)
		{
			break;
		}

		// set retcode
		if (strncmp(s.c_str(), "HTTP/", 5) == 0)
		{
			if (strncmp(s.c_str() + 9, "200", 3) == 0)
			{
				rc = true;
			}
		}
	}
	return rc;
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
