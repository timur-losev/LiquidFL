// tu_net_file.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>, Thatcher Ulrich <tu@tulrich.com> 2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// An implementation of tu_file that can open things over the net
// using HTTP.

#include "net/tu_net_file.h"
#include "net/http_client.h"


//
// tu_file functions using a HTTP connection
//

struct netfile
{
	int m_position;
	http_connection* m_nc;
	bool m_eof;

	// We use set_position(back) therefore we need to use buf
	// to store input data
	Uint8* m_buf;		// data
	int m_bufsize;	// current bufsize
	int m_size;			// data size

	netfile(const char* url) :
		m_position(0),
		m_eof(false),
		m_buf(NULL),
		m_bufsize(0),
		m_size(0)
	{
		m_nc = new http_connection();
		m_nc->connect(url, HTTP_SERVER_PORT);
		if (m_nc->is_open() == false)
		{
			delete m_nc;
			m_nc = NULL;
		}
	}

	~netfile()
	{
		delete m_nc;
		m_nc = NULL;
		free(m_buf);
		m_buf = NULL;
	}

	int read(void* dst, int bytes) 
	{
		assert(dst);

		if (m_nc == NULL)
		{
			return 0;
		}

		// ensure buf
		while (m_bufsize < m_position + bytes)
		{
			m_bufsize += 4096;
			m_buf = (Uint8*) realloc(m_buf, m_bufsize);
		}

		// not enough data in the buffer
		if (m_position + bytes > m_size)
		{
			int n = m_nc->read(m_buf + m_size, m_position + bytes - m_size);
			m_size += n;
		}

		int n = imin(bytes, m_size - m_position);
		memcpy(dst, m_buf + m_position, n);
		m_position += n;
		m_eof = n < bytes ? true : false;

		return n;
	}

	int seek(int pos)
	// Return 0 on success, or TU_FILE_SEEK_ERROR on failure.
	{
		if (m_nc == NULL)
		{
			return TU_FILE_SEEK_ERROR;
		}

		if (pos < m_position)
		{
			m_position = pos;
			m_eof = false;
		}
		else
		{
			int n = 1;
			while (m_position < pos && n == 1)
			{
				Uint8 b;
				n = read(&b, 1);
			}

		}
		return m_position == pos ? TU_FILE_NO_ERROR : TU_FILE_SEEK_ERROR;
	}
};


static int http_read(void* dst, int bytes, void* appdata) 
// Return the number of bytes actually read.  EOF or an error would
// cause that to not be equal to "bytes".
{
	netfile* nf = (netfile*) appdata;
	return nf->read(dst, bytes);
}

static int http_tell(const void *appdata)
{
	assert(appdata);
	netfile* nf = (netfile*) appdata;
	return nf->m_position;
}

static int http_close(void* appdata)
{
	assert(appdata);
	netfile* nf = (netfile*) appdata;
	delete nf;
	return 0;
}

static int http_write(const void* src, int bytes, void* appdata)
// Return the number of bytes actually written.
{
	// todo
	assert(0);
	assert(appdata);
	assert(src);
	return 0;
}

static int http_seek(int pos, void *appdata)
// Return 0 on success, or TU_FILE_SEEK_ERROR on failure.
{
	netfile* nf = (netfile*) appdata;
	return nf->seek(pos);
}

static int http_seek_to_end(void *appdata)
{
	assert(appdata);
//	netfile* nf = (netfile*) appdata;

	int n = 0;
	do
	{
		Uint8 b;
		n = http_read(&b, 1, appdata);
	}
	while (n == 1);
	return 0;
}

static bool http_get_eof(void *appdata)
// Return true if we're positioned at the end of the buffer.
{
	assert(appdata);
	netfile* nf = (netfile*) appdata;
	return nf->m_eof;
}

// Create a tu_file that can stream things via HTTP (if the filename
// starts with "http").  Otherwise it behavies like a regular tu_file.
tu_file* new_tu_net_file(const char* filename_or_url, const char* mode)
{
	if (strncasecmp(filename_or_url, "http://", 7) == 0 && mode[0] != 'w') {
		// Create a file from a URL using HTTP protocol
		netfile* nf = new netfile(filename_or_url + 7);
		if (nf->m_nc) {
			tu_file* file = new tu_file(
				nf,
				http_read,
				http_write,
				http_seek,
				http_seek_to_end,
				http_tell,
				http_get_eof,
				http_close);
			return file;
		} else {
			delete nf;
			return NULL;
		}
	} else {
		// Regular tu_file.
		return new tu_file(filename_or_url, mode);
	}
}
