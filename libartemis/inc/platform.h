// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

//#define SHOW_FAILURES
//#define ENABLE_FUZZING

/////////////////////////////
#define AR_STR_HELPER(x) #x
#define AR_STR(x) AR_STR_HELPER(x)

/////////////////////////////
#if !defined(PLATFORM_PRAGMAS)
	#define PLATFORM_PRAGMAS
	#if defined(_WINDOWS)
		#define PRAGMA_SOMETHING __pragma("something")
		#define PRAGMA_PUSH
		#define PRAGMA_POP
		#define PRAGMA_O3
	#elif defined(__ANDROID__)        
		#define PRAGMA_PUSH
		#define PRAGMA_POP
		#define PRAGMA_O3
	#else
		#define PRAGMA_PUSH _Pragma("GCC push_options")
		#define PRAGMA_POP _Pragma("GCC pop_options")
		#define PRAGMA_O3 _Pragma("GCC optimize(\"O3\")")
	#endif
#endif

/////////////////////////////
#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
	#if defined(_WINDOWS)
		#if defined(_M_IX86) || defined(_M_AMD64) || defined(_M_I86) || defined(__alpha)
			#define LITTLE_ENDIAN
		#else
			#define BIG_ENDIAN
		#endif
	#else
		#if defined(__BYTE_ORDER__)
			#if (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
				#define BIG_ENDIAN
			#else
				#define LITTLE_ENDIAN
			#endif
		#else
			int CompilerEndianProblems[-1];
		#endif
	#endif
#endif

/////////////////////////////
#if !defined(DEBUGPRINT)
	#if defined(_DEBUG)
		#if defined(__ANDROID__)
			#include <android/log.h>
			#define DEBUGPRINT(...) __android_log_print(ANDROID_LOG_INFO, "libartemis", __VA_ARGS__ );
		#else
			#define DEBUGPRINT(...) printf(__VA_ARGS__)
		#endif
	#else
		#define DEBUGPRINT(...) (0)
	#endif
#endif

/////////////////////////////
#if !defined(DEBUGHALT)
	#if defined(_DEBUG)
		#if defined(_WINDOWS)
			#define DEBUGHALT __debugbreak()
		#else
			#include <assert.h>
			#define DEBUGHALT assert(0)
		#endif
	#else
		#define DEBUGPRINT (0)
	#endif
#endif

/////////////////////////////
#if !defined(ASSERT)
	#if defined(_DEBUG)
		#if defined(__ANDROID__)
			#define ASSERT( c ) do { if( !(c) ) { DEBUGPRINT("ASSERT Failed %s line %d", __FILE__, __LINE__ ); DEBUGHALT; } } while(0)
		#else
			#define ASSERT( c ) do { if( !(c) ) { DEBUGHALT; } } while( 0 )
		#endif
	#else
		#define ASSERT( c ) (0)
	#endif
#endif

// TODO: rip
#if !defined(TESTASSERT)
	#define TESTASSERT(c) ASSERT(c)
#endif

/////////////////////////////
#if !defined(STATICASSERT)
	#define STATICASSERT_CONCAT(x,y) x ## y
	#define STATICASSERT_SUBST(x,y) STATICASSERT_CONCAT(x,y)
	#if defined(_DEBUG)
		#define STATICASSERT( c ) static char STATICASSERT_SUBST( static_assert, __COUNTER__ ) [ (c) ? 1 : -1 ] = {0};
	#else
		#define STATICASSERT( c ) static char STATICASSERT_SUBST( static_assert, __COUNTER__ ) [ 1 ] = {0};
	#endif
#endif

/////////////////////////////
#if !defined(LOGFAIL)
	#if defined(_DEBUG)
		int ar_util_istest();
		const char* ar_util_rclookup(int);
		#if defined(__ANDROID__)
			// halt not allowed on android
			#define LOGFAIL(c) do { DEBUGPRINT("LOGFAIL %s at %s line %d\n", ar_util_rclookup(c), __FILE__, __LINE__); } while( 0 )
		#else
			#if defined(SHOW_FAILURES)
				#define LOGFAIL(c) do { if( !ar_util_istest() ) { DEBUGHALT; } else DEBUGPRINT("LOGFAIL %s at %s line %d\n", ar_util_rclookup(c), __FILE__, __LINE__); } while( 0 )
			#else
				#define LOGFAIL(c) do { if( !ar_util_istest() ) { DEBUGHALT; } } while( 0 )
			#endif
		#endif
	#else
		#define LOGFAIL(c) (0)
	#endif
#endif

///////////////////

#if defined(_WINDOWS)

	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>

	#include <crtdefs.h>

	//#if defined(WINDLL_EXPORTS)
	#define DLLDECL __declspec(dllexport)
	//#else // WINDLL_EXPORTS
	//#define DLLDECL __declspec(dllimport)
	//#endif // WINDLL_EXPORTS

	// TODO: remove?
	#if defined(_DEBUG)
		DLLDECL char* RC_LOOKUP( int rc );
	#endif

	#if !defined(strdup)
		#define strdup _strdup
	#endif

	char *strndup(const char *s, size_t n);

	#define DEPRECATED __declspec(deprecated)

#else // win

	#include <stddef.h>

	#define DLLDECL

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
