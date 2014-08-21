// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
	#if defined(_M_IX86) || defined(_M_AMD64) || defined(_M_I86) || defined(__alpha)
		#define LITTLE_ENDIAN
	#else
		#define BIG_ENDIAN
	#endif
#endif

#if defined(_WIN32) || defined(_WIN64)

	#include <crtdefs.h>

	//#if defined(WINDLL_EXPORTS)
	#define DLLDECL __declspec(dllexport)
	//#else // WINDLL_EXPORTS
	//#define DLLDECL __declspec(dllimport)
	//#endif // WINDLL_EXPORTS

	#if defined(_DEBUG)
		#define ASSERT( c ) do { if( !(c) ) { __debugbreak(); } } while( 0 )
	#else // debug
		#define ASSERT(...) (0)
	#endif // debug

#else // win

	#include <stddef.h>

	#define DLLDECL

	#include <assert.h>
	#define ASSERT(...) assert( __VA_ARGS__ )

	#define max(a,b) ((a)>(b)?(a):(b))
	#define memcpy_s(a,b,c,d) memcpy(a,c,d)
	#define strcat_s(a,b,c) strcat(a,c)

#endif // win

///////////////////////

DLLDECL void platform_init();

DLLDECL void platform_cleanup();

DLLDECL int platform_isdebug();

DLLDECL int platform_isdemo();

DLLDECL word32 platform_vmajor();
DLLDECL word32 platform_vminor();
DLLDECL word32 platform_keylength();

word32 platform_rnd32();

#endif // _PLATFORM_H_
