// http_file_handler.h	-- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// HTTP request handler that serves static files.


#ifndef HTTP_FILE_HANDLER_H
#define HTTP_FILE_HANDLER_H


#include "net/http_server.h"
#include "base/container.h"


struct http_file_handler : public http_handler
{
	// file_basepath is the location on the local file system to
	// get the source files.  No trailing slash.
	//
	// http_basepath is the URI base path corresponding to the
	// file_basepath.
	//
	// i.e. if the http request is
	// "http:/some.server/path/to/some/file.html" and the
	// http_basepath is "/path/to", and the file_basepath is
	// "c:/some/other/path", then we try to serve
	// "c:/some/other/path/some/file.html".
	//
	// Mime type is inferred from the file extension, based on a
	// few common file extensions.
	http_file_handler(const char* file_basepath, const char* http_basepath);
	// enum serve_embedded_files { SERVE_EMBEDDED_FILES };
	// http_file_handler(serve_embedded_files s, const char* http_basepath);
	~http_file_handler();

	const char* http_basepath() const { return m_http_basepath.c_str(); }

	virtual void handle_request(http_server* server, const tu_string& key, http_request* req);

private:
	tu_string m_file_basepath;
	tu_string m_http_basepath;
};


#endif // HTTP_FILE_HANDLER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
