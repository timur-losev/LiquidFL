// http_server.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Very simple HTTP server.  Intended for embedding in games or other
// apps, for monitoring/tweaking.
//
// HTTP 1.1 protocol defined here:
// http://www.w3.org/Protocols/rfc2616/rfc2616.html
//
// "EHS - Embedded HTTP Server" http://xaxxon.slackworks.com/EHS.html
// has very similar goals.  Unfortunately it's LGPL.


#include "net/http_server.h"
#include "net/net_interface.h"
#include "base/tu_timer.h"
#include "base/logger.h"


void http_request::dump_html(tu_string* outptr)
// Debug helper.
//
// Append our contents into the given string, in HTML format.
{
	tu_string& out = *outptr;
	
	out += "Request info:<br>\n";
	out += "status: ";
	char buf[200];
	sprintf(buf, "%d<br>\n", int(m_status));
	out += buf;
	out += "method: ";
	out += m_method;
	out += "<br>\n";
	out += "uri: ";
	out += m_uri;
	out += "<br>\n";
	out += "path: ";
	out += m_path;
	out += "<br>\n";
	out += "file: ";
	out += m_file;
	out += "<br>\n";

	out += "params:<br><ul>\n";
	for (string_hash<array<tu_string>* >::iterator it = m_param.begin(); it != m_param.end(); ++it)
	{
		for (int i = 0, n = it->second->size(); i < n; i++) {
			out += "<li><b>";
			out += it->first;
			out += "</b> - ";
			out += (*it->second)[i];
			out += "</li>\n";
		}
	}
	out += "</ul><br>\n";
	
	out += "headers:<br><ul>\n";
	for (string_hash<tu_string>::iterator it = m_header.begin(); it != m_header.end(); ++it)
	{
		out += "<li><b>";
		out += it->first;
		out += "</b> - ";
		out += it->second;
		out += "</li>\n";
	}
	out += "</ul><br>\n";
	
	out += "body: <br><pre>";
	out += m_body;
	out += "\n</pre><br>";
}



http_server::http_server(net_interface* net)
	:
	m_net(net)
{
}


http_server::~http_server()
{
	for (int i = 0; i < m_state.size(); i++) {
		delete m_state[i];
	}
	m_state.resize(0);

	if (m_net)
	{
		delete m_net;
		m_net = NULL;
	}
}


bool http_server::add_handler(const char* path, http_handler* handler)
// Hook a handler, return true on success.
{
	if (m_handlers.get(path, NULL))
	{
		// Already linked this one!
		return false;
	}

	m_handlers.add(path, handler);
	return true;
}


void http_server::update()
// Call this periodically to serve pending requests.
{
	// if (active sockets < 10 or so)
	// Check for new connections.
	net_socket* sock = m_net->accept();
	if (sock) {
		// Find or create a state for this socket.
		int use_state = -1;
		for (int i = 0; i < m_state.size(); i++) {
			if (!m_state[i]->is_alive()) {
				// Re-use this state.
				use_state = i;
				break;
			}
		}
		if (use_state == -1) {
			use_state = m_state.size();
			m_state.push_back(new http_request_state);
		}
		m_state[use_state]->activate(sock);
		printf("accepted new socket on state %d\n", use_state);//xxxxxx
	}
	
	// Update any active requests.
	static const float UPDATE_TIMEOUT_SECONDS = 0.100f;
	uint32 start = tu_timer::get_ticks();
	uint32 last_activity_ticks = start;
	for (;;) {
		int active_requests = 0;
		
		for (int i = 0; i < m_state.size(); i++) {
			if (!m_state[i]->is_alive()) {
				continue;
			}
			m_state[i]->update(this);

			if (m_state[i]->is_alive() == false || m_state[i]->is_pending() == false) {
				continue;
			}

			active_requests++;
			last_activity_ticks = tu_timer::get_ticks();
		}

		if (!active_requests) {
			break;
		}
		
		// Even when there are pending requests, don't block
		// for very long.
		uint32 now = tu_timer::get_ticks();
		if ((now - start) / 1000.0f >= UPDATE_TIMEOUT_SECONDS)
		{
			printf("state long timed out ****\n");//xxxxxx
			break;
		}
		tu_timer::sleep(0);
	}
}


