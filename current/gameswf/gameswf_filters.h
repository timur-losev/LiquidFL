// gameswf_filters.h	-- Julien Hamaide <julien.hamaide@gmail.com> 2008

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Filters

#ifndef GAMESWF_FILTERS_H
#define GAMESWF_FILTERS_H

#include "gameswf_stream.h"

namespace gameswf
{
	enum E_SWF_FILTERS
	{
		FILTER_DropShadowFilter			= 0,
		FILTER_BlurFilter				= 1,
		FILTER_GlowFilter				= 2,
		FILTER_BevelFilter				= 3,
		FILTER_GradientGlowFilter		= 4,
		FILTER_ConvolutionFilter		= 5,
		FILTER_ColorMatrixFilter		= 6,
		FILTER_GradientBevelFilter		= 7,

		Last_SWF_Filter
	};

	void read_filter_list( lfl_stream* in );
}

#endif //GAMESWF_FILTERS_H

// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
