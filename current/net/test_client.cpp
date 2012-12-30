// test_client.cpp	-- Vitaly Alexeev <tishka92@yahoo.com>	2007

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Small test program to exercise tu-testbed/net/http_client library.
// net file grubber

#include "base/tu_file.h"
#include "net/tu_net_file.h"

// arg1: source file, like "http://www.my.com/swf/my.swf"
// arg2: dest file, like "c:\my.swf"
int main(int argc, const char** argv)
{
	if (argc < 3)
	{
		printf("2 args is needed\n");
		exit(0);
	}

	tu_file* source = new_tu_net_file(argv[1], "rb");
	if (source)
	{
		tu_file target(argv[2], "wb"); 
		target.copy_from(source);
		delete source;
	}
	else
	{
		printf("can't open '%s'\n", argv[1]);
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
