// webwidgets.h	-- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Immediate-mode web-based UI.  Relies on http_server.  Uses
// "Immediate Mode GUI" ideas.


#ifndef WEBWIDGETS_H
#define WEBWIDGETS_H


#include "base/container.h"

struct http_request;


// State for generating & keeping track of unique ID's for a set of
// widgets.
struct webids
{
	webids(http_request* req);
	
	void push_label(const char* label);
	void pop_label();

	// Gets an id for the label, uniquified by previous nested
	// labels.
	tu_string get_id(const char* label);

	// For querying the http_request.
	int param_count(const tu_string& label);
	const tu_string& get_param(const tu_string& label, int index);

private:
	http_request* m_req;
	array<tu_string> m_label_stack;
};


// Container for web widgets generated via an immediate-mode
// interface.  Multiple webwidgets can share the same webids; for
// example you can make a multi-frame UI using several webwidgets
// pointing at one webids.
struct webwidgets
{
	webwidgets(webids* ids);

	// This is for making a node in a treeview.  End it with
	// end_tree_group(). It returns true if the group is open.
	//
	// Typical usage is:
	//
	// if (begin_tree_group("my node label")) {
	// 	// display the subordinate widgets, typically nested tree nodes...
	// }
	// end_tree_group();
	bool begin_tree_group(const char* label);
	void end_tree_group();

	// For adding raw HTML output.
	void print(const char* text);
	void print(const tu_string& text);
	
	// Presents a labelled button.  You can optionally pass a
	// pointer to a var to be filled with the value.
	//
	// Use like:
	//
	// if (button("press me")) {
	//   // User pressed the button.
	// }
	bool button(const char* label);

	// Use like:
	//
	// if (checkbox("Are you stinky?", &stinky)) {
	//   // stinky is true.
	// }
	bool checkbox(const char* label, bool* value);

	// Float slider.
	void slider(const char* label, float* value, float min, float max);
	
	const tu_string& get_html() const { return m_output; }
	
private:
	webids* m_ids;
	tu_string m_output;
};


#endif // WEBWIDGETS_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

