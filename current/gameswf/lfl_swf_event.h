// lfl_swf_event.h - by Timur Losev 2010

#ifndef __lfl_swf_event_h__
#define __lfl_swf_event_h__

#include "gameswf/gameswf_action.h"
#include "gameswf/gameswf_value.h"

namespace gameswf
{
	//base event struct
	struct swf_event
	{
		// NOTE: DO NOT USE THESE AS VALUE TYPES IN AN
		// array<>!  They cannot be moved!  The private
		// operator=(const SSWFEvent&) should help guard
		// against that.

		event_id	m_event;
		as_value	m_method;

		swf_event() {}

	private:

		// DON'T USE THESE
		swf_event(const swf_event& s) { assert(0); }
		void	operator=(const swf_event& s) { assert(0); }
	}; //SSWFEvent
}//gameswf

#endif //__lfl_swf_event_h__