void http_server::send_response(http_request* req, const char* content_type, const void* data, int len)
// Send a known-length response.
{
	tu_string header = string_printf(
		"HTTP/1.1 %d %s\r\nContent-length: %d\r\n"
		"Content-type: %s\r\n"
		"\r\n"
		,
		int(req->m_status),
		req->m_status >= 400 ? "ERROR" : "OK",
		len,
		content_type);
	req->m_sock->write(header.c_str(), header.length(), 0.010f);
	req->m_sock->write(data, len, 0.010f);

	VLOG("send_response: \n%s\n", header.c_str());
}


void http_server::dispatch_request(http_request* req)
// Return a response for the given request.
{
	assert(req);
	assert(req->m_sock);

	VLOG("dispatch_request: \n%s\n", req);
	
	if (! req->m_sock->is_open())
	{
		// Can't respond, just ignore the request.
		return;
	}

	if (req->m_status >= 400)
	{
		// Minimal error response.
		tu_string error_body = string_printf(
			"<html><head><title>Error %d</title></head><body>Error %d</body></html>", req->m_status, req->m_status);
		send_html_response(req, error_body);

		return;
	}

	// Look at the request, and dispatch it to the appropriate
	// handler.

	// Try the paths to see if there's a corresponding handler, in
	// order from most specific to least specific.
	const char* path = req->m_path.c_str();
	const char* last_slash = path + req->m_path.length();
	for (;;)
	{
		// Key is a subset of the path.
		tu_string key(path, last_slash - path);

		http_handler* handler(NULL);
		if (m_handlers.get(key, &handler))
		{
			// Dispatch.
			handler->handle_request(this, key, req);
			return;
		}
		
		// Shrink the key.
		const char* next_last_slash_in_key = strrchr(key.c_str(), '/');
		if (next_last_slash_in_key == NULL)
		{
			// Done checking subkeys of this path.
			break;
		}

		int slash_index = next_last_slash_in_key - key.c_str();
		last_slash = path + slash_index;
	}

	// No handler found for this request, return error.
	req->m_status = HTTP_NOT_FOUND;
	tu_string error_string = "<html><head><title>Error 404</title></head>"
		"<body>Error 404 - No handler for object</body></html>";
	send_html_response(req, error_string);
}


void http_server::http_request_state::update(http_server* server)
// Continues processing the request.  Deactivates our state when the
// request is finished.
{
	if (! is_alive()) {
		return;
	}

	int bytes_in;

	static const int MAX_LINE_BYTES = 32768;  // very generous, but not insane, max line size.
	static const float CONNECTION_TIMEOUT = 300.0f;  // How long to leave the socket open.

	if (m_request_state == IDLE) {
		// Watch for the start of a new request.
		if (m_req.m_sock->is_readable() == false) {
			uint32 now = tu_timer::get_ticks();
			if ((now - m_last_activity) / 1000.0f > CONNECTION_TIMEOUT) {
				// Timed out; close the socket.
				deactivate();
				printf("socket timed out, deactivating.\n");//xxxxxx
			}

			// Idle.
			return;
		} else {
			// We have some data on the socket; start
			// parsing it.
			m_request_state = PARSE_START_LINE;
			m_last_activity = tu_timer::get_ticks();

			// Fall through and start processing.
			//printf("socket is now readable\n");//xxxx
		}
	}

	if (m_req.m_sock->is_open() == false) {
		// The connection closed on us -- abort the current request!
		deactivate();
		printf("socket closed, deactivating.\n");//xxxxxx
		return;
	}

	switch (m_request_state)
	{
	default:
		// Invalid state.
		assert(0);
		deactivate();
		break;

	case PARSE_START_LINE:
	case PARSE_HEADER:
		// wait for a whole line, parse it.
		bytes_in = m_req.m_sock->read_line(&m_line_buffer, MAX_LINE_BYTES - m_line_buffer.length(), 0.010f);
		if (m_line_buffer.length() >= 2 && m_line_buffer[m_line_buffer.length() - 1] == '\n')
		{
			//printf("req got header line: %s", m_line_buffer.c_str());//xxxxxx
			
			// We have the whole line.  Parse and continue.
			m_req.m_status = parse_message_line(m_line_buffer.c_str());
			if (m_req.m_status >= 400)
			{
				m_line_buffer.clear();
				m_request_state = PARSE_DONE;
			}
			// else we're either still in the header, or
			// process_header changed our parse state.

			m_line_buffer.clear();
		}
		else if (m_line_buffer.length() >= MAX_LINE_BYTES)
		{
			printf("req invalid header line length\n");//xxxxxx
			
			// Invalid line.
			m_line_buffer.clear();
			m_req.m_status = HTTP_BAD_REQUEST;
			m_request_state = PARSE_DONE;
		}
		break;

	case PARSE_BODY_IDENTITY:
	case PARSE_BODY_CHUNKED_CHUNK:
	case PARSE_BODY_CHUNKED_TRAILER:
		m_req.m_status = parse_body();
		if (m_req.m_status >= 400)
		{
			// Something bad happened.
			m_request_state = PARSE_DONE;
		}

		break;

	case PARSE_DONE:
		// Respond to the request.
		server->dispatch_request(&m_req);

		// Leave the connection open, but go idle, waiting for
		// another request.
		m_request_state = IDLE;
		m_last_activity = tu_timer::get_ticks();
		clear();
		break;
	}
}


