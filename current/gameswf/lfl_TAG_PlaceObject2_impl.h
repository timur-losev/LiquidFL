// lfl_TAG_PlaceObject2_impl.h - by Timur Losev 2010

// For embedding event handlers in place_object_2

#ifndef __lfl_TAG_PlaceObject2_impl_h__
#define __lfl_TAG_PlaceObject2_impl_h__

#include "lfl_execute_tag.h"

namespace gameswf
{

	class CTAGPlaceObject2: public execute_tag
	{
	private:
		int						m_iTagType;
		lfl_string				m_CharacterName;
		float					m_fRatio;
		cxform					m_ColorTransform;
		matrix					m_Matrix;
		bool					m_bHasMatrix;
		bool					m_bHasCxform;
		int						m_iDepth;
		Uint16					m_uCharacterId;
		Uint16					m_uClipDepth;
		Uint8					m_uBlendMode;

		enum	E_PLACE_TYPE
		{
			EPT_PLACE = 0,
			EPT_MOVE,
			EPT_REPLACE
		};

		E_PLACE_TYPE			m_ePlaceType;
		array<swf_event*>		m_EventHandlers;
	public:
		//ctor
		CTAGPlaceObject2();
		//dtor
		~CTAGPlaceObject2();

		void					read(player* pPlayer, lfl_stream* pInStream, int iTagType, int iMovieVersion);
		// Place/move/whatever our object in the given movie.
		void					execute(character *pCharacter);
		void					execute_state(character *pCharacter){ this->execute(pCharacter); }
		void					execute_state_reverse(character *pCharacter, int iFrame);
		// "depth_id" is the 16-bit depth & id packed into one 32-bit int.
		virtual uint32			get_depth_id_of_replace_or_add_tag() const;

	}; //CTAGPlaceObject2

} //gameswf

#endif //__lfl_TAG_PlaceObject2_impl_h__
