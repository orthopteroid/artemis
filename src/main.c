
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ar_uricodec.h"
#include "ar_core.h"
#include "test.h"

int ar_main(int argc, char **argv);

#if defined(_WIN32) || defined(_WIN64)

int main(int argc, _TCHAR* argv[])
{
	return ar_main( argc, argv );
}

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
	char* message = 0;
	char* clueData = 0;
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
				printf("(artemis v%u.%u, libartemis v%u.%u KL%u %s)\n", app_vmajor, app_vminor, platform_vmajor(), platform_vminor(), platform_keylength(), platform_isdemo() ? "crippleware" : "");
				printf("usage: -h | -z | -d <newline delimited data> | -s <sharesize> -t <thresholdsize> -c \"<topiclue>|<shareclue1>|...|<shareclueN>\" -m \"<textmessage>\"\n");
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
		size_t messlen = 0;
		size_t cluelen = 0;
		word16 shares = 0;
		word16 sharenum = 0;
		arShareptr* srecordArr = 0;
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

			int type = ar_uri_parse_type( inbuf );
			if( type == -1 ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }

			if( type == 1 )
			{
				if( !arecord )
				{
					ar_uri_parse_cluelen( &cluelen, inbuf ); // optional

					ar_uri_parse_messlen( &messlen, inbuf );
					if( !messlen ) { return 1; }

					message = malloc( messlen );
					if( !message ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
					memset( message, 0, messlen );

					size_t buflen = cluelen + messlen;
					size_t structlen = sizeof(arAuth) + buflen;

					arecord = malloc( structlen );
					if( !arecord ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
					memset( arecord, 0, structlen );

					arecord->bufmax = (word16)buflen;
				}

				if( !ar_uri_parse_a( arecord, inbuf ) ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
			}
			else
			{
				if( !srecordArr )
				{
					ar_uri_parse_sharecount( &shares, inbuf );
					if( !shares ) { return 1; }

					size_t arrlen = sizeof(arShareptr) * shares;

					srecordArr = malloc( arrlen );
					if( !srecordArr ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
					memset( srecordArr, 0, arrlen );
				}

				ar_uri_parse_cluelen( &cluelen, inbuf ); // optional

				size_t structlen = sizeof(arShare) + cluelen;

				arShare* pShare = malloc( structlen );
				if( !pShare ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
				memset( pShare, 0, structlen );

				srecordArr[sharenum++]= pShare;

				if( !ar_uri_parse_s( pShare, inbuf ) ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
			}
		}

		if( !arecord || !srecordArr ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }

		rc = ar_core_decrypt( message, (word16)messlen, arecord, srecordArr, sharenum );

FAILDECRYPT:

		printf( "%s\n", !rc ? (char*)message : "# decrypt error" );

		if( arecord ) free( arecord );
		for( int i=0; i<sharenum; i++ ) { if( srecordArr[i] ) { free( srecordArr[i] ); srecordArr[i] = 0; } }
		if( srecordArr ) free( srecordArr );
		if( message ) free( message );
		if( inbuf ) free( inbuf );
		if( clueData ) free( clueData );
	}
	else
	{
		byteptr outbuf = 0;
		arAuth* arecord = 0;
		arShareptr* srecordArr = 0;
		byteptr* clueArr = 0;
		
		if( threshold == 0 || shares == 0 || message == 0 ) { return 1; }

		if( clueData )
		{
			size_t clueDataLen = strlen( clueData );
			int clueIndex = 0;
			clueArr = malloc( sizeof(byteptr) * shares + 1 );
			clueArr[clueIndex++] = clueData;							// index 0 is topiclue
			for( size_t i=0; i<clueDataLen; i++ )
			{
				if( clueData[i] == '|' )
				{
					clueArr[clueIndex++] = &clueData[i] + 1;
					clueData[i] = 0;
				}
			}
		}

		size_t messlen = strlen( message );

		// alloc arecord
		size_t acluelen = clueArr ? strlen( clueArr[0] ) : 0;
		size_t abuflen = ( acluelen + messlen + 1 /* +1 for \0 */ );
		size_t astructlen = sizeof(arAuth) + abuflen;
		if( !(arecord = malloc( astructlen ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( arecord, 0, astructlen );
		arecord->bufmax = (word16)abuflen;

		// alloc srecord arr and the srecords themselves
		if( !(srecordArr = malloc( sizeof(arShareptr) * shares ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( srecordArr, 0, sizeof(arShareptr) * shares );
		for( word16 i=0; i<shares; i++ )
		{
			size_t sbuflen = clueArr ? ( strlen( clueArr[i] ) + 1 /* +1 for \0 */ ) : 0;
			size_t sstructlen = sizeof(arShare) + abuflen;
			if( !(srecordArr[i] = malloc( sstructlen ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
			memset( srecordArr[i], 0, sstructlen );
			srecordArr[i]->bufmax = (word16)sbuflen;
		}

		// +1 to include \0
		rc = ar_core_create( arecord, srecordArr, shares, threshold, message, (word16)messlen+1, clueArr );
		if( rc ) { printf("# encrypt error\n"); goto FAILCRYPT; }

		size_t uribufsize = 0;
		ar_uri_bufsize_a( &uribufsize, arecord );

		if( (outbuf = malloc( uribufsize )) == 0 ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( outbuf, 0, uribufsize );

		outbuf[0] = 0;
		rc = ar_uri_create_a( outbuf, uribufsize, arecord );
		if( rc ) { printf("# uri creation error\n"); goto FAILCRYPT; }

		printf("%s\n", outbuf);

		for( word16 s=0; s!=shares; s++ )
		{
			outbuf[0] = 0;
			rc = ar_uri_create_s( outbuf, uribufsize, srecordArr[s] );
			if( rc ) { printf("# uri creation error\n"); goto FAILCRYPT; }

			printf("%s\n", outbuf);
		}

FAILCRYPT:

		if( arecord ) free( arecord );
		for( word16 i=0; i<shares; i++ ) { if( srecordArr[i] ) free( srecordArr[i] ); }
		if( srecordArr ) free( srecordArr );
		if( clueArr ) free( clueArr );
		if( outbuf ) free( outbuf );
	}

	platform_cleanup();

	if( pause ) getchar();

	return rc ? 1 : 0 ;
}
