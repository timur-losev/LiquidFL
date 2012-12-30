// webtweaker.h	-- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// A tweaker UI using immediate-mode webwidgets.  Defines a basic
// layout: a header with some generic info, tree nav on the left, and
// the main tweaker pane on the right/middle.


#ifndef WEBTWEAKER_H
#define WEBTWEAKER_H


#include "net/webwidgets.h"

struct http_server;
struct http_request;


struct webtweaker
{
	// Instantiate one of these when you want to generate a web UI.
	// E.g.:
	//
	// void do_ui(webtweaker* wt) {
	//   ... ui traversal, calling wt->begin_group(), wt->button() etc.
	// }
	//
	// struct do_my_ui : public http_handler
	// {
	// 	virtual void handle_request(http_server* server, const tu_string& key, http_request* req)
	// 	{
	//		webtweaker wt("My Tweaker", server, req);
	// 		do_ui(&wt);
	// 		wt.send_response();
	// 	}
	// } my_ui;
	// server->add_handler("/tweak", &my_ui);
	webtweaker(const char* name, http_server* server, http_request* req);
	~webtweaker();

	// Sends the HTML response back to the client.
	void send_response();

	// Access to the various panes.
	webwidgets* head() { return m_head; }
	webwidgets* nav() { return m_nav; }
	webwidgets* body() { return m_body; }

private:
//data:
	tu_string m_name;

	http_server* m_server;
	http_request* m_req;

	webids* m_ids;
	webwidgets* m_head;
	webwidgets* m_nav;
	webwidgets* m_body;
};


#endif // WEBTWEAKER_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
