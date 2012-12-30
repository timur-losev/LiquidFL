# Makefile	-- Thatcher Ulrich <tu@tulrich.com> 2003

# This source code has been donated to the Public Domain.  Do
# whatever you want with it.

# Makefile for gameswf library & tests.

TOP = ..
include $(TOP)/config


INCLUDE_PATHS := $(ENGINE_INCLUDE) $(ZLIB_INCLUDE) $(SDL_MIXER_INCLUDE) $(LIBMAD_INCLUDE) # $(XML2_INCLUDE) 

CFLAGS := $(CFLAGS) $(INCLUDE_PATHS) $(CC_DEBUG_FLAGS)
LDFLAGS := $(SDL_LDFLAGS) $(LIB_DEBUG_FLAGS) $(LDFLAGS)

LIB_OUT = $(LIB_PRE)gameswf.$(LIB_EXT)
EXE_OUT = gameswf_test_ogl$(EXE_EXT)
PARSER_OUT = gameswf_parser$(EXE_EXT)
PROCESSOR_OUT = gameswf_processor$(EXE_EXT)

LIBS := $(LIB_OUT) $(BASE_LIB) $(NET_LIB) $(LIBS) $(JPEGLIB) $(ZLIB) $(SDL_MIXER_LIB) $(LIBMAD_LIB) # $(XML2LIB)

# Embedders: if you don't want network support, then leave out
# SOCKET_LIBS and don't reference new_tu_net_file().
LIBS := $(LIBS) $(SOCKET_LIBS)

all: base_lib net_lib $(LIB_OUT) $(EXE_OUT) $(PARSER_OUT) $(PROCESSOR_OUT)


LIB_OBJS = \
	gameswf_as_classes/as_array.$(OBJ_EXT) \
	gameswf_as_classes/as_boolean.$(OBJ_EXT) \
	gameswf_as_classes/as_broadcaster.$(OBJ_EXT) \
	gameswf_as_classes/as_class.$(OBJ_EXT) \
	gameswf_as_classes/as_color.$(OBJ_EXT) \
	gameswf_as_classes/as_color_transform.$(OBJ_EXT) \
	gameswf_as_classes/as_date.$(OBJ_EXT) \
	gameswf_as_classes/as_event.$(OBJ_EXT) \
	gameswf_as_classes/as_flash.$(OBJ_EXT) \
	gameswf_as_classes/as_geom.$(OBJ_EXT) \
	gameswf_as_classes/as_global.$(OBJ_EXT) \
	gameswf_as_classes/as_key.$(OBJ_EXT) \
	gameswf_as_classes/as_loadvars.$(OBJ_EXT) \
	gameswf_as_classes/as_math.$(OBJ_EXT)	\
	gameswf_as_classes/as_matrix.$(OBJ_EXT)	\
	gameswf_as_classes/as_mcloader.$(OBJ_EXT)	\
	gameswf_as_classes/as_mouse_event.$(OBJ_EXT)	\
	gameswf_as_classes/as_netconnection.$(OBJ_EXT)	\
	gameswf_as_classes/as_netstream.$(OBJ_EXT)	\
	gameswf_as_classes/as_number.$(OBJ_EXT)	\
	gameswf_as_classes/as_point.$(OBJ_EXT) \
	gameswf_as_classes/as_selection.$(OBJ_EXT) \
	gameswf_as_classes/as_sharedobject.$(OBJ_EXT) \
	gameswf_as_classes/as_sound.$(OBJ_EXT) \
	gameswf_as_classes/as_string.$(OBJ_EXT) \
	gameswf_as_classes/as_textformat.$(OBJ_EXT) \
	gameswf_as_classes/as_transform.$(OBJ_EXT) \
	gameswf_as_classes/as_xml.$(OBJ_EXT) \
	gameswf_as_classes/as_xmlsocket.$(OBJ_EXT) \
	gameswf_abc.$(OBJ_EXT)	\
	gameswf_action.$(OBJ_EXT)	\
	gameswf_avm2.$(OBJ_EXT)	\
	gameswf_as_sprite.$(OBJ_EXT)	\
	gameswf_button.$(OBJ_EXT)	\
	gameswf_canvas.$(OBJ_EXT)	\
	gameswf_character.$(OBJ_EXT)	\
	gameswf_dlist.$(OBJ_EXT)	\
	gameswf_disasm.$(OBJ_EXT)	\
	gameswf_environment.$(OBJ_EXT)	\
	gameswf_filters.$(OBJ_EXT)		\
	gameswf_font.$(OBJ_EXT)		\
	gameswf_freetype.$(OBJ_EXT)	\
	gameswf_function.$(OBJ_EXT)	\
	gameswf_impl.$(OBJ_EXT)		\
	gameswf_listener.$(OBJ_EXT)	\
	gameswf_log.$(OBJ_EXT)		\
	gameswf_morph2.$(OBJ_EXT)	\
	gameswf_movie_def.$(OBJ_EXT)	\
	gameswf_object.$(OBJ_EXT)	\
	gameswf_player.$(OBJ_EXT)	\
	gameswf_render.$(OBJ_EXT)	\
	gameswf_root.$(OBJ_EXT)		\
	gameswf_shape.$(OBJ_EXT)	\
	gameswf_sound.$(OBJ_EXT)	\
	gameswf_sprite.$(OBJ_EXT)	\
	gameswf_sprite_def.$(OBJ_EXT)	\
	gameswf_stream.$(OBJ_EXT)	\
	gameswf_styles.$(OBJ_EXT)	\
	gameswf_tesselate.$(OBJ_EXT)	\
	gameswf_text.$(OBJ_EXT)		\
	gameswf_tools.$(OBJ_EXT)	\
	gameswf_types.$(OBJ_EXT)	\
	gameswf_value.$(OBJ_EXT)	\
	gameswf_video_impl.$(OBJ_EXT)	\


