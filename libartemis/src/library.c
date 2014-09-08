// Copyright 2014 John Howard (orthopteroid@gmail.com)


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_WINDOWS)
	#define _CRT_RAND_S
#endif

#include "stdlib.h"

#include "platform.h"
#include "version.h"

#include "ec_field.h"
#include "ar_uricodec.h"
#include "ar_core.h"

#if defined(__ANDROID__)
	#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);
#endif

static word32 testendianness()
{
	static byte x[4] = {1,2,3,4};
#ifdef LITTLE_ENDIAN
	if ( *(word32*)x != 0x04030201 )
	{
#if defined(_DEBUG)
		printf( "# libartemis: expected BIG_ENDIAN, found LITTLE_ENDIAN\n" );
#endif
		ASSERT(0);
		return 1;
	}

#if defined(_DEBUG)
   	printf( "# libartemis: BIG_ENDIAN\n" );
#endif

#else
	if ( *(word32*)x != 0x01020304 )
	{
#if defined(_DEBUG)
		printf( "# libartemis: expected LITTLE_ENDIAN, found BIG_ENDIAN\n" );
#endif
		ASSERT(0);
		return 1;
	}
	
#if defined(_DEBUG)
	printf( "# libartemis: LITTLE_ENDIAN\n" );
#endif

#endif

	return 0;
}

//////////////////

word32 library_init()
{
	if( testendianness() ) return 1;

	srand( 1 );
	gfInit();

	return 0;
}

void library_cleanup()
{
	gfQuit();
}

int library_isdebug()
{
#if defined(_DEBUG)
	return 1;
#else // _DEBUG
	return 0;
#endif // _DEBUG
}

int library_isdemo()
{
	return AR_DEMO;
}

word32 library_vmajor()
{
	return AR_VMAJOR;
}

word32 library_vminor()
{
	return AR_VMINOR;
}

word32 library_keylength()
{
	return AR_KEYLENGTH;
}

