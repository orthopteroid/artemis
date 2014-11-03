
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar_codes.h"

#include "ar_uricodec.h"
#include "ar_core.h"
#include "ar_util.h"
#include "library.h"
#include "test.h"

// -p -l foo.bar -d < x  -s 5 -t 3 -c "000|111|222|333|444|555" -m "hello world"  > x

int ar_main(int argc, char **argv);

#if defined(_WINDOWS)

// declared elsewhere

#else

#define strncpy_s(a,b,c,d) strncpy(a,c,d)
#define sscanf_s sscanf

int main(int argc, char* argv[])
{
	return ar_main( argc, argv );
}

#endif

static void* realloc_zero(void* pBuffer, size_t oldSize, size_t newSize) {
  void* pNew = realloc(pBuffer, newSize);
  if ( newSize > oldSize && pNew ) {
    size_t diff = newSize - oldSize;
    void* pStart = ((char*)pNew) + oldSize;
    memset(pStart, 0, diff);
  }
  return pNew;
}

word32 app_vmajor = 0;
word32 app_vminor = 1;

int ar_main(int argc, char **argv)
{
	int rc = 0;
	char* messageArg = 0;
	char* clueData = 0;
	char* location = 0;
	unsigned int shares = 0, threshold = 0, testmode = 0, pause = 0, decode = 0;

	if( argc == 1 ) { goto HELP; }

	for( int i=1; i<argc; i++ )
	{
		if( strlen( argv[i] ) == 2 && argv[i][0] == '-' )
		{
			switch( argv[i][1] )
			{
			case 'h':
HELP:
				printf("(artemis v%u.%u, libartemis v%u KL%u %s)\n", app_vmajor, app_vminor, library_version(), library_keylength(), library_isdemo() ? "crippleware" : "");
				printf("usage: -h | -z | -d <newline delimited data> | -l <locationURL> -s <sharesize> -t <thresholdsize> [ -c \"<topiclue>|<shareclue1>|...|<shareclueN>\" ] -m \"<textmessage>\"\n");
				return 0;
				break;
			case 'p':
				pause = 1;
				break;
			case 'd':
				decode = 1;
				break;
			case 'z':
				testmode = 1;
				break;
			case 'c':
				if( ++i >= argc ) { return 1; }
				clueData = malloc( strlen( argv[i] ) + 1 );
				strncpy_s( clueData, strlen( argv[i] ) + 1, argv[i], strlen( argv[i] ) );
				break;
			case 'm':
				if( ++i >= argc ) { return 1; }
				messageArg = argv[i];
				break;
			case 'l':
				if( ++i >= argc ) { return 1; }
				location = argv[i];
				break;
			case 's':
				if( ++i >= argc ) { return 1; }
				sscanf_s( argv[i], "%ud", &shares );
				break;
			case 't':
				if( ++i >= argc ) { return 1; }
				sscanf_s( argv[i], "%ud", &threshold );
				break;
			}
		}
	}

	if( library_init() )
	{
		printf("# platform init fail\n");
		goto FAILPLATFORM;
	}

	if( testmode )
	{
		test_all();
	}
	else if( decode )
	{
		byteptr message_out = 0;
		byteptr shareArr = 0;

		size_t bufsize = 1024;
		if( !(shareArr = malloc( bufsize )) ) { ASSERT(0); rc = RC_MALLOC; goto FAIL_DECRYPT; }

		size_t shareArrLen = 0;
		while( 1 )
		{
			char ch = getc( stdin ); // must be \n delim'd
			if( ch == EOF ) { shareArr[shareArrLen] = 0; break; }
			shareArr[shareArrLen++] = ch;
			if( shareArrLen == bufsize -1 )
			{
				size_t newbufsize = bufsize * 2;
				rc = (shareArr = realloc_zero( shareArr, bufsize, newbufsize )) ? 0 : RC_MALLOC;
				bufsize = newbufsize;
				if( rc ) { printf("# decrypt error\n"); goto FAIL_DECRYPT; }
			}
		}

		rc = library_uri_decoder( &message_out, location, shareArr );
		if( rc ) { printf("# decrypt error\n"); goto FAIL_DECRYPT; }

		printf( "%s\n", message_out );

FAIL_DECRYPT:

		if( message_out ) library_free( &message_out );
		if( shareArr ) free( shareArr );
	}
	else
	{
		byteptr recordArr = 0;

		size_t clueStrLen = strlen( clueData );
		for( size_t i = 0; i < clueStrLen; i++ ) { if( clueData[i]=='|' ) { clueData[i]='\n'; } }

		rc = library_uri_encoder( &recordArr, shares, threshold, location, clueData, messageArg );
		if( rc ) { printf("# encrypt error\n"); goto FAIL_CRYPT; }

		printf( "%s\n", recordArr );

FAIL_CRYPT:

		if( recordArr ) library_free( &recordArr );
	}

FAILPLATFORM:

	library_cleanup();

	if( pause ) getchar();

	return rc ? 1 : 0 ;
}
