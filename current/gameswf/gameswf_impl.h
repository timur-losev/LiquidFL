// gameswf_impl.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some implementation code for the gameswf SWF player library.


#ifndef GAMESWF_IMPL_H
#define GAMESWF_IMPL_H


#include "gameswf/gameswf.h"
#include "gameswf/gameswf_action.h"
#include "gameswf/gameswf_types.h"
#include "gameswf/gameswf_log.h"
#include "gameswf/gameswf_movie_def.h"
#include "gameswf/gameswf_character.h"
#include "gameswf/gameswf_render.h"
#include "gameswf/lfl_execute_tag.h"
#include "gameswf/lfl_swf_event.h"
#include <assert.h>
#include "base/container.h"
#include "base/utility.h"
#include <stdarg.h>


namespace jpeg { struct input; }


namespace gameswf
{
	struct action_buffer;
	struct bitmap_character_def;
	struct bitmap_info;
	struct character;
	struct character_def;
	struct display_info;
	//struct SExecuteTag;
	struct font;
	struct root;
	struct movie_definition_sub;

	struct lfl_stream;
	//struct SSWFEvent;	


	// For characters that don't store unusual state in their instances.
	struct generic_character : public character
	{
		gc_ptr<character_def>	m_def;

		generic_character(player* player, character_def* def, character* parent, int id) :
			character(player, parent, id),
			m_def(def)
		{
			assert(m_def != NULL);
		}

		~generic_character()
		{
		}

		virtual character_def* get_character_def() { return m_def.get_ptr();	}

		virtual void	display()
		{
			m_def->display(this);	// pass in transform info
			do_display_callback();
		}

		virtual bool get_topmost_mouse_entity( character * &te, float x, float y)
		{
			assert(get_visible());	// caller should check this.

			const matrix&	m = get_matrix();
			point	p;
			m.transform_by_inverse(&p, point(x, y));

			if (m_def->point_test_local(p.m_x, p.m_y))
			{
				// The mouse is inside the shape.
				te = this;
				return true;
			}
			return false;
		}
	};


	struct bitmap_character_def : public character_def
	{
		bitmap_character_def(player* player) :
			character_def(player)
		{
		}

		virtual gameswf::bitmap_info*	get_bitmap_info() = 0;
	};

	// Bitmap character
	struct bitmap_character : public bitmap_character_def
	{
		bitmap_character(movie_definition* rdef, bitmap_info* bi);

		// Return true if the specified point is on the interior of our shape.
		// Incoming coords are local coords.
		bool	point_test_local(float x, float y);

		virtual void get_bound(rect* bound);
		virtual void	display(character* ch);
		gameswf::bitmap_info*	get_bitmap_info();

		private:

			gc_ptr<gameswf::bitmap_info>	m_bitmap_info;
	};

	// Execute tags include things that control the operation of
	// the movie.  Essentially, these are the events associated
	// with a frame.
	// Timur - moved to lfl_execute_tag.h
	// TODO: delete this
/*	struct execute_tag
	{
		virtual ~execute_tag() {}
		virtual void	execute(character* m) {}
		virtual void	execute_state(character* m) {}
		virtual void	execute_state_reverse(character* m, int frame) { execute_state(m); }
		virtual bool	is_remove_tag() const { return false; }
		virtual bool	is_action_tag() const { return false; }
		virtual uint32	get_depth_id_of_replace_or_add_tag() const { return static_cast<uint32>(-1); }
	};*/

	//
	// Loader callbacks.
	//

	// Register a loader function for a certain tag type.  Most
	// standard tags are handled within gameswf.  Host apps might want
	// to call this in order to handle special tag types.
	void	register_tag_loader(int tag_type, loader_function lf);


	// Tag loader functions.
	void	null_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	set_background_color_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	jpeg_tables_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_bits_jpeg_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_bits_jpeg2_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_bits_jpeg3_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_shape_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_shape_morph_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_font_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_font_info_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_text_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_edit_text_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	place_object_2_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_bits_lossless_2_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	sprite_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	end_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	remove_object_2_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	do_action_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	button_character_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	frame_label_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	export_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	import_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_sound_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	start_sound_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	button_sound_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	do_init_action_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_video_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	video_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	sound_stream_head_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	sound_stream_block_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_file_attribute_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_font_alignzones(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_csm_textsetting_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_metadata_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_enable_debugger_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_tabindex_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_abc_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	symbol_class_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_scene_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);
	void	define_font_name(lfl_stream* in, int tag_type, movie_definition_sub* m);
    void    define_script_limits_loader(lfl_stream* in, int tag_type, movie_definition_sub* m);

}	// end namespace gameswf


#endif // GAMESWF_IMPL_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
