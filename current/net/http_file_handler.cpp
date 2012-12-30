// http_file_handler.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// HTTP handler that serves static files.


#include "base/file_util.h"
#include "base/membuf.h"
#include "base/tu_file.h"
#include "net/http_file_handler.h"


http_file_handler::http_file_handler(const char* file_basepath, const char* http_basepath)
	:
	m_file_basepath(file_basepath),
	m_http_basepath(http_basepath)
{
}

http_file_handler::~http_file_handler()
{
}


static const char* lookup_content_type(const char* file_extension)
// Given a file extension, determine the HTTP content type to return.
{
	static stringi_hash<const char*> s_types;
	static bool s_inited = false;
	if (!s_inited) {
		s_inited = true;

		// A small table of well-known content types.
		struct type_pair {
			const char* extension;
			const char* type;
		} types[] = {
			{ "txt", "text/plain" },
			{ "html", "text/html" },
			{ "htm", "text/html" },
			{ "jpg", "image/jpeg" },
			{ "jpeg", "image/jpeg" },
			{ "gif", "image/gif" },
			{ "png", "image/png" },
			{ "js", "text/javascript" },
			// etc.
		};

		// Register the types.
		for (int i = 0, n = TU_ARRAYSIZE(types); i < n; i++) {
			s_types.add(tu_stringi(types[i].extension), types[i].type);
		}
	}

	const char* type = NULL;
	if (s_types.get(tu_stringi(file_extension), &type)) {
		return type;
	}

	// No known type for this file extension.
	return "application/octet-stream";
}


void http_file_handler::handle_request(http_server* server, const tu_string& key, http_request* req)
// Serve a file, if we have it; otherwise make an error response.
{
	if (strncmp(m_http_basepath.c_str(), req->m_path.c_str(), m_http_basepath.length()) != 0) {
		// URI path doesn't start with the http base path, so
		// we can't make a valid local file path.
		//
		// This is really an error in the handler
		// configuration.  The http_file_handler should be
		// registered in the http_server under the
		// http_basepath.
		req->m_status = HTTP_NOT_FOUND;
		tu_string error_string = string_printf("Error 404 -- %s does not match expected basepath %s",
						       req->m_path.c_str(), m_http_basepath.c_str());
		server->send_text_response(req, error_string);
		return;
	}

	if (strstr(req->m_path.c_str(), "..")) {
		// Deny any path hanky-panky.
		req->m_status = HTTP_BAD_REQUEST;
		tu_string error_string = "Error 400 - bad request";
		server->send_text_response(req, error_string);
		return;
	}

	// Form the local filename.
	tu_string local_path = m_file_basepath;
	local_path += (req->m_uri.c_str() + m_http_basepath.length());

	tu_file in(local_path.c_str(), "rb");
	if (in.get_error()) {
		// 404
		req->m_status = HTTP_NOT_FOUND;
		server->send_text_response(req, "Error 404 - not found");
		return;
	}

	// slurp in the contents of *in
	membuf contents;
	in.copy_to(&contents);

	// Determine the content type.
	const char* file_extension = file_util::get_extension(local_path.c_str());
	const char* content_type = lookup_content_type(file_extension);

	// Send the file data.
	server->send_response(req, content_type, contents.data(), contents.size());

	// destructors should handle the cleanup of in & contents.
}
