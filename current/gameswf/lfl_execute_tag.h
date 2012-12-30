// lfl_execute_tag.h - by Timur Losev 2010

// Execute tags include things that control the operation of
// the movie.  Essentially, these are the events associated
// with a frame.

#ifndef __lfl_execute_tag_h__
#define __lfl_execute_tag_h__


#include "gameswf_character.h"

namespace gameswf 
{
	struct execute_tag
	{
		virtual			~execute_tag() {}
		virtual void	execute(character* ){}
		virtual void	execute_now(character* m) { execute(m); }
		virtual void	execute_state(character*){}
		virtual void	execute_state_reverse(character* m, int frame) { execute_state(m); }
		virtual bool	is_remove_tag() const { return false; }
		virtual bool	is_action_tag() const { return false; }
		virtual uint32	get_depth_id_of_replace_or_add_tag() const { return static_cast<uint32>(-1); }
	};
}//gameswf 

#endif //__lfl_execute_tag_h__
