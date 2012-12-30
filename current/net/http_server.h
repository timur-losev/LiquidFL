// http_server.h	-- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Very simple embeddable HTTP server.  For tweaking/monitoring.


#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H


#include "base/tu_types.h"
#include "base/container.h"
#include "net/net_interface.h"


struct http_server;


enum http_status
{
	HTTP_CONTINUE = 100,
	HTTP_OK = 200,
	HTTP_BAD_REQUEST = 400,
	HTTP_UNAUTHORIZED = 401,
	HTTP_FORBIDDEN = 403,
	HTTP_NOT_FOUND = 404,
	HTTP_METHOD_NOT_ALLOWED = 405,
	HTTP_REQUEST_ENTITY_TOO_LARGE = 413,
	HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
	HTTP_INTERNAL_SERVER_ERROR = 500,
	HTTP_NOT_IMPLEMENTED = 501,
	HTTP_HTTP_VERSION_NOT_SUPPORTED = 505,
/*
          | "101"  ; Section 10.1.2: Switching Protocols
          | "201"  ; Section 10.2.2: Created
          | "202"  ; Section 10.2.3: Accepted
          | "203"  ; Section 10.2.4: Non-Authoritative Information
          | "204"  ; Section 10.2.5: No Content
          | "205"  ; Section 10.2.6: Reset Content
          | "206"  ; Section 10.2.7: Partial Content
          | "300"  ; Section 10.3.1: Multiple Choices
          | "301"  ; Section 10.3.2: Moved Permanently
          | "302"  ; Section 10.3.3: Found
          | "303"  ; Section 10.3.4: See Other
          | "304"  ; Section 10.3.5: Not Modified
          | "305"  ; Section 10.3.6: Use Proxy
          | "307"  ; Section 10.3.8: Temporary Redirect
          | "402"  ; Section 10.4.3: Payment Required
          | "406"  ; Section 10.4.7: Not Acceptable

          | "407"  ; Section 10.4.8: Proxy Authentication Required
          | "408"  ; Section 10.4.9: Request Time-out
          | "409"  ; Section 10.4.10: Conflict
          | "410"  ; Section 10.4.11: Gone
          | "411"  ; Section 10.4.12: Length Required
          | "412"  ; Section 10.4.13: Precondition Failed
          | "414"  ; Section 10.4.15: Request-URI Too Large
          | "416"  ; Section 10.4.17: Requested range not satisfiable
          | "417"  ; Section 10.4.18: Expectation Failed
          | "500"  ; Section 10.5.1: Internal Server Error
          | "502"  ; Section 10.5.3: Bad Gateway
          | "503"  ; Section 10.5.4: Service Unavailable
          | "504"  ; Section 10.5.5: Gateway Time-out
*/
};


struct http_request
{
	net_socket* m_sock;  // I own *m_sock

	// I'm making no attempt to be efficient here.
	http_status m_status;
	int m_http_version_x256;  // HTTP 1.1 == 0x0101
	tu_string m_method;
	tu_string m_uri;
	// pre-parsed elements of the request.
	tu_string m_path;
	tu_string m_file;
	// A param can have more than one value (canonical example is
	// a set of checkboxes in a form).
	string_hash<array<tu_string>* > m_param;
	string_hash<tu_string> m_header;

	tu_string m_body;

	http_request()
		:
		m_sock(NULL),
		m_status(HTTP_OK),
		m_http_version_x256(0x0101)
	{
	}

	~http_request() { deactivate(); }

	void deactivate()
	{
		if (m_sock)
		{
			delete m_sock;
			m_sock = NULL;
		}

		clear();
	}

	void clear()
	// Clear everything, but not our socket.
	{
		m_status = HTTP_OK;
		m_http_version_x256 = 0x0101;
		m_method.clear();
		m_uri.clear();
		m_path.clear();
		m_file.clear();
		for (string_hash<array<tu_string>* >::iterator it = m_param.begin(); it != m_param.end(); ++it) {
			delete it->second;
		}
		m_param.clear();
		m_header.clear();
		m_body.clear();
	}

	void add_param(const tu_string& key, const tu_string& value)
	// Helper.  Adds a value under the given key.
	{
		array<tu_string>* values;
		if (m_param.get(key, &values) == false) {
			values = new array<tu_string>;
			m_param.add(key, values);
		}
		values->push_back(value);
	}

