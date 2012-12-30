// as_point.h	-- Julien Hamaide <julien.hamaide@gmail.com>	2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.


#ifndef GAMESWF_AS_MATRIX_H
#define GAMESWF_AS_MATRIX_H

#include "gameswf/gameswf_action.h"	// for as_object
#include "gameswf/gameswf_character.h"

namespace gameswf
{

	void	as_global_matrix_ctor(const function_call& fn);

	struct as_matrix : public as_object
	{
		// Unique id of a gameswf resource
		enum { m_class_id = AS_MATRIX };
		virtual bool is(int class_id) const
		{
			if (m_class_id == class_id) return true;
			else return as_object::is(class_id);
		}

		as_matrix(player* player);

		matrix m_matrix;
	};

}	// end namespace gameswf


#endif // GAMESWF_AS_MATRIX_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
