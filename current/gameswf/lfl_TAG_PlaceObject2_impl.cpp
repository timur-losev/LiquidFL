#include "lfl_TAG_PlaceObject2_impl.h"
#include "lfl_tag_defines.h"

#include "gameswf/gameswf_filters.h"
#include "gameswf/gameswf_action.h"
#include "gameswf/lfl_key_handling.h"
#include "gameswf/lfl_swf_event.h"

namespace gameswf
{

	CTAGPlaceObject2::CTAGPlaceObject2():
		 m_iTagType(0)
		,m_fRatio(0.f)
		,m_bHasMatrix(false)
		,m_bHasCxform(false)
		,m_iDepth(0)
		,m_uCharacterId(0)
		,m_uClipDepth(0)
		,m_uBlendMode(0)
		,m_ePlaceType(EPT_PLACE)
	{

	}

	CTAGPlaceObject2::~CTAGPlaceObject2()
	{
		for(Uint32 i = 0, n = (Uint32)m_EventHandlers.size(); i < n; ++i)
		{
			delete m_EventHandlers[i];
		}
		m_EventHandlers.resize(0);
	}

	void CTAGPlaceObject2::read( player* pPlayer, lfl_stream* pInStream, int iTagType, int iMovieVersion )
	{
		assert(iTagType == TAG_PlaceObject || iTagType == TAG_PlaceObject2 || iTagType == TAG_PlaceObject3);
		m_iTagType = iTagType;

		if(iTagType == TAG_PlaceObject)
		{
			m_uCharacterId	= pInStream->read_u16();
			m_iDepth		= pInStream->read_u16();
			m_Matrix.read(pInStream);
			
			IF_VERBOSE_PARSE(log_msg("  char_id = %d\n"
							"  depth = %d\n"
							"  mat = \n",
							m_uCharacterId,
							m_iDepth);
							m_Matrix.print());

			if(pInStream->get_position() < pInStream->get_tag_end_position())
			{
				m_ColorTransform.read_rgb(pInStream);
				IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_ColorTransform.print());
			}
		}
		else 
		if(iTagType == TAG_PlaceObject2 || iTagType == TAG_PlaceObject3)
		{
			pInStream->align();

			int bHasActions				=	pInStream->read_uint(1);
			int bHasClipBracket			=	pInStream->read_uint(1);
			int bHasName				=	pInStream->read_uint(1);
			int bHasRatio				=	pInStream->read_uint(1);
			int bHasCxform				=	pInStream->read_uint(1);
			int bHasMatrix				=	pInStream->read_uint(1);
			int bHasChar				=	pInStream->read_uint(1);
			int bFlagMove				=	pInStream->read_uint(1);

			int bHasImage				=	0;
			int bHasClassName			=	0;
			int bCacheAsBmp				=	0;
			int bHasBlendMode			=	0;
			int bHasFilterList			=	0;

			if(iTagType == TAG_PlaceObject3)
			{
				pInStream->read_uint(3); //unused
				
				bHasImage				=	pInStream->read_uint(1);
				bHasClassName			=	pInStream->read_uint(1);
				bCacheAsBmp				=	pInStream->read_uint(1);
				bHasBlendMode			=	pInStream->read_uint(1);
				bHasFilterList			=	pInStream->read_uint(1);
			}

			m_iDepth					=	pInStream->read_u16();	//required field
			IF_VERBOSE_PARSE(log_msg("  depth = %d\n", m_iDepth));

			if(bHasChar)
			{
				m_uCharacterId			=	pInStream->read_u16();
				IF_VERBOSE_PARSE(log_msg("  char id = %d\n", m_uCharacterId));
			}

			if(bHasMatrix)
			{
				m_bHasMatrix = true;
				m_Matrix.read(pInStream);
				IF_VERBOSE_PARSE(log_msg("  mat:\n"); m_Matrix.print());
			}

			if(bHasCxform)
			{
				m_bHasCxform = true;
				m_ColorTransform.read_rgba(pInStream);
				IF_VERBOSE_PARSE(log_msg("  cxform:\n"); m_ColorTransform.print());
			}

			if(bHasRatio)
			{
				m_fRatio				=	(float)pInStream->read_u16() / (float)65536;\
				IF_VERBOSE_PARSE(log_msg("  ratio: %f\n", m_fRatio));
			}

			if(bHasName)
			{
				pInStream->read_string(&m_CharacterName);
				IF_VERBOSE_PARSE(log_msg("  name = %s\n", m_CharacterName.c_str()));
			}

			if(bHasClipBracket)
			{
				m_uClipDepth			=	pInStream->read_u16();
				IF_VERBOSE_PARSE(log_msg("  clip_depth = %d\n", m_uClipDepth));
			}

			if(bHasFilterList)
			{
				read_filter_list(pInStream);
			}
			
			if(bHasBlendMode)
			{
				//TODO:
				m_uBlendMode = pInStream->read_u8();
			}

			if(bHasActions)
			{
				uint16 unused = pInStream->read_u16();
				UNUSED(unused);

				Uint32 all_flags = 0;
				if(iMovieVersion >= 6)
				{
					all_flags = pInStream->read_u32();
				}
				else
				{
					all_flags = pInStream->read_u16();
				}

				IF_VERBOSE_PARSE(log_msg("  actions: flags = 0x%X\n", all_flags));

				// read events
				while(true)
				{
					//read event
					pInStream->align();

					Uint32 flags = (iMovieVersion >= 6) ? pInStream->read_u32() : pInStream->read_u16();
					if (flags == 0)
					{
						// Done with events.
						break;
					}
					
					Uint32 event_length = pInStream->read_u32();
					Uint8 ch = key::INVALID;

					if(flags & 0x20000) //has keypress event
					{
						ch = pInStream->read_u8();
						event_length--;
					}

					//read the actions for event(s)
					action_buffer action;
					action.read(pInStream);

					if(action.get_length() != static_cast<int>(event_length))
					{
						log_error("SSWFEvent::read(), event_length = %d, but read %d\n",
							event_length, action.get_length());
						break;
					}

					// 13 bits reserved, 19 bits used
					static const event_id s_code_bits[19] =
					{
						event_id::LOAD,
						event_id::ENTER_FRAME,
						event_id::UNLOAD,
						event_id::MOUSE_MOVE,
						event_id::MOUSE_DOWN,
						event_id::MOUSE_UP,
						event_id::KEY_DOWN,
						event_id::KEY_UP,
						event_id::DATA,
						event_id::INITIALIZE,
						event_id::PRESS,
						event_id::RELEASE,
						event_id::RELEASE_OUTSIDE,
						event_id::ROLL_OVER,
						event_id::ROLL_OUT,
						event_id::DRAG_OVER,
						event_id::DRAG_OUT,
						event_id(event_id::KEY_PRESS, key::CONTROL),
						event_id::CONSTRUCT
					};
					const int code_bits_len = LFL_ARRAYSIZE(s_code_bits);
					// Let's see if the event flag we received is for an event that we know of
					if (1 << code_bits_len < flags)
					{
						log_error("SSWFEvent::read() -- unknown event type received, flags = 0x%x\n", flags);
					}

					for (int i = 0, mask = 1; i < code_bits_len; i++, mask <<= 1)
					{
						if (flags & mask)
						{
							swf_event* ev = new swf_event;
							ev->m_event = s_code_bits[i];

							if (i == 17)	// has keypress event ?
							{
								ev->m_event.m_key_code = ch;
							}

							// Create a function to execute the actions.
							array<with_stack_entry>	empty_with_stack;
							as_s_function*	func = new as_s_function(pPlayer, &action, 0, empty_with_stack);
							func->set_length(action.get_length());

							ev->m_method.set_as_object(func);

							m_EventHandlers.push_back(ev);
						}
					}
				}
			}

			if (bHasChar != 0 && bFlagMove != 0)
			{
				// Remove whatever's at m_iDepth, and put m_character there.
				m_ePlaceType = EPT_PLACE;
			}
			else if (bHasChar == 0 && bFlagMove != 0)
			{
				// Moves the object at m_iDepth to the new location.
				m_ePlaceType = EPT_MOVE;
			}
			else if (bHasChar != 0 && bFlagMove == 0)
			{
				// Put m_character at m_iDepth.
				m_ePlaceType = EPT_PLACE;
			}

		}
	}

	void CTAGPlaceObject2::execute( character *pCharacter )
	{
		switch (m_ePlaceType)
			{
			default:
				break;

			case EPT_PLACE:
				if ((m_uBlendMode == 0) && (pCharacter->m_blend_mode != 0))
				{
					pCharacter->add_display_object( m_uCharacterId, m_CharacterName.c_str(), m_EventHandlers, m_iDepth,
						m_iTagType != TAG_PlaceObject, m_ColorTransform, m_Matrix, m_fRatio, m_uClipDepth, pCharacter->m_blend_mode);
				}
				else
				{
					pCharacter->add_display_object( m_uCharacterId, m_CharacterName.c_str(), m_EventHandlers, m_iDepth, 
						m_iTagType != TAG_PlaceObject, m_ColorTransform, m_Matrix, m_fRatio, m_uClipDepth, m_uBlendMode);
				}
				break;

			case EPT_MOVE:
				if ((m_uBlendMode == 0) && (pCharacter->m_blend_mode != 0))
				{
					pCharacter->move_display_object( m_iDepth, m_bHasCxform, m_ColorTransform, m_bHasMatrix, m_Matrix, m_fRatio,
						m_uClipDepth, pCharacter->m_blend_mode);
				}
				else
				{
					pCharacter->move_display_object( m_iDepth, m_bHasCxform, m_ColorTransform, m_bHasMatrix, m_Matrix, m_fRatio,
						m_uClipDepth, m_uBlendMode);
				}
				break;

			case EPT_REPLACE:
				{
					pCharacter->replace_display_object( m_uCharacterId, m_CharacterName.c_str(), m_iDepth, m_bHasCxform, m_ColorTransform, 
						m_bHasMatrix, m_Matrix, m_fRatio, m_uClipDepth, m_uBlendMode);
				}
				break;
			}
	}

	void CTAGPlaceObject2::execute_state_reverse( character *pCharacter, int iFrame )
	{
		switch (m_ePlaceType)
		{
			default:
				break;

			case EPT_PLACE:
				{
					// reverse of add is remove
					// NOTE:  this line used to read m->remove_display_object(m_depth, m_tag_type == TAG_PlaceObject ? m_character : -1);
					// we were having problems with in certain cases remove_display_object removing the incorrect object,
					// the line was returning -1 frequently and inside remove_display_object it would pick an incorrect object
					// at the same depth
					pCharacter->remove_display_object(m_iDepth, m_uCharacterId);
				}
				break;

			case EPT_MOVE:
				{
					// reverse of move is move
					pCharacter->move_display_object( m_iDepth, m_bHasCxform, m_ColorTransform, m_bHasMatrix, 
						m_Matrix, m_fRatio, m_uClipDepth, pCharacter->m_blend_mode);
				}
				break;

			case EPT_REPLACE:
				{
					// reverse of replace is to re-add the previous object.
					execute_tag*	last_add = pCharacter->find_previous_replace_or_add_tag(iFrame, m_iDepth, -1);
					if (last_add)
					{
						last_add->execute_state(pCharacter);
					}
					else
					{
						log_error("reverse REPLACE can't find previous replace or add tag(%d, %d)\n", iFrame, m_iDepth);
					}
					break;
				}
		}
	}

	uint32 CTAGPlaceObject2::get_depth_id_of_replace_or_add_tag() const
	{
		if (m_ePlaceType == EPT_PLACE || m_ePlaceType == EPT_REPLACE)
			{
				int	id = -1;
				if (m_iTagType == TAG_PlaceObject)
				{
					// Old-style PlaceObject; the corresponding Remove
					// is specific to the character_id.
					id = m_uCharacterId;
				}
				return ((m_iDepth & 0x0FFFF) << 16) | (id & 0x0FFFF);
			}
			else
			{
				return (uint32) -1;
			}
	}

}//gameswf