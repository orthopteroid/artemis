// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

#if defined(_WIN32) || defined(_WIN64)

	#define PRAGMA_SOMETHING __pragma("something")
	#define PRAGMA_PUSH
	#define PRAGMA_POP
	#define PRAGMA_O3

	#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
		#if defined(_M_IX86) || defined(_M_AMD64) || defined(_M_I86) || defined(__alpha)
			#define LITTLE_ENDIAN
		#else
			#define BIG_ENDIAN
		#endif
	#endif

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

	#define PRAGMA_PUSH _Pragma("GCC push_options")
	#define PRAGMA_POP _Pragma("GCC pop_options")
	#define PRAGMA_O3 _Pragma("GCC optimize(\"O3\")")
	
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define BIG_ENDIAN
	#else
		#define LITTLE_ENDIAN
	#endif

	#include <stddef.h>

	#define DLLDECL

	#include <assert.h>
	#define ASSERT(...) assert( __VA_ARGS__ )

	#define max(a,b) ((a)>(b)?(a):(b))
	#define memcpy_s(a,b,c,d) memcpy(a,c,d)
	#define strcpy_s(a,b,c) strcpy(a,c)
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
