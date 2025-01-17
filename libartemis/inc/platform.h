// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "ec_types.h"

//#define SHOW_FAILURES
//#define ENABLE_TESTS
//#define ENABLE_FUZZING

/////////////////////////////
#define AR_STR_HELPER(x) #x
#define AR_STR(x) AR_STR_HELPER(x)

/////////////////////////////
#if !defined(PLATFORM_PRAGMAS)
	#define PLATFORM_PRAGMAS
	#if defined(_WINDOWS)
		#define PRAGMA_PUSH
		#define PRAGMA_POP
		#define PRAGMA_OPTIMIZE		__pragma(optimize( "g", on ))
		#define PRAGMA_WARN(x)		__pragma(warning(x))
	#elif defined(__ANDROID__)
		#define PRAGMA_PUSH
		#define PRAGMA_POP
		#define PRAGMA_OPTIMIZE
		#define PRAGMA_WARN(x)
	#else
		#define PRAGMA_PUSH			_Pragma("GCC push_options")
		#define PRAGMA_POP			_Pragma("GCC pop_options")
		#define PRAGMA_OPTIMIZE		_Pragma("GCC optimize(\"O3\")")
		#define PRAGMA_WARN(x)
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
		#define DEBUGHALT (0)
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

	#define INLINE __inline
	#define DLLDECL __declspec(dllexport)
	#define STACKGAP() _alloca( 8 * ar_util_rnd4() )

	#if !defined(strdup)
		#define strdup _strdup
	#endif

	char *strndup(const char *s, size_t n);

	#define DEPRECATED __declspec(deprecated)

#else // win

	#include <stddef.h>

	#define INLINE __attribute__((always_inline))
	#define DLLDECL __attribute__((visibility ("default")))
	#define STACKGAP() alloca( 8 * ar_util_rnd4() )

	#define max(a,b) ((a)>(b)?(a):(b))

	char *strdup(const char *s);
	char *strndup(const char *s, size_t n);
	
	#define DEPRECATED __attribute__((deprecated))

#endif // win

#endif // _PLATFORM_H_
