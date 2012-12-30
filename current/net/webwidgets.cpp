// webwidgets.cpp -- Thatcher Ulrich http://tulrich.com 2005

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// Immediate-mode web-based UI.  Relies on http_server.  Uses
// "Immediate Mode GUI" ideas.


#include "net/webwidgets.h"
#include "net/http_server.h"


//
// webids
//


webids::webids(http_request* req)
	:
	m_req(req)
{
}


void webids::push_label(const char* label)
{
	m_label_stack.push_back(tu_string(label));
}


void webids::pop_label()
{
	m_label_stack.pop_back();
}


static char hex_digit(int val)
{
	assert(val >= 0 && val < 16);
	if (val < 10) {
		return '0' + val;
	} else {
		return 'A' + val - 10;
	}
}


tu_string webids::get_id(const char* label)
{
	uint32 hash_val = 0;
	for (int i = 0, n = m_label_stack.size(); i < n; i++) {
		const tu_string& str =  m_label_stack[i];
		hash_val = bernstein_hash("/", 1, hash_val);
		hash_val = bernstein_hash((const char*) str, str.size(), hash_val);
	}
	hash_val = bernstein_hash("/", 1, hash_val);
	hash_val = bernstein_hash(label, strlen(label), hash_val);

#ifndef NDEBUG
	// Check to make sure the computed id is unique.
	//if (m_used_ids.get(hash_val, NULL)) {
	//	// assert, log, whatever
	//}
#endif // NDEBUG

	// Convert the hash to a string.
	tu_string id("00000000");
	int digit = 7;
	while (hash_val > 0) {
		assert(digit >= 0);
		id[digit] = hex_digit(hash_val & 15);
		hash_val >>= 4;
		digit--;
	}

	return id;
}


int webids::param_count(const tu_string& label)
{
	return m_req->param_count(label);
}


const tu_string& webids::get_param(const tu_string& key, int index)
{
	return m_req->get_param(key, index);
}



//
// webwidgets
//


webwidgets::webwidgets(webids* ids)
	:
	m_ids(ids)
{
}


bool webwidgets::begin_tree_group(const char* label)
{
	tu_string str_label(label);

	tu_string id = m_ids->get_id(str_label);
	m_ids->push_label(str_label);
	
	bool is_open = false;
	if (m_ids->param_count(id) && m_ids->get_param(id, 0) == "1") {
		// This branch is open.
		is_open = true;
	} else {
		// This branch is closed.
	}

	m_output += string_printf("<script>new wt_tree_node(\"%s\", \"%s\", %d);</script>\n"
				  "<div style=\"padding-left: 1em\">\n",
				  id.c_str(),
				  label,
				  int(is_open));

	return is_open;
}


void webwidgets::end_tree_group()
{
	m_output += "</div>";

	m_ids->pop_label();
}


void webwidgets::print(const char* text)
{
	m_output += text;
}


void webwidgets::print(const tu_string& text)
{
	m_output += text;
}


bool webwidgets::button(const char* label)
{
	tu_string str_label(label);
	tu_string id = m_ids->get_id(str_label);
	
	m_output += string_printf("<button name=\"%s\" value=1>%s</button>\n", id.c_str(), label);
	if (m_ids->param_count(id)) {
		return true;
	} else {
		return false;
	}
}


void webwidgets::slider(const char* label, float* value, float min, float max)
{
	assert(min <= max);

	tu_string str_label(label);
	tu_string id = m_ids->get_id(str_label);

	// Update.
	if (m_ids->param_count(id)) {
		const tu_string& new_value_str = m_ids->get_param(id, 0);
		*value = fclamp(float(atof(new_value_str.c_str())), min, max);
	}
	
	m_output += string_printf("<script>\nnew wt_slider(\"%s\", \"%s\", %f, %f, %f);\n</script>\n",
				  id.c_str(), label, *value, min, max);
}
