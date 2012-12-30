// tu_net_file.h	-- Thatcher Ulrich <tu@tulrich.com> 2007 -*- coding: utf-8;-*-

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// An implementation of tu_file that can open things over the net
// using HTTP.

#ifndef TU_NET_FILE_H
#define TU_NET_FILE_H

#include "base/tu_file.h"
#include "base/container.h"

exported_module tu_file* new_tu_net_file(const char* filename_or_url, const char* mode);

#endif // TU_NET_FILE_H

