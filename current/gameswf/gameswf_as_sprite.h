// gameswf_as_sprite.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Some implementation for SWF player.

// Useful links:
//
// http://sswf.sourceforge.net/SWFalexref.html
// http://www.openswf.org

#ifndef GAMESWF_AS_SPRITE_H
#define GAMESWF_AS_SPRITE_H

namespace gameswf
{

	//
	// sprite built-in ActionScript methods
	//

	void	as_global_movieclip_ctor(const function_call& fn);
	void	sprite_hit_test(const function_call& fn);
	void	sprite_start_drag(const function_call& fn);
	void	sprite_stop_drag(const function_call& fn);
	void	sprite_play(const function_call& fn);
	void	sprite_stop(const function_call& fn);
	void	sprite_goto_and_play(const function_call& fn);
	void	sprite_goto_and_stop(const function_call& fn);
	void	sprite_next_frame(const function_call& fn);
	void	sprite_prev_frame(const function_call& fn);
	void	sprite_get_bytes_loaded(const function_call& fn);
	void	sprite_get_bytes_total(const function_call& fn);
	void sprite_swap_depths(const function_call& fn) ;
	void sprite_duplicate_movieclip(const function_call& fn);
	void sprite_get_depth(const function_call& fn);
	void sprite_create_empty_movieclip(const function_call& fn);
	void sprite_remove_movieclip(const function_call& fn);
	void sprite_loadmovie(const function_call& fn);
	void sprite_unloadmovie(const function_call& fn);
	void sprite_getnexthighestdepth(const function_call& fn);
	void sprite_create_text_field(const function_call& fn);
	void sprite_attach_movie(const function_call& fn);
	void sprite_local_global(const function_call& fn);
	void sprite_global_local(const function_call& fn);
	void sprite_get_bounds(const function_call& fn);
	void sprite_get_rect(const function_call& fn);
	void sprite_set_mask(const function_call& fn);

	// drawing API
	void sprite_begin_fill(const function_call& fn);
	void sprite_end_fill(const function_call& fn);
	void sprite_clear(const function_call& fn);
	void sprite_move_to(const function_call& fn);
	void sprite_line_to(const function_call& fn);
	void sprite_curve_to(const function_call& fn);
	void sprite_line_style(const function_call& fn);

	// gameSWF extension
	void sprite_set_fps(const function_call& fn);
	void sprite_get_play_state(const function_call& fn);

	// flash9
	void sprite_add_script(const function_call& fn);

}

#endif