void http_server::http_request_state::activate(net_socket* sock)
// Initialize to handle a request coming in on the given socket.
{
	VLOG("hrs::activate(%x)", sock);

	deactivate();

	// Adopt the given socket.
	assert(m_req.m_sock == NULL);
	m_req.m_sock = sock;

	m_last_activity = tu_timer::get_ticks();
}


void http_server::http_request_state::deactivate()
// Clear request state & close the socket.
{
	VLOG("hrs::deactivate(), sock = %x\n", m_req.m_sock);

	clear();
	m_req.deactivate();
}

void http_server::http_request_state::clear()
// Clear out request state, but don't mess with our socket.
{
	VLOG("hrs::clear(), sock = %x\n", m_req.m_sock);

	m_req.clear();
	m_line_buffer.clear();
	m_parse_key.clear();
	m_parse_value.clear();
	m_request_state = IDLE;
	m_content_length = -1;
}


bool is_linear_whitepace(char c)
{
	return c == ' ' || c == '\t';
}


bool is_whitespace(char c)
{
	return is_linear_whitepace(c) || c == '\r' || c == '\n';
}


tu_string trim_whitespace(const char* buf)
// Return the arg with whitespace at either end trimmed off.
{
	const char* c = buf;
	while (is_whitespace(c[0]))
	{
		c++;
	}
	int end = strlen(c);
	while (end > 0 && is_whitespace(c[end - 1]))
	{
		end--;
	}
	
	return tu_string(c, end);
}


http_status http_server::http_request_state::parse_message_line(const char* line)
// Parse the next line.
{
	VLOG("parse_message_line: '%s'\n", line);
	http_status status = HTTP_BAD_REQUEST;
	
	switch (m_request_state)
	{
	default:
	case PARSE_DONE:
		assert(0);
		break;
	case PARSE_START_LINE:
		status = parse_request_line(line);
		if (status < 400)
		{
			// Move on to the rest of the header.
			m_request_state = PARSE_HEADER;
		}
		break;
	case PARSE_HEADER:
		status = parse_header_line(line);
		break;
	}

	return status;
}


http_status http_server::http_request_state::parse_request_line(const char* reqline)
// Parses the first line of an HTTP request.  Fills in fields in m_req
// appropriately.  Returns HTTP result code, 400+ if we detected an
// error, HTTP_OK if the parse is OK.
{
	VLOG("parse_request_line(), sock = %x: %s\n", m_req.m_sock, reqline);

	// Method.
	const char* first_space = strchr(reqline, ' ');
	if (first_space == NULL || first_space == reqline)
	{
		// No method.
		return HTTP_BAD_REQUEST;
	}
	m_req.m_method = tu_string(reqline, first_space - reqline);

	// URI.
	const char* second_space = strchr(first_space + 1, ' ');
	if (second_space == NULL || second_space == first_space + 1)
	{
		// No URI.
		return HTTP_BAD_REQUEST;
	}
	m_req.m_uri = tu_string(first_space + 1, second_space - first_space - 1);

	// HTTP version.
	const char* version = second_space + 1;
	if (strncmp(version, "HTTP/1.0", 8) == 0)
	{
		m_req.m_http_version_x256 = 0x0100;
	}
	else if (strncmp(version, "HTTP/1.1", 8) == 0)
	{
		m_req.m_http_version_x256 = 0x0101;
	}
	else
	{
		return HTTP_HTTP_VERSION_NOT_SUPPORTED;
	}

	return HTTP_OK;  // OK so far.
}


