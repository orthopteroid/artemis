
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar_uricodec.h"
#include "ar_core.h"

#include "test.h"

unsigned long app_vmajor = 0;
unsigned long app_vminor = 1;

int ar_main(int argc, char **argv)
{
	int rc = 0;
	char* message = 0;
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
				printf("(winartemis v%u.%u, libartemis v%u.%u KL%u %s)\n", app_vmajor, app_vminor, platform_vmajor(), platform_vminor(), platform_keylength(), platform_isdemo() ? "crippleware" : "");
				printf("usage: -h | -z | -d <newline delimited data> | -s <sharesize> -t <thresholdsize> -m <textmessage>\n");
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
			case 'm':
				if( ++i >= argc ) { return 1; }
				message = argv[i];
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

	platform_init();

	if( testmode )
	{
		test_all();
	}
	else if( decode )
	{
		size_t messsize = 0;
		word16 shares = 0;
		word16 sharenum = 0;
		arShare* srecordArr = 0;
		arAuth* arecord = 0;
		byteptr message = 0;

		char ch;
		size_t pos;
		size_t bufsize = 256;
		byteptr inbuf = malloc( bufsize );
		if( !inbuf ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }

		while( 1 )
		{
			pos = 0;
			while( 1 )
			{
				ch = getc( stdin );
				if( ch == EOF || ch == '\n' ) { inbuf[pos] = 0; break; }
				inbuf[pos++] = ch;
				if( pos == bufsize-1 )
				{
					bufsize *= 2;
					inbuf = realloc( inbuf, bufsize );
					if( !inbuf ) { return 1; }
				}
			}
			if( ch == EOF ) { break; }

			if( !arecord )
			{
				ar_uri_estimatemessagesize( &messsize, inbuf );
				if( !messsize ) { return 1; }

				message = malloc( messsize );
				if( !message ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
				memset( message, 0, messsize );

				ar_uri_estimateshares( &shares, inbuf );
				if( !shares ) { return 1; }

				arecord = malloc( sizeof(arAuth) + messsize );
				if( !arecord ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
				memset( arecord, 0, sizeof(arAuth) + messsize );
				arecord->bufmax = (word16)messsize;

				srecordArr = malloc( sizeof(arShare) * shares );
				if( !srecordArr ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
				memset( srecordArr, 0, sizeof(arShare) * shares );

				if( rc = ar_uri_parse( arecord, &srecordArr[sharenum], inbuf ) != 1 ) { ASSERT(0); goto FAILDECRYPT; }
			}
			else
			{
				if( rc = ar_uri_parse( arecord, &srecordArr[sharenum++], inbuf ) != 2 ) { ASSERT(0); goto FAILDECRYPT; }
			}
		}

		rc = ar_core_decrypt( message, (word16)messsize, arecord, srecordArr, sharenum );

		printf( "%s\n", !rc ? message : "# decrypt error" );

FAILDECRYPT:

		if( arecord ) free( arecord );
		if( srecordArr ) free( srecordArr );
		if( message ) free( message );
		if( inbuf ) free( inbuf );
	}
	else
	{
		byteptr outbuf = 0;
		arAuth* arecord = 0;
		arShare* srecordArr = 0;

		if( threshold == 0 || shares == 0 || message == 0 ) { return 1; }

		if( !(srecordArr = malloc( sizeof(arShare) * shares )) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( srecordArr, 0, sizeof(arShare) * shares );

		size_t buflen = 2 * strlen(message);
		if( !(arecord = malloc( sizeof(arAuth) + buflen )) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( arecord, 0, sizeof(arAuth) + buflen );
		arecord->bufmax = (word16)buflen;

		// +1 to include \0
		rc = ar_core_create( arecord, srecordArr, shares, threshold, message, (word16)(strlen(message) + 1) );
		if( rc ) { printf("# encrypt error\n"); goto FAILCRYPT; }

		size_t uribufsize = 0;
		ar_uri_estimatebufsize( &uribufsize, arecord );

		if( (outbuf = malloc( uribufsize )) == 0 ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( outbuf, 0, uribufsize );

		outbuf[0] = 0;
		rc = ar_uri_create_a( outbuf, uribufsize, arecord );
		if( rc ) { printf("# uri creation error\n"); goto FAILCRYPT; }

		printf("%s\n", outbuf);

		for( word16 s=0; s!=shares; s++ )
		{
			outbuf[0] = 0;
			rc = ar_uri_create_s( outbuf, uribufsize, &srecordArr[s] );
			if( rc ) { printf("# uri creation error\n"); goto FAILCRYPT; }

			printf("%s\n", outbuf);
		}

FAILCRYPT:

		if( arecord ) free( arecord );
		if( srecordArr ) free( srecordArr );
		if( outbuf ) free( outbuf );
	}

	platform_cleanup();

	if( pause ) getchar();

	return rc ? 1 : 0 ;
}
