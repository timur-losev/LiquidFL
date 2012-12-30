// net_interface.h	-- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Abstract interfaces for network connections.


#ifndef NET_INTERFACE_H
#define NET_INTERFACE_H

#include "base/tu_types.h"
#include "base/container.h"
#include <string.h>


class tu_file;
struct net_socket;
class lfl_string;


struct net_interface
// Abstract network interface.  This is a wrapper around a socket
// library, or what-have-you.
{
	virtual ~net_interface() {}

	// Returns a net_socket* if somebody has opened an incoming
	// connection.
	//
	// Returns NULL if no incoming connection is pending.
	virtual net_socket* accept() = 0;

	// create new client connection
	virtual net_socket* connect(const char* c_url, int port) = 0; 
};


struct net_socket
// Abstract IO stream interface.
{
	virtual ~net_socket() {}

	// 0 if we're OK; non-zero for error.
	virtual int get_error() const = 0;

	virtual bool is_open() const = 0;

	// Returns true if there's data available to read.
	virtual bool is_readable() const = 0;
	
	// Returns bytes read.
	//
	// May return early if all the bytes aren't available within
	// timeout_ms milliseconds.
	//
	// If timeout_seconds is 0, blocks indefinitely waiting for data.
	virtual int read(void* data, int bytes, float timeout_seconds) = 0;

	// Like read(), but stops reading after it hits a '\n'.
	//
	// *data need not be empty -- this function will append in
	// that case.
	virtual int read_line(lfl_string* data, int maxbytes, float timeout_seconds) = 0;

	// Returns bytes written.
	//
	// Blocks until all the data is written, there is some socket
	// error, or timeout_seconds is exceeded.
	virtual int write(const void* data, int bytes, float timeout_seconds) = 0;

	int write_string(const char* str, float timeout_seconds)
	{
		return write(str, (int) strlen(str), timeout_seconds);
	}

	int write_string(const lfl_string& str, float timeout_seconds)
	{
		return write(str.c_str(), str.length(), timeout_seconds);
	}
};

// Factories for some concrete net_interfaces.

// TCP server interface.
exported_module net_interface* tu_create_net_interface_tcp(int port_number);

// Handy for testing.
//
// Either arg can be NULL, to have no input or discard output.
exported_module net_interface* tu_create_net_interface_file(tu_file* in, tu_file* out);


#endif // NET_INTERFACE_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