http_status http_server::http_request_state::parse_header_line(const char* hline)
// Parse a header line.  Call this multiple lines, once with each
// header line, to build up m_req.
//
// Sets m_request_state to PARSE_BODY if hline is the last header line.
//
// Returns an error code >= 400 on error; < 400 on success.
{
	assert(m_request_state == PARSE_HEADER);

	if (hline[0] == '\r' && hline[1] == '\n')
	{
		// End of the header.
		
		// Finish the previous key.
		if (m_parse_key.length())
		{
			m_req.m_header.set(m_parse_key, m_parse_value);
		}

		return process_header();
	}

	if (is_linear_whitepace(hline[0]))
	{
		// Looks like a continuation.
		if (m_parse_key.length())
		{
			m_parse_value += " ";
			m_parse_value += trim_whitespace(hline);
			return HTTP_OK;
		}
		else
		{
			// Non-continuation line starts with whitespace.
			return HTTP_BAD_REQUEST;
		}
	}
	
	// Finish any previous key.
	if (m_parse_key.length())
	{
		m_req.m_header.set(m_parse_key, m_parse_value);
		m_parse_key = "";
		m_parse_value = "";
	}

	const char* first_colon = strchr(hline, ':');
	if (first_colon == NULL || first_colon == hline)
	{
		return HTTP_BAD_REQUEST;
	}

	// Start a key/value pair.
	m_parse_key = tu_string(hline, first_colon - hline).utf8_to_lower();

	// Fill in any value-so-far from an earlier occurrence of this header.
	assert(m_parse_value.length() == 0);
	m_req.m_header.get(m_parse_key, &m_parse_value);
	if (m_parse_value.length())
	{
		m_parse_value += ",";
	}
	
	m_parse_value += trim_whitespace(first_colon + 1);

	return HTTP_OK;
}


static const int MAX_CONTENT_LENGTH_ACCEPTED = 32768;


int hex_to_int(char c)
// Convert a hex digit ([0-9] or [a-fA-F]) to an integer [0,15].
// Return -1 if the character is not a hex digit.
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'a' && c <= 'f') {
		return 10 + (c - 'a');
	} else if (c >= 'A' && c <= 'F') {
		return 10 + (c - 'A');
	}

	return -1;
}


static void unescape_url_component(tu_string* str)
// Do URL unescaping on a URL component.  I.e. convert the hex code
// "%xx" to the corresponding byte, and change '+' to ' '.
{
	tu_string out;
	for (int i = 0; i < str->length(); i++) {
		char c = (*str)[i];
		if (c == '%'
		    && i + 2 < str->length()) {
			// Interpret the next two chars as hex digits.
			char digit_hi = (*str)[i + 1];
			char digit_lo = (*str)[i + 2];
			int val_hi = hex_to_int(digit_hi);
			int val_lo = hex_to_int(digit_lo);
			if (val_hi < 0 || val_lo < 0) {
				// Invalid hex digits.  Pass the '%'
				// straight through and don't consume the
				// two following chars.
				out += c;
			} else {
				char encoded = (val_hi << 4) | (val_lo);
				if (encoded > 0) {
					out += encoded;
				}
				i += 2;
			}
		} else if (c == '+') {
			// '+' is turned into a space.
			out += ' ';
		} else {
			// Pass this character straight through.
			out += c;
		}
	}

	*str = out;
}


void http_server::http_request_state::parse_query_string(const char* query)
// Parses a URL-encoded query string.  Puts decoded param/value pairs
// into m_req.m_params.
{
	while (query) {
		const char* equals = strchr(query, '=');
		tu_string param;
		tu_string value;
		if (equals) {
			param = tu_string(query, equals - query);
			
			// Find the end of the value and update the query.
			query = strchr(query, '&');
			if (query) {
				value = tu_string(equals + 1, query - (equals + 1));
				query++;  // Skip the '&'
			} else {
				value = equals + 1;
			}
		} else {
			const char* next_query = strchr(query, '&');
			if (next_query) {
				param = tu_string(query, next_query - query);
				next_query++;  // Skip the '&'
			} else {
				param = query;
			}
			
			query = next_query;
		}

		unescape_url_component(&param);
		unescape_url_component(&value);

		if (param.length()) {
			m_req.add_param(param, value);
		}
	}
}


