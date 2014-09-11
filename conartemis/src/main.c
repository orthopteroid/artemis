
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
				printf("(artemis v%u.%u, libartemis v%u.%u KL%u %s)\n", app_vmajor, app_vminor, library_vmajor(), library_vminor(), library_keylength(), library_isdemo() ? "crippleware" : "");
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
		byteptr outbuf = 0;

		size_t bufsize = 1024;
		byteptr inbuf = malloc( bufsize );
		if( !inbuf ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }

		size_t pos = 0;
		while( 1 )
		{
			char ch = getc( stdin );
			if( ch == EOF ) { inbuf[pos] = 0; break; }
			inbuf[pos++] = ch;
			if( pos == bufsize-1 )
			{
				bufsize *= 2;
				rc = (inbuf = realloc( inbuf, bufsize )) ? 0 : -9;
				if( rc ) { printf("# decrypt error\n"); goto FAILDECRYPT; }
			}
		}

		rc = library_uri_decoder( &outbuf, location, inbuf );
		if( rc ) { printf("# decrypt error\n"); goto FAILDECRYPT; }

		printf( "%s\n", outbuf );

FAILDECRYPT:

		if( outbuf ) library_free( &outbuf );
		if( inbuf ) free( inbuf );
	}
	else
	{
		byteptr outbuf = 0;

		rc = library_uri_encoder( &outbuf, shares, threshold, location, clueData, messageArg );
		if( rc ) { printf("# encrypt error\n"); goto FAILCRYPT; }

		for( byteptr p=outbuf; *p; p++ ) { if( *p == '|' ) { *p = '\n'; } }

		printf( "%s\n", outbuf );

FAILCRYPT:

		if( outbuf ) library_free( &outbuf );
	}

FAILPLATFORM:

	library_cleanup();

	if( pause ) getchar();

	return rc ? 1 : 0 ;
}
