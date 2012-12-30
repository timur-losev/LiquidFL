// webtweaker.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Immediate-mode web-based tweaker UI.


#include "base/tu_types.h"
#include "base/tu_file.h"
#include "base/container.h"
#include "net/webtweaker.h"
#include "net/http_server.h"


webtweaker::webtweaker(const char* name, http_server* server, http_request* req)
	:
	m_name(name),
	m_server(server),
	m_req(req)
{
	m_ids = new webids(req);
	m_head = new webwidgets(m_ids);
	m_nav = new webwidgets(m_ids);
	m_body = new webwidgets(m_ids);
}


webtweaker::~webtweaker()
{
	delete m_body;
	delete m_nav;
	delete m_head;
	delete m_ids;
}


void webtweaker::send_response()
{
	tu_string out;
	out += string_printf(
		"<html><head><title>%s</title>\n"
		"<META http-equiv=\"Content-Script-Type\" content=\"text/javascript\">\n"
		"<script src=\"/static/wt_widgets.js\"></script>"
		"</head>\n<body onLoad=\"wt_init()\">\n"
//		"<form action=\"%s\" method=POST name=\"webtweaker\">\n"
		"<form action=\"http://localhost:30000/tweak\" method=POST name=\"webtweaker\">\n"
		,
		m_name.c_str()/*,
		m_req->m_path.c_str() */);

	// Overall page structure, using a table.
	out += "<table><tr height=100px><td colspan=2 style=\"border-bottom: 2px solid #1020D0;\">\n";
	out += m_head->get_html();
	out += "</td></tr>\n";
	out += "<tr><td width=200px valign=top style=\"border-right: 2px solid #1020D0;\">\n";
	out += m_nav->get_html();
	out += "</td><td valign=top>\n";
	out += m_body->get_html();
	out += "</td></tr></table>\n";

	out += "</form>\n";
	
	// if (debug) { ...
	out += 
		"<!-- debug log window -->\n"
		"<form action=\"\" method=POST name=\"debugout\">\n"
		"  <textarea readonly=1 id=\"debugout\" cols=80 rows=10>\n"
		"  </textarea>\n"
		"  <br>\n"
		"  <button type=button name=clear onclick=\"document.getElementById('debugout').value='';\">Clear</button>\n"
		"</form>\n"
		;
	// }

	m_req->dump_html(&out);

	out += "</body></html>\n";

	m_server->send_html_response(m_req, out);
}


#if 0
webtweaker* webtweaker::begin_vertical_split(const char* right_pane_label)
// Split the output pane.  Output to *this goes in the left pane,
// output to the return value goes to the right pane.  Use
// end_vertical_split() when you're done outputting to the panes.
{
	// Can't split the left pane again.
	//
	// TODO: this actually might be worth supporting at some
	// point...
	assert(m_right_pane == NULL);

	m_right_pane = new webtweaker(right_pane_label, m_server, m_req);
	// m_right_pane.seed_hash(...);

	m_output += "<table><tr><td width=200px valign=top>\n";
	
	return m_right_pane;
}

void webtweaker::end_vertical_split()
{
	assert(m_right_pane);

	// Close the left pane, open the right pane.
	m_output += "</td>\n<td valign=top>\n";

	// Emit the right pane.
	m_output += m_right_pane->get_html();

	// Close the table.
	m_output += "</td></tr></table>\n";

	delete m_right_pane;
	m_right_pane = NULL;
}
#endif // 0


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