http_status http_server::http_request_state::process_header()
// Call this after finishing parsing the header.  Sets the following
// parse state.  Returns an HTTP status code.
{
	assert(m_request_state == PARSE_HEADER);
	assert(m_req.m_body.length() == 0);

	// Set up path/file vars.
	const char* pathend = strchr(m_req.m_uri.c_str(), '?');
	const char* query = NULL;
	if (pathend) {
		query = pathend + 1;
	} else {
		pathend = m_req.m_uri.c_str() + m_req.m_uri.length();
	}
	m_req.m_path = tu_string(m_req.m_uri.c_str(), pathend - m_req.m_uri.c_str());
	unescape_url_component(&m_req.m_path);
	
	const char* filestart = strrchr(m_req.m_path.c_str(), '/');
	if (filestart)
	{
		m_req.m_file = (filestart + 1);
		unescape_url_component(&m_req.m_file);
	}

	// Parse params in the request string.
	parse_query_string(query);

	m_content_length = -1;
	tu_string content_length_str;
	if (m_req.m_header.get("content-length", &content_length_str))
	{
		m_content_length = atol(content_length_str.c_str());
		if (m_content_length > MAX_CONTENT_LENGTH_ACCEPTED)
		{
			m_request_state = PARSE_DONE;
			return HTTP_REQUEST_ENTITY_TOO_LARGE;
		}
	}
	
	tu_string transfer_encoding;
	if (m_req.m_header.get("transfer-encoding", &transfer_encoding))
	{
		if (transfer_encoding.to_tu_stringi() == "chunked")
		{
			// We're required to ignore the content-length
			// header.
			m_content_length = -1;
			m_request_state = PARSE_BODY_CHUNKED_CHUNK;
			return parse_body();
		}
		else if (transfer_encoding.to_tu_stringi() == "identity")
		{
			// This is OK; basically a no-op.
		}
		else
		{
			// A transfer encoding we don't know how to handle.
			// Reject it.
			m_request_state = PARSE_DONE;
			return HTTP_NOT_IMPLEMENTED;
		}
	}

	// If there's a body section, then we need to read it & parse it.
	if (m_content_length >= 0)
	{
		m_request_state = PARSE_BODY_IDENTITY;
		return parse_body();
	}
	else
	{
		m_request_state = PARSE_DONE;
	}
	
	return HTTP_OK;
}


http_status http_server::http_request_state::parse_body()
// Read the body section from the socket and fills m_req.m_body.
//
// Sets m_request_state to PARSE_DONE.
//
// Returns an error code >= 400 on error; < 400 on success.
{
	assert(m_req.m_body.length() == 0);

	// Get the body data and stash it in m_req.m_body.
	switch (m_request_state)
	{
	default:
		assert(0);
		break;

	case PARSE_BODY_IDENTITY:
		m_request_state = PARSE_DONE;

		// We assume we can get the whole body within 100 ms.
		m_req.m_body.resize(m_content_length);
		int bytes_read = m_req.m_sock->read(&(m_req.m_body[0]), m_content_length, 0.100f);
		if (bytes_read < m_content_length)
		{
			return HTTP_BAD_REQUEST;
		}
		break;

#if 0
	case PARSE_BODY_CHUNKED_CHUNK:
		read a line;
		get chunk size;
		while (chunk_size > 0)
		{
			read chunk data and CRLF;
			m_body += chunk data;
			read a line;
			get chunk size;
		}

		read entity_header;
		while (entity_header.length() > 0)
		{
			parse entity_header;
			read entity_header;
		}

		m_request_state = PARSE_DONE;

		break;
#endif // 0
	}

	// Parse the body data if necessary.
	tu_string type_str;
	if (m_req.m_method == "POST" && m_req.m_header.get("content-type", &type_str)) {
		if (type_str == "application/x-www-form-urlencoded") {
			// Parse "Content-type: application/x-www-form-urlencoded" in the body
			//
			// Basically this is a long string that looks like:
			//
			//  key1=value1&key2=value2&key3=value3 [...]
			//
			// I.e. just like in the URL.
			//
			// http://www.w3.org/TR/REC-html40/interact/forms.html says
			// it's "the default content type".  Quote from that doc:
			//
			//    1. Control names and values are escaped. Space
			//    characters are replaced by `+', and then reserved
			//    characters are escaped as described in [RFC1738],
			//    section 2.2: Non-alphanumeric characters are replaced by
			//    `%HH', a percent sign and two hexadecimal digits
			//    representing the ASCII code of the character. Line
			//    breaks are represented as "CR LF" pairs (i.e.,
			//    `%0D%0A').
			//
			//    2. The control names/values are listed in the order they
			//    appear in the document. The name is separated from the
			//    value by `=' and name/value pairs are separated from
			//    each other by `&'.
			parse_query_string(m_req.m_body.c_str());
			
		} else if (type_str == "multipart/form-data") {
			// Parse "Content-type: multipart/form-data" in the body
			// (i.e. file uploads, non-ASCII fields, ... -- what else?)
			//   etc...
			//   // each part has a header like: Content-disposition: form-data; name="something"
			//   // each part optionally has a Content-type: header.  The default is "text/plain".
			//   // each part may have a Content-transfer-encoding:, if it differs from the body
			//
			//   // http://www.faqs.org/rfcs/rfc2388.html covers multipart/form-data for HTTP 1.1
			//   // http://www.faqs.org/rfcs/rfc2046.html covers MIME multipart encoding generically
			return HTTP_UNSUPPORTED_MEDIA_TYPE;
		} else {
			return HTTP_UNSUPPORTED_MEDIA_TYPE;
		}
	}

	return HTTP_OK;
}


