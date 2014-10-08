// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

//#define ENABLE_TESTFAIL
//#define SHOW_FAILURES

#if defined(_WINDOWS)

	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

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
		DLLDECL char* RC_LOOKUP( int rc );
	#endif

	#if !defined(LOGFAIL)
		#if defined(_DEBUG)
			#if defined(SHOW_FAILURES)
				#define LOGFAIL(c) do { if( !ar_util_istest() ) { __debugbreak(); } else printf("LOGFAIL %s at %s line %d\n", ar_util_rclookup(c), __FILE__, __LINE__); } while( 0 )
			#else
				#define LOGFAIL(c) do { if( !ar_util_istest() ) { __debugbreak(); } } while( 0 )
			#endif
		#else // debug
			#define LOGFAIL(c) (0)
		#endif // debug
	#endif

	#if !defined(ASSERT)
		#if defined(_DEBUG)
			#include <assert.h>
			#define ASSERT( c ) do { if( !(c) ) { __debugbreak(); } } while( 0 )
		#else // debug
			#define ASSERT( c ) (0)
		#endif // debug
	#endif

	#if !defined(TESTASSERT)
		#if defined(_DEBUG)
			#define TESTASSERT( c ) do { if( !(c) ) { __debugbreak(); } } while( 0 )
		#else // debug
			#define TESTASSERT( c ) (0)
		#endif // debug
	#endif

	#if !defined(strdup)
		#define strdup _strdup
	#endif

	char *strndup(const char *s, size_t n);

	#define DEPRECATED __declspec(deprecated)

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

	int ar_util_istest();

	#if !defined(LOGFAIL)
		#if defined(NDK_DEBUG)
			#define LOGFAIL(c) __android_log_print(ANDROID_LOG_INFO, "libartemis", "LOGFAIL %s at %s line %d", ar_util_rclookup(c), __FILE__, __LINE__ )
		#elif defined(_DEBUG)
			#if defined(SHOW_FAILURES)
				#include <assert.h>
				#define LOGFAIL(c) do { printf( "LOGFAIL %s at %s line %d\n", ar_util_rclookup(c), __FILE__, __LINE__ ); if( !ar_util_istest() ) { assert(0); } } while(0)
			#else
				#include <assert.h>
				#define LOGFAIL(c) do { if( !ar_util_istest() ) { printf( "LOGFAIL %s at %s line %d\n", ar_util_rclookup(c), __FILE__, __LINE__ ); assert(0); } } while(0)
			#endif
		#else // debug
			#define LOGFAIL(c) (0)
		#endif // debug
	#endif

	#if !defined(ASSERT)
		#if defined(NDK_DEBUG)
			#include <assert.h>
			#define ASSERT( c ) do { if( !(c) ) { __android_log_print(ANDROID_LOG_INFO, "libartemis", "ASSERT Failed %s line %d", __FILE__, __LINE__ ); assert(0); } } while(0)
		#elif defined(_DEBUG)
			#include <assert.h>
			#define ASSERT( c ) assert( c )
		#else // debug
			#define ASSERT( c ) (0)
		#endif // debug
	#endif

	#if !defined(TESTASSERT)
		#if defined(_DEBUG)
			#include <assert.h>
			#define TESTASSERT( c ) assert( c )
		#else // debug
			#define TESTASSERT( c ) (0)
		#endif // debug
	#endif

	#define max(a,b) ((a)>(b)?(a):(b))
	#define memcpy_s(a,b,c,d) memcpy(a,c,d)
	#define strcpy_s(a,b,c) strcpy(a,c)
	#define strncpy_s(a,b,c,d) strncpy(a,c,d)
	#define strcat_s(a,b,c) strcat(a,c)
	#define strncat_s(a,b,c,d) strncat(a,c,d)

	char *strdup(const char *s);
	char *strndup(const char *s, size_t n);
	
	#define DEPRECATED __attribute__((deprecated))

#endif // win

#endif // _PLATFORM_H_
