// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

#if defined(_WINDOWS)

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

	#if !defined(ASSERT)
		#if defined(_DEBUG)
			#define ASSERT( c ) do { if( !(c) ) { __debugbreak(); } } while( 0 )
		#else // debug
			#define ASSERT(...) (0)
		#endif // debug
	#endif

	#define _strdup strdup
	#define _strndup strndup

#else // win

    #if defined(__ANDROID__)
        #include <android/log.h>
        
        #if defined(NDK_DEBUG) && !defined(_DEBUG)
            #define _DEBUG
        #endif

        #define PRAGMA_PUSH
        #define PRAGMA_POP
        #define PRAGMA_O3
    #else
        #define PRAGMA_PUSH _Pragma("GCC push_options")
        #define PRAGMA_POP _Pragma("GCC pop_options")
        #define PRAGMA_O3 _Pragma("GCC optimize(\"O3\")")
    #endif
	
	#if defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
		#define BIG_ENDIAN
	#else
		#define LITTLE_ENDIAN
	#endif

	#include <stddef.h>

	#define DLLDECL

	#if !defined(ASSERT)
		#if defined(_DEBUG)
			#include <assert.h>
			#define ASSERT(...) assert( __VA_ARGS__ )
		#else // debug
			#define ASSERT(...) (0)
		#endif // debug
	#endif

	#define max(a,b) ((a)>(b)?(a):(b))
	#define memcpy_s(a,b,c,d) memcpy(a,c,d)
	#define strcpy_s(a,b,c) strcpy(a,c)
	#define strcat_s(a,b,c) strcat(a,c)
	#define strncat_s(a,b,c,d) strncat(a,c,d)

	char *strdup(const char *s);
	char *strndup(const char *s, size_t n);
	
#endif // win

#endif // _PLATFORM_H_
