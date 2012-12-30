// net_test.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Small test program to exercise tu-testbed/net/ library.


#include <stdio.h>
#include "base/logger.h"
#include "base/tu_timer.h"
#include "net/http_file_handler.h"
#include "net/http_server.h"
#include "net/net_interface.h"
#include "net/webtweaker.h"


struct my_handler : public http_handler
{
	virtual void handle_request(http_server* server, const tu_string& key, http_request* req)
	{
		tu_string out;
		out += "<html><head><title>tu-testbed net_test</title></head><body>";
		req->dump_html(&out);

		// Show the current timer.
		char buf[200];
		sprintf(buf, "Current timer: %f<br>\n", tu_timer::get_ticks() / 1000.0f);
		out += buf;

		out += "</body></html>\n";

		server->send_html_response(req, out);
	}
};


static bool s_quit = false;


struct quit_handler : public http_handler
{
	virtual void handle_request(http_server* server, const tu_string& key, http_request* req)
	{
		server->send_text_response(req, "Bye!");
		
		// Exit the app, next time through the main loop.
		s_quit = true;
	}
};


void do_ui(webtweaker* wt)
// This is the entry point to our UI traversal code.  Webtweaker calls
// this; we do all our ui stuff like, wt->body().button(...) yadda yadda from
// here.
{
	static float s_slider_value = 0;

	wt->head()->print("<h2>My Tweaker</h2>");
	wt->head()->print("<a href=\"/quit\">quit</a>");
	
	if (wt->nav()->begin_tree_group("stuff 0")) {
		if (wt->nav()->begin_tree_group("Stuff 1")) {
			if (wt->nav()->button("Push me!")) {
				printf("Pushed!\n");
				wt->body()->print("Pushed!\n");
			}

			if (wt->body()->button("Quit")) {
				// Exit the app, next time through the main loop.
				wt->body()->print("Bye!");
				s_quit = true;
			}

			wt->body()->slider("Some value", &s_slider_value, -10, 10);
		}
		wt->nav()->end_tree_group();
	}
	wt->nav()->end_tree_group();
}


int main(int argc, const char** argv)
{
	int port = 30000;

	logger::set_standard_log_handlers();

	for (int i = 1; i < argc; i++) {
		const char* arg = argv[i];
		if (arg[1] && arg[0] == '-') {
			switch (arg[1]) {
			default:
				break;
			case 'v':
				// Verbose logging.
				logger::FLAG_verbose_log = true;
				break;
			case 'p':
				if (argc > i) {
					i++;
					port = atoi(argv[i]);
				}
				break;
			}
		}
	}

	// Open a socket to receive connections on.
	net_interface* iface = tu_create_net_interface_tcp(port);
	if (iface == NULL)
	{
		fprintf(stderr, "Couldn't open net interface\n");
		exit(1);
	}

	// Attach a server to the socket.
	http_server* server = new http_server(iface);

	// Add handlers to the server.

	// This one gets called if the browser hits
	// "http://this.host:<port>/status"
	my_handler handler;
	server->add_handler("/status", &handler);

	// This one gets called if the browser hits
	// "http://this.host:<port>/quit"
	quit_handler quit_h;
	server->add_handler("/quit", &quit_h);

	// Create a webtweaker, for presenting the app's tweaking UI.
	// Hook it up to the URL "http://this.host:<port>/tweak".
	struct do_my_ui : public http_handler
	{
		virtual void handle_request(http_server* server, const tu_string& key, http_request* req)
		{
			webtweaker wt("My Tweaker", server, req);
			do_ui(&wt);
			wt.send_response();
		}
	} my_ui;
	server->add_handler("/tweak", &my_ui);

	// Create a handler for serving static files.
	http_file_handler static_handler("./static", "/static");
	server->add_handler(static_handler.http_basepath(), &static_handler);
	
	printf("Point a browser at http://localhost:%d/tweak to test webtweaker\n", port);
	fflush(stdout);

	// This is a stand-in for the typical game loop.  We just need
	// to call server->update() somewhere in there.
	while (s_quit == false)
	{
		server->update();
		tu_timer::sleep(10);
	}
}


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:

