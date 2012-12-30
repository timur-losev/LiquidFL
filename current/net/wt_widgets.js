// wt_widgets.js -- Thatcher Ulrich 2005

// This code has been donated to the Public Domain.

// JavaScript/HTML widgets for WebTweaker


// "wt_" --> Web Tweaker

// I hate all the commercial JavaScript/HTML help sites saturated with
// animated ads and low-density information.
//
// This site is pretty good for explaining some DHTML basics:
// http://www.brainjar.com/dhtml/intro/
//
// The Mozilla docs are OK: http://www.mozilla.org/docs/dom
//
// Some more good DOM info:
// http://wally.cs.iupui.edu/n341-client/jsab08/DomNotes.html
//
// Official HTML info:
// http://www.w3.org/TR/REC-html40/cover.html#minitoc
//
// Inheritance tutorial:
// http://www.kevlindev.com/tutorials/javascript/inheritance/
//
// Nice example slider: http://webfx.eae.net/dhtml/slider/slider.html

// Mollyrocket IMGUI forum:
// https://mollyrocket.com/forums/viewforum.php?f=10

// Decent HTML/CSS tutorials:
// http://www.goer.org/HTML/intermediate/align_and_indent/

// "Core JavaScript 1.5 Guide" @ mozilla.org
// http://developer.mozilla.org/en/docs/Core_JavaScript_1.5_Guide

// "How To Write HTML Forms"
// http://www.cs.tut.fi/~jkorpela/forms/index.html

// DOM
// http://www.csscripting.com/css-multi-column/dom-width-height.php

// onclick
// onmouseup
// onmousemove
// onmouseout


var wt_widgets = new Array();
var wt_tracking_widget = undefined;


var isIE = navigator.userAgent.indexOf("MSIE") >= 0;


// page_offset_x(event) gives the x coord of the mouse event, in
// relation to the page origin.
var page_offset_x;
if (isIE) {
	page_offset_x = function(event) {
		return window.event.clientX + document.documentElement.scrollLeft
		+ document.body.scrollLeft;
	}
} else {
	page_offset_x = function(event) {
		return event.pageX;
	}
}


// page_offset_y(event) gives the y coord of the mouse event, in
// relation to the page origin.
var page_offset_y;
if (isIE) {
	page_offset_y = function(event) {
		return window.event.clientY + document.documentElement.scrollTop
		+ document.body.scrollTop;
	}
} else {
	page_offset_y = function(event) {
		return event.pageY;
	}
}


// event_target(event) gives the object that is the target of the
// event.
var event_target;
if (isIE) {
	event_target = function(event) {
		return window.event.srcElement;
	}
} else {
	event_target = function(event) {
		return event.target;
	}
}


//document.documentElement.scrollLeft;


function fclamp(val, min, max)
// Return the val, clamped to be within [min,max].
{
	if (val < min) {
		return min;
	} else if (val > max) {
		return max;
	} else {
		return val;
	}
}


function absolute_left(elem)
// absolute_left(elem) gives the x coordinate of the left of the given
// element, relative to the page origin.
{
	var x = elem.offsetLeft;
	for (;;) {
		elem = elem.offsetParent;
		if ((!elem) || elem == document.body) {
			break;
		}
		if (typeof(elem.offsetLeft) == "number") {
			x += elem.offsetLeft;
		}
	}
	return x;
}


function absolute_top(elem)
// absolute_top(elem) gives the y coordinate of the top of the given
// element, relative to the page origin.
{
	var y = elem.offsetTop;
	for (;;) {
		elem = elem.offsetParent;
		if ((!elem) || elem == document.body) {
			break;
		}
		if (typeof(elem.offsetTop) == "number") {
			y += elem.offsetTop;
		}
	}
	return y;
}


function wt_init()
{
	// Needed?
}


function wt_register(id, obj)
// Register a widget object and give it a unique 'id' attribute.
// You can retrieve the widget later via wt_widgets[id].
{
	obj.id = id;
	wt_widgets[obj.id] = obj;
}


function log(msg)
// Log a message to the debug area, if available.
{
	var out = document.getElementById("debugout");
	if (out)
	{
		if (msg[msg.length - 1] != '\n') {
			// Make sure the line is terminated.
			msg += "\n";
		}
		out.value += msg;

		// Make sure we're scrolled to the bottom.
		out.scrollTop = out.scrollHeight;
	}
}


function wt_click(widget_id, event)
{
	if (widget_id == -1) { return; }

	var widget = wt_widgets[widget_id];
	if (widget) {
		widget.click(event);
	} else {
		alert("mousedown: no widget with id: '" + widget_id + "'!");
	}
}


function wt_mousedown(widget_id, event)
{
	if (widget_id == -1) { return; }
	
	var widget = wt_widgets[widget_id];
	if (widget) {
		widget.mousedown(event);
	} else {
		alert("mousedown: no widget with id: '" + widget_id + "'!");
	}
}

function wt_mouseup(widget_id, event)
{
	var widget;
	if (widget_id == -1) {
		// Default mouse-up.  Cancel any pending operation.
		if (wt_tracking_widget) {
			widget = wt_tracking_widget;
		} else {
			return false;
		}
	} else {
		widget = wt_widgets[widget_id];
	}

	if (widget) {
		widget.mouseup(event);
	} else {
		alert("mouseup: no widget with id: '" + widget_id + "'!");
	}
}

function wt_mousemove(widget_id, event)
{
	var widget;
	if (widget_id == -1) {
		if (wt_tracking_widget) {
			widget = wt_tracking_widget;
		} else {
			return false;
		}
	} else {
		widget = wt_widgets[widget_id];
	}

	if (widget) {
		return widget.mousemove(event);
	} else {
		alert("mousemove: no widget with id: '" + widget_id + "'!");
		return false;
	}
}

function wt_mouseout(widget_id, event)
{
	if (widget_id == -1) { return; }

	var widget = wt_widgets[widget_id];
	if (widget) {
		widget.mouseout(event);
	} else {
		alert("mouseout: no widget with id: '" + widget_id + "'!");
	}
}


// Default handlers
document.onmouseup = function(event) { return wt_mouseup(-1, event); }
document.onmousemove = function(event) { return wt_mousemove(-1, event); }
document.onmouseout = function(event)
{
	if (event.relatedTarget == undefined) {
		// Mouse left the client window -- treat it as a
		// mouse-up event, since we don't have any way to know
		// if the user stopped dragging while outside the
		// window.
		return wt_mouseup(-1, event);
	}
}


function event_handlers(id)
// Return the HTML for dispatching standard event handlers for the
// widget with the given id.
{
	return ' onmousedown=\'return wt_mousedown("' + id + '", event);\''
		+ ' onmouseup=\'return wt_mouseup("' + id + '", event);\''
		+ ' onmousemove=\'return wt_mousemove("' + id + '", event);\''
//		+ ' onmouseout="return wt_mouseout(' + id + ', event);"'
		;
}


//
// class wt_slider
//
// Numeric slider, use like:
// <script>new wt_slider("id###", "varname", initial_value, min, max);</script>


function wt_slider(id, name, val, min, max)
// Constructor for a slider widget.  Inserts necessary HTML into the
// document.
{
	wt_register(id, this);
	
	this.name = name;
	this.initial_value = val;
	this.min = min;
	this.max = max;
	this.is_tracking = false;
	this.width = 353;

	this.mousedown = function(event)
	{
		wt_tracking_widget = this;
		this.is_tracking = true;
		this.initial_value = this.slidervalue.innerHTML;
		
		this.update_slider(event);

		return false;
	}

	this.mouseup = function(event)
	{
		if (this.is_tracking)
		{
			this.is_tracking = false;
			wt_tracking_widget = undefined;
			document.forms[0].submit();
		}
		return false;
	}

	this.mousemove = function(event)
	{
		if (this.is_tracking)
		{
			this.update_slider(event);
		}
		return false;
	}

	this.mouseout = function(event)
	{
		return false;
	}

	this.update_slider = function(event)
	{
		var x = page_offset_x(event) - this.sliderbody_x - 20;

		var value = (x / this.width) * (this.max - this.min) + this.min;
		value = fclamp(value.toPrecision(3), this.min, this.max);
		
		this.set_slider_pos(value);
		this.slidervalue.innerHTML = value;
		this.hiddenvalue.value = value;
	}

	this.set_slider_pos = function(value)
	{
		var x = this.width * (value - this.min) / (this.max - this.min);
		x = fclamp(x, 0, this.width);

		this.sliderpointer.style.left = (this.sliderbody_x + x + 20 - 5) + "px";
	}

	this.stop_tracking = function()
	{
		this.is_tracking = false;
		wt_tracking_widget = undefined;
	}

	// Generate the HTML for the visible parts of the widget.
	var html = '';
	html += '<table style="border-style:solid; border-color:black; border-width:1px"><tr><td>' + name + '</td>';
	html += '<td align=right><span id="slidervalue' + this.id + '">' + val + '</span></td></tr><tr><td colspan=2>';
	html += '<input type=hidden name="' + this.id + '" value="' + this.initial_value + '"></input>';

	html += '<div id="sliderbody' + this.id + '"';
	html += event_handlers(this.id) + ' style="padding-left: 20px; padding-right: 20px">';

	// Slider line.
	html += '<hr style="width: ' + this.width + 'px; height: 28px; ';
	html += 'border: none; border-top: 13px solid #FFFFFF; border-bottom: 13px solid #FFFFFF; background-color: #A0A0A0;">';

	// Pointer
	html += '<hr style="width: 12px; height: 20px; background-color: #A0A0A0; border: 1px solid black; border-left: 1px solid #E0E0E0; border-top: 1px solid #E0E0E0; " id="sliderpointer' + this.id + '">';
	
	make_incrementer = function(inc)
	{
		var html = "<td><table cellpadding=0 cellspacing=0><tr>";
		html += '<td><span onclick=\'return 1\'><a href="javascript:void(0)"><img src="images/wt_up.png" border=0></a></span></td>';
		html += '<td rowspan=2>' + inc + '&nbsp;&nbsp;&nbsp;</td></tr><tr>';
		html += '<td><span onclick=\'return 1\'><a href="javascript:void(0)"><img src="images/wt_down.png" border=0>hi</a></span></td>';
		html += '</tr></table></td>';
		return html;
	}

	html += "<table>\n";
	html += "<tr>" + make_incrementer(1) + make_incrementer(10) + make_incrementer(100) + "</tr>";
	html += "</table>";
	
	html += '</td></tr></table>';
	html += '<style>';
	html += '#sliderpointer' + this.id + ' { position: absolute; top: 0px; left: 0px; padding: 0px; }';
	html += '</style>';

	document.write(html);
	document.close();

	// Cache references to relevent elements.
	this.sliderbody = document.getElementById("sliderbody" + this.id);
	this.sliderpointer = document.getElementById("sliderpointer" + this.id);
	this.slidervalue = document.getElementById("slidervalue" + this.id);

	this.sliderbody_x = absolute_left(this.sliderbody);
	this.sliderbody_y = absolute_top(this.sliderbody);
	this.sliderpointer.style.top = this.sliderbody_y - 4;
	
	this.hiddenvalue = document.getElementsByName(this.id)[0];

	this.set_slider_pos(this.initial_value);
}


//
// class wt_color_picker
//
// Color picker, use like:
// <script>new wt_color_picker("id###", "varname", initial_value);</script>
//
// initial_value should be in the usual html format "#RRGGBB"