	int param_count(const tu_string& key)
	// Return the number of values for the given param.
	{
		array<tu_string>* values;
		if (m_param.get(key, &values)) {
			return values->size();
		}

		return 0;
	}

	const tu_string& get_param(const tu_string& key, int index)
	// Returns the index'th value of the specified key.
	{
		array<tu_string>* values;
		if (m_param.get(key, &values)) {
			return (*values)[index];
		}

		assert(0);
		static tu_string dummy;
		return dummy;
	}

	// Utility.  Append info to the given string.
	void dump_html(tu_string* out);
};


struct http_handler
// Interface for code that wants to service http requests.
{
	// The server calls this method on your handler in order to
	// respond to a request.
	//
	// server is the http_server that is calling us.  key is the
	// substring of the request object that matched us.  req has
	// all the details of the actual request.
	virtual void handle_request(http_server* server, const tu_string& key, http_request* req) = 0;
};


struct http_server
// Server class.  Listens to a socket, and depending on the query,
// dispatches requests to various registered handlers.
{
	// Takes ownership of *net.
	http_server(net_interface* net);

	~http_server();
	
	// Register a handler for servicing requests on a particular
	// path.  E.g. if you want to handle requests of the form
	//
	//   "http://localhost:54321/path/to/something/my_request?a=1&b=2"
	// 
	// you pass
	//
	//   "/path/to/something"
	//
	// to this function.  Your handler gets all the request info,
	// so it can serve it any way it likes.
	//
	// Does not take ownership of *handler.
	//
	// Returns false if it can't attach the handler (i.e. another
	// handler is already attached to that path).
	bool add_handler(const char* path, http_handler* handler);
	
	// Use this when you want to delete your handler.
	void remove_handler(http_handler* handler);

	void update();

	// Helpers for handlers.
	
	// Simple known-length response.  content_type can be things
	// such as "text/html", "text/plain", "image/jpeg", etc.
	void send_response(http_request* req, const char* content_type, const void* data, int len);
	void send_response(http_request* req, const char* content_type, const tu_string& body)
	{
		send_response(req, content_type, body.c_str(), body.length());
	}
	void send_html_response(http_request* req, const tu_string& body)
	{
		send_response(req, "text/html", body);
	}
	void send_text_response(http_request* req, const tu_string& body)
	{
		send_response(req, "text/plain", body);
	}

	// For a response of unknown length.  Do begin, append+, end.
	// Uses chunked transfer coding.
	void begin_response(http_request* req, const char* content_type);
	void append_response(http_request* req, const tu_string& data);
	void end_response(http_request* req);
	
	// Mainly for internal use, but may be useful sometimes externally.
	//
	// Return a response for the given fully-parsed request.  The
	// request contains the net_socket to reply on.
	void dispatch_request(http_request* req);
	
	// For testing only.
	static void parse_test();

private:
	
//data:
	net_interface* m_net;

	// Request handlers.
	string_hash<http_handler*> m_handlers;
	
	// State for managing a single request.
	struct http_request_state
	{
		http_request m_req;
		
		// State for parsing.
		tu_string m_line_buffer;
		tu_string m_parse_key;
		tu_string m_parse_value;

		enum request_state
		{
			PARSE_START_LINE,
			PARSE_HEADER,
			PARSE_BODY_IDENTITY,
			PARSE_BODY_CHUNKED_CHUNK,
			PARSE_BODY_CHUNKED_TRAILER,
			PARSE_DONE,
			IDLE,
		};
		request_state m_request_state;
		int m_content_length;

		uint32 m_last_activity;

		http_request_state()
			:
			m_request_state(PARSE_START_LINE),
			m_content_length(-1),
			m_last_activity(0)
		{}

		~http_request_state()
		{
			deactivate();
		}
		
		void activate(net_socket* sock);
		void deactivate();

		void clear();

		bool is_alive() const
		{
			return m_req.m_sock != NULL;
		}
		bool is_pending() const { return m_request_state != IDLE; }
		void update(http_server* server);

		http_status parse_message_line(const char* line);

		// Helpers for parse_message_line().
		http_status parse_request_line(const char* reqline);
		http_status parse_header_line(const char* hline);
		http_status process_header();
		http_status parse_body();
		void parse_query_string(const char* query);
	};
	array<http_request_state*> m_state;
};


#endif // HTTP_SERVER_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
