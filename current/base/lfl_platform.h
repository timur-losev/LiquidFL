// lfl_config.h - by Sergey 'iOrange' Kudlay <programmist.c@gmail.com> 2010

// cross-platform stuff

#ifndef	__lfl_platform_h__
#define	__lfl_platform_h__

#if defined(_WIN32) || defined(_LINUX)
#	define	LFL_OPENGL_HEADER	<gl/gl.h>
#elif defined (_MACOSX)
#	define	LFL_OPENGL_HEADER	<OpenGL/gl.h>
#else
#	error Have no opengl headers for your platform
#endif

#endif	// __lfl_platform_h__