function wt_color_picker(id, name, initial_value)
{
	wt_register(id, this);
	
	this.name = name;
	this.initial_value = initial_value;
	this.is_tracking = false;
	// If tracking, whether we're tracking the value slider or the
	// hue/sat area.
	this.is_tracking_slider = false;
	this.hsv_width = 360;
	this.height = 100;

	this.mouseevent = function(event)
	{
		if (this.is_tracking_slider) {
			// Deal with slider.
			var vertical = page_offset_y(event) - this.hsv_y;
			vertical = fclamp(vertical, 0, 99);

			var value = (99 - vertical) / 99;

			this.set_cursors_hsv(this.hsv[0], this.hsv[1], value);
		} else {
			// Deal with the hue/sat area.
			var hue = page_offset_x(event) - this.hsv_x;
			var vertical = page_offset_y(event) - this.hsv_y;
			hue = fclamp(hue, 0, 360 - 1);
			vertical = fclamp(vertical, 0, 99);

			// In the HSV area.
			var sat = (99 - vertical) / 99;

			this.set_cursors_hsv(hue, sat, this.hsv[2]);
		}
	}

	this.mousedown = function(event)
	{
		wt_tracking_widget = this;
		this.is_tracking = true;

		// Decide whether we're tracking the hsv area or the
		// value slider.
		var x = page_offset_x(event) - this.hsv_x;
		if (x > 365) {
			this.is_tracking_slider = true;
		} else {
			this.is_tracking_slider = false;
		}
		
		this.mouseevent(event);
		return false;
	}

	this.mouseup = function(event)
	{
		if (this.is_tracking)
		{
			this.mouseevent(event);
			this.is_tracking = false;
			wt_tracking_widget = undefined;

			// TODO submit the form!
			// xxxxx document.forms[0].submit();
		}
		return false;
	}

	this.mousemove = function(event)
	{
		if (this.is_tracking)
		{
			this.mouseevent(event);
		}
		// log("mouse move");
		return false;
	}

	this.mouseout = function(event)
	{
	}

	this.update_slider = function(event)
	{
	}

	this.cancel_tracking = function()
	{
	}


	this.stop_tracking = function()
	{
	}

	this.set_hsv_cursor_pos = function(hue, sat)
	{
	}

	this.set_value_slider_pos = function(value)
	{
		
	}

	this.html_to_hsv = function(color)
	// Returns a triple [hue, saturation, value].
	{
		var r = parseInt(color.substring(1, 3), 16);
		var g = parseInt(color.substring(3, 5), 16);
		var b = parseInt(color.substring(5, 7), 16);

		var max = r;
		if (g > max) max = g;
		if (b > max) max = b;

		var min = r;
		if (g < min) min = g;
		if (b < min) min = b;

		if (max == 0 || max == min) {
			// A shade of gray.
			var value = max / 255.0;
			return [0, 0, value];
		}

		var hue;
		if (r == max) {
			hue = (g - b) / (max - min) * 60;
		} else if (g == max) {
			hue = (2 + (b - r) / (max - min)) * 60;
		} else {
			hue = (4 + (r - g) / (max - min)) * 60;
		}
		if (hue < 0) hue += 360;

		var saturation = (max - min) / max;
		var value = max / 255.0;

		return [ hue, saturation, value ];
	}

	this.hsv_to_html = function(hue, sat, val)
	// generate a HTML color given hue, saturation and value in [0,1]
	{
		if (hue >= 360.0) hue -= 360;
		
		var hi = Math.floor(hue / 60);
		var f = (hue / 60) - hi;
		var p = val * (1 - sat);
		var q = val * (1 - f * sat);
		var t = val * (1 - (1 - f) * sat);
		var r, g, b;
		if (hi == 0) {
			r = val; g = t; b = p;
		} else if (hi == 1) {
			r = q; g = val; b = p;
		} else if (hi == 2) {
			r = p; g = val; b = t;
		} else if (hi == 3) {
			r = p; g = q; b = val;
		} else if (hi == 4) {
			r = t; g = p; b = val;
		} else {
			r = val; g = p; b = q;
		}

		r = Math.floor(r * 255 + 0.5).toString(16);
		g = Math.floor(g * 255 + 0.5).toString(16);
		b = Math.floor(b * 255 + 0.5).toString(16);

		if (r.length < 2) r = '0' + r;
		if (g.length < 2) g = '0' + g;
		if (b.length < 2) b = '0' + b;

		var color = '#' + r.toString(16) + g.toString(16) + b.toString(16);

		return color;
	}

	this.set_cursors_sub = function(h, s, v)
	// Position the actual cursors.  Use the wrappers to also set
	// the color swatch.
	{
		this.hsv = [ h, s, v ];
		
		this.hsv_cursor.style.left = (h / 360 * 360) + this.hsv_x - 4;
		this.hsv_cursor.style.top = (1 - s) * 99 + this.hsv_y - 12;

		this.sliderpointer.style.top = (1 - v) * (this.height - 1) + this.hsv_y - 14;
	}

	this.set_cursors_hsv = function(h, s, v)
	// Position the cursors according to the given hsv color.
	{
		var color = this.hsv_to_html(h, s, v);
		this.color_value.style.backgroundColor = color;

		this.set_cursors_sub(h, s, v);
	}

	this.set_cursors = function(color)
	// Position the cursors according to the given (html) color.
	{
		var hsv = this.html_to_hsv(color);

		var h = hsv[0];
		var s = hsv[1];
		var v = hsv[2];

		this.color_value.style.backgroundColor = color;
		
		this.set_cursors_sub(h, s, v);
	}
	

	// Generate the HTML for the visible parts of the widget.
	var html = '';
	html += '<table style="border-style:solid; border-color:black; border-width:1px"><tr>';
	html += '<td>' + name + '</td><td width=100px id="colorvalue' + this.id + '" ';
	html += 'style="background-color: red;">';
	html += '</td><td></td></tr><tr><td colspan=2>';
	html += '<input type=hidden name="' + this.id + '" value=""></input>';

	// Color Hue/Sat picker.
	html += '<div id="hsv_body' + this.id + '" style="background-image: url(\'images/wt_hsv.png\'); width: 360px; height: 100px;" ';
	html += event_handlers(this.id) + '></div>';

	// HSV cursor.
	html += '<hr id="hsv_cursor' + this.id + '" style="position:absolute; border: 2px solid black; height: 9px; width: 9px" ';
	html += event_handlers(this.id) + '>';

	html += '</td><td>';
	
	// Color value picker.
	html += '<span id="sliderbody' + this.id + '"';
	html += event_handlers(this.id) + '>';

	// Line.
	html += '<hr id="sliderline' + this.id + '" style="width: 28px; height: 100px; ';
	html += 'border: none; border-left: 13px solid #FFFFFF; border-right: 13px solid #FFFFFF; background-color: #A0A0A0;">';
	
	// Pointer.
	html += '<hr style="width: 20px; height: 12px; background-color: #A0A0A0; border: 1px solid black; border-left: 1px solid #E0E0E0; border-top: 1px solid #E0E0E0; " id="sliderpointer' + this.id + '">';

	html += '</span>';
	html += '</td></tr></table>';
	html += '<style>';
	html += '#sliderpointer' + this.id + ' { position: absolute; top: 0px; left: 0px; padding: 0px; }';
	html += '</style>';

	document.write(html);
	document.close();

	// Cache references to relevent elements.
	this.hsv_body = document.getElementById("hsv_body" + this.id);
	this.hsv_x = absolute_left(this.hsv_body);
	this.hsv_y = absolute_top(this.hsv_body);

	this.color_value = document.getElementById("colorvalue" + this.id);
	this.hsv_cursor = document.getElementById("hsv_cursor" + this.id);
	this.sliderline = document.getElementById("sliderline" + this.id);
	this.sliderpointer = document.getElementById("sliderpointer" + this.id);
	this.hiddenvalue = document.getElementsByName(this.id)[0];

//	this.hsv_cursor.style.left = this.hsv_x + 50;
//	this.hsv_cursor.style.top = this.hsv_y + 50;
	
	this.sliderline_x = absolute_left(this.sliderline);
	this.sliderline_y = absolute_top(this.sliderline);
	this.sliderpointer.style.top = this.sliderline_y + 1;
	this.sliderpointer.style.left = this.sliderline_x + 4;
	
	this.set_cursors(this.initial_value);
}


//
// class wt_tree_node
//
// A node in a tree widget.  Can be open or closed, according to
// the third arg.  Use like:
//
// <script>new wt_tree_node("id###", "label", 1);</script>


function wt_tree_node(id, label, open)
// Constructor for a tree node label.  Inserts necessary HTML into the
// document.
//
// TODO needs to take id as a param.
{
	wt_register(id, this);
	
	this.name = label;
	this.open_state = open ? 1 : 0;

	this.click = function(event)
	{
		this.open_state = 1 - this.open_state;
		this.hiddenvalue.value = this.open_state;
		document.forms[0].submit();
	}

	// Generate the HTML for the visible parts of the widget.
	var html = '';
	html += '<div id="tree_node' + this.id + '" onclick=\'return wt_click("' + this.id + '", event);\'>'
	html += '<a href="javascript:void(0)" style="text-decoration: none;">';
	if (this.open_state) {
		html += "- ";
	} else {
		html += "+ ";
	}
	html += label + '</a></div>';
	html += '<input type=hidden name="' + this.id + '" value="' + this.open_state + '"></input>';

	document.write(html);
	document.close();

	this.hiddenvalue = document.getElementsByName(this.id)[0];
}