#ifdef HTTP_SERVER_TEST

// Compile with e.g.:
//
// cl http_server.cpp -Fehttp_server_test.exe -DHTTP_SERVER_TEST -I.. -Od -Zi -MD ../base/base.lib /link /NODEFAULTLIB:msvcprt.lib


/*static*/ void http_server::parse_test()
{
	// Test the parsing of the request line (first line).
	static struct reqline_test_vector
	{
		const char* reqline;
		http_status status;
		const char* method;
		const char* uri;
		int version_x256;
	}
	tests[] =
	{
		{ "GET http://tulrich.com/index.html HTTP/1.1", HTTP_OK, "GET", "http://tulrich.com/index.html", 0x0101 },
		{ "POST /index.html HTTP/1.1", HTTP_OK, "POST", "/index.html", 0x0101 },
		{ "PUT * HTTP/1.0", HTTP_OK, "PUT", "*", 0x0100 },
		{ "OPTIONS http://localhost/ HTTP/1.1", HTTP_OK, "OPTIONS", "http://tulrich.com/", 0x0101 },
		{ "HEAD * HTTP/1.2", HTTP_HTTP_VERSION_NOT_SUPPORTED, "HEAD", "*", 0x0102 },
		{ "nospaces", HTTP_BAD_REQUEST, "", "", 0 },
		{ "one spaces", HTTP_BAD_REQUEST, "", "", 0 },
		{ "too  many spaces", HTTP_BAD_REQUEST, "", "", 0 },
		{ "too many  spaces", HTTP_HTTP_VERSION_NOT_SUPPORTED, "", "", 0 },
	};

	for (int i = 0; i < TU_ARRAYSIZE(tests); i++)
	{
		http_server::http_request_state state;
		int status = state.parse_request_line(tests[i].reqline);
		if (status >= 400)
		{
			// Make sure test is expected to fail.
			assert(tests[i].status >= 400);
			printf("status = %d, expected = %d\n", status, int(tests[i].status));
		}
		else
		{
			assert(state.m_req.m_method == tests[i].method);
			assert(state.m_req.m_uri == tests[i].uri);
			assert(state.m_req.m_http_version_x256 == tests[i].version_x256);

			printf("reqline = %s\n", tests[i].reqline);
			printf("Req method = %s\n", state.m_req.m_method.c_str());
			printf("Req URI = %s\n", state.m_req.m_uri.c_str());
			printf("Req version = 0x%X\n", state.m_req.m_http_version_x256);
		}
		printf("\n");
	}

	// Test parsing of whole header.
	static struct header_test_vector
	{
		const char* headertext;
		http_status status;
		const char* path;
		const char* file;
		const char* arg_list;
		const char* argval_list;
	}
	tests2[] =
	{
		{ "GET http://127.0.0.1/path/to/SomeJunk.html?a=1&b=2 HTTP/1.1\r\n"
		  "Host: 127.0.0.1\r\n"
		  "User-Agent: something or other\r\n"
		  "\r\n"
		  "" // Message body
		  ,
		  HTTP_OK,
		  "path/to",
		  "SomeJunk.html",
		  "a,b",
		  "1,2"
	};
	
}


int main()
{
	http_server::parse_test();

	return 0;
}


#endif // HTTP_SERVER_TEST


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
