// Copyright 2014 John Howard (orthopteroid@gmail.com)

#if defined(_WINDOWS)

#define _CRT_RAND_S
#include "stdlib.h"

#else

#include "stdlib.h"

#endif

//#include <time.h>
//#include <ctype.h>

#include "platform.h"
#include "version.h"

#include "ec_field.h"

static word32 testendianness()
{
	static byte x[4] = {1,2,3,4};
#ifdef LITTLE_ENDIAN
	if ( *(word32*)x != 0x04030201 )
	{
		fputs( "# libartemis error: expected BIG_ENDIAN, found LITTLE_ENDIAN\n", stderr );
		ASSERT(0);
		return 1;
	}
#else
	if ( *(word32*)x != 0x01020304 )
	{
		fputs( "# libartemis error: expected LITTLE_ENDIAN, found BIG_ENDIAN\n", stderr );
		ASSERT(0);
		return 1;
	}
#endif

	return 0;
}

//////////////////

word32 platform_init()
{
	if( testendianness() ) return 1;

	srand( 1 );
	gfInit();

	return 0;
}

void platform_cleanup()
{
	gfQuit();
}

int platform_isdebug()
{
#if defined(_DEBUG)
	return 1;
#else // _DEBUG
	return 0;
#endif // _DEBUG
}

int platform_isdemo()
{
	return AR_DEMO;
}

word32 platform_vmajor()
{
	return AR_VMAJOR;
}

word32 platform_vminor()
{
	return AR_VMINOR;
}

word32 platform_keylength()
{
	return AR_KEYLENGTH;
}

word32 platform_rnd32()
{

#if defined(_WINDOWS)

	word32 r;
	rand_s( &r );
	return r;
	
#else

	return rand();

#endif

}