TEST_PROGRAM_OBJS = \
	gameswf_test_ogl.$(OBJ_EXT)             \
	gameswf_render_handler_ogl.$(OBJ_EXT)	\
	gameswf_sound_handler_sdl.$(OBJ_EXT)	\

PARSER_OBJS = \
	gameswf_parser.$(OBJ_EXT) 

PROCESSOR_OBJS = \
	gameswf_processor.$(OBJ_EXT)

OBJS = $(LIB_OBJS) $(TEST_PROGRAM_OBJS) $(PARSER_OBJS) $(PROCESSOR_OBJS)

gameswf_impl.$(OBJ_EXT): gameswf.h gameswf_impl.h gameswf_types.h

$(LIB_OUT): $(LIB_OBJS)
	$(AR) $(LIB_OUT_FLAG)$@ $^


$(EXE_OUT): $(TEST_PROGRAM_OBJS) $(LIB_OUT) $(BASE_LIB) $(NET_LIB)
	$(CC) -o $@ $(TEST_PROGRAM_OBJS) $(LIBS) $(LDFLAGS)


$(PARSER_OUT): $(PARSER_OBJS) $(LIB_OUT) $(BASE_LIB) $(NET_LIB)
	$(CC) -o $@ $(PARSER_OBJS) $(LIBS) $(LDFLAGS)


$(PROCESSOR_OUT): $(PROCESSOR_OBJS) $(LIB_OUT) $(BASE_LIB) $(NET_LIB)
	$(CC) -o $@ $(PROCESSOR_OBJS) $(LIBS) $(LDFLAGS)


clean:
	make -C $(TOP)/base clean
	-rm $(OBJS) $(LIB_OUT) $(EXE_OUT) $(PARSER_OUT) $(PROCESSOR_OUT)

depend:
	makedepend -Y -I.. -f Makefile *.cpp

fresh: clean all
