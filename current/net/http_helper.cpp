// http_helper.cpp -- Julien Hamaide http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// This file contains mainly code from http_server that is needed by as_loadvars

#include "net/http_helper.h"
#include "base/utility.h"

#define is_ascii(_c_) ( (_c_&0x80) == 0 )

static int hex_to_int(char c)
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

static bool is_special_character(char c)
{
	static char special_character[] = "$&+,/:=?@ \"<>#%{}|\\^~[]`";

	int i;
	for(i=0; i<LFL_ARRAYSIZE(special_character); ++i )
	{
		if( special_character[i] == c )
			break;
	}

	return i != LFL_ARRAYSIZE(special_character);
}

void url_decode(lfl_string* str)
// Do URL unescaping on a URL component.  I.e. convert the hex code
// "%xx" to the corresponding byte, and change '+' to ' '.
{
	lfl_string out;
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

void url_encode(lfl_string* str)
{
	lfl_string out;
	for (int i = 0; i < str->length(); i++) {
		char c = (*str)[i];
		if( !is_ascii( c ) || is_special_character(c) )
		{   
			out += string_printf( "%%%2x", c );
		} else {
			// Pass this character straight through.
			out += c;
		}
	}

	*str = out;
}

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
