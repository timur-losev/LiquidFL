// gameswf_string.h	-- Rob Savoye <rob@welcomehome.org> 2005

// This source code has been donated to the Public Domain. Do whatever
// you want with it.

// Implementation for ActionScript String object.


#ifndef GAMESWF_STRING_H
#define GAMESWF_STRING_H

#include "gameswf/gameswf_action.h"

namespace gameswf 
{

	// Constructor for creating ActionScript String object.
	void as_global_string_ctor(const function_call& fn);
	as_object * get_global_string_ctor(player * player);

	void string_char_code_at(const function_call& fn);
	void string_concat(const function_call& fn);
	void string_from_char_code(const function_call& fn);
	void string_index_of(const function_call& fn);
	void string_last_index_of(const function_call& fn);
	void string_slice(const function_call& fn);
	void string_split(const function_call& fn);
	void string_substr(const function_call& fn);
	void string_substring(const function_call& fn);
	void string_to_lowercase(const function_call& fn);
	void string_to_uppercase(const function_call& fn) ;
	void string_char_at(const function_call& fn);
	void string_to_string(const function_call& fn);
	void string_length(const function_call& fn);
}


#endif // GAMESWF_STRING_H
