// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "version.h"

#include "library.h"
#include "ec_field.h"
#include "ar_uricodec.h"
#include "ar_util.h"
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

#else
	if ( *(word32*)x != 0x01020304 )
	{
#if defined(_DEBUG)
		printf( "# libartemis: expected LITTLE_ENDIAN, found BIG_ENDIAN\n" );
#endif
		ASSERT(0);
		return 1;
	}
	
#endif

	return 0;
}

//////////////////

word32 library_init()
{
#if defined(_DEBUG)
	printf( "library_init\n" );
#endif

	if( testendianness() ) return 1;

	srand( 1 );
	gfInit();

	return 0;
}

void library_free( byteptr* ppObject )
{
	if( *ppObject) { free( *ppObject); *ppObject = 0; }
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
	return AR_CRYPTKEYLENGTH;
}

//////////////////

int library_uri_encoder( byteptr* sharesBarArr_out, int shares, int threshold, byteptr location, byteptr clueBarArr, byteptr message )
{
	int rc = 0;

	*sharesBarArr_out = 0;
	arAuth* arecord = 0;
	arShareptr* srecordArr = 0;
	byteptr* clueArr = 0;

	if( threshold == 0 || shares == 0 || threshold > shares ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t clueDataLen = clueBarArr ? strlen( clueBarArr ) : 0;
	if( clueDataLen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	int clueDelimCount = 0;
	int clueIndex = 0;
	clueArr = malloc( sizeof(byteptr) * ( shares + 1 ) );
	clueArr[clueIndex++] = clueBarArr;							// index 0 is topiclue
	for( size_t i=0; i<clueDataLen; i++ )
	{
		if( clueBarArr[i] == '|' )
		{
			clueArr[clueIndex++] = &clueBarArr[i] + 1;
			clueBarArr[i] = 0;
			clueDelimCount++;
		}
	}
	if( clueDelimCount != shares ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t loclen = location ? strlen( location ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	// alloc arecord
	size_t acluelen = ( clueArr && clueArr[0] ) ? strlen( clueArr[0] ) : 0;
	size_t abuflen = ( loclen + acluelen + messlen + 1 /* +1 for \0 */ );
	size_t astructlen = sizeof(arAuth) + abuflen;
	if( !(arecord = malloc( astructlen ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
	memset( arecord, 0, astructlen );
	arecord->bufmax = (word16)abuflen;

	// alloc srecord arr and the srecords themselves
	if( !(srecordArr = malloc( sizeof(arShareptr) * shares ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
	memset( srecordArr, 0, sizeof(arShareptr) * shares );
	for( word16 i=0; i<shares; i++ )
	{
		size_t scluelen = ( clueArr && clueArr[i] ) ? ( strlen( clueArr[i] ) + 1 /* +1 for \0 */ ) : 0;
		size_t sbuflen = loclen + scluelen;
		size_t sstructlen = sizeof(arShare) + sbuflen;
		if( !(srecordArr[i] = malloc( sstructlen ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( srecordArr[i], 0, sstructlen );
		srecordArr[i]->bufmax = (word16)sbuflen;
	}

	// +1 to include \0
	rc = ar_core_create( arecord, srecordArr, shares, threshold, message, (word16)messlen+1, clueArr, location );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	//////////////

	size_t outbufsize = 0;
	ar_uri_bufsize_a( &outbufsize, arecord );
	outbufsize *= shares;

	if( (*sharesBarArr_out = malloc( outbufsize )) == 0 ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
	memset( *sharesBarArr_out, 0, outbufsize );

	(*sharesBarArr_out)[0] = 0;
	rc = ar_uri_create_a( (*sharesBarArr_out), outbufsize, arecord );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	for( word16 s=0; s!=shares; s++ )
	{
		rc = ar_util_strcat( (*sharesBarArr_out), outbufsize, "|" ); // delimiter
		rc = ar_uri_create_s( (*sharesBarArr_out), outbufsize, srecordArr[s] );
		if( rc ) { ASSERT(0); goto FAILCRYPT; }
	}

FAILCRYPT:

	if( arecord ) free( arecord );
	for( word16 i=0; i<shares; i++ ) { if( srecordArr[i] ) free( srecordArr[i] ); }
	if( srecordArr ) free( srecordArr );
	if( clueArr ) free( clueArr );

	return rc;
}

int library_uri_decoder( byteptr* message_out, byteptr location, byteptr sharesNLArr )
{
	int rc = 0;

	*message_out = 0;

	size_t messlen = 0;
	size_t cluelen = 0;
	word16 shares = 0;
	word16 sharenum = 0;
	arShareptr* srecordArr = 0;
	arAuth* arecord = 0;
	byteptr message = 0;
	byteptr sharesNLArr_copy = 0;

	size_t loclen = location ? strlen( location ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAILDECRYPT; }

	size_t shareslen = sharesNLArr ? strlen( sharesNLArr ) : 0;
	if( shareslen == 0 ) { ASSERT(0); rc=-1; goto FAILDECRYPT; }

	sharesNLArr_copy = (byteptr)strdup( sharesNLArr );
	if( sharesNLArr_copy == 0 ) { ASSERT(0); rc=-1; goto FAILDECRYPT; }

	byteptr end = sharesNLArr_copy + shareslen;
	byteptr s_record = sharesNLArr_copy;
	byteptr e_record = sharesNLArr_copy;

	while( s_record < end )
	{
		while( *e_record != '\n' && *e_record != '\0' ) { e_record++; }
		*e_record = 0;
		byteptr inbuf = s_record;

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

				size_t buflen = cluelen + messlen + loclen + 1; // +1 for \0
				size_t structlen = sizeof(arAuth) + buflen;

				arecord = malloc( structlen );
				if( !arecord ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
				memset( arecord, 0, structlen );

				arecord->bufmax = (word16)buflen;
			}

			if( ar_uri_parse_a( arecord, inbuf, location ) ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
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

			size_t buflen = cluelen + loclen + 1; // +1 for \0
			size_t structlen = sizeof(arShare) + buflen;

			arShare* pShare = malloc( structlen );
			if( !pShare ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
			memset( pShare, 0, structlen );

			pShare->bufmax = (word16)buflen;

			srecordArr[sharenum++] = pShare;

			if( ar_uri_parse_s( pShare, inbuf, location ) ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }
		}

		s_record = e_record = e_record + 1; // +1 for char after \0
	}

	if( !arecord || !srecordArr ) { ASSERT(0); rc=-9; goto FAILDECRYPT; }

	rc = ar_core_decrypt( message, (word16)messlen, arecord, srecordArr, sharenum );

FAILDECRYPT:

	if( arecord ) free( arecord );
	for( int i=0; i<sharenum; i++ ) { if( srecordArr[i] ) { free( srecordArr[i] ); srecordArr[i] = 0; } }
	if( srecordArr ) free( srecordArr );
	if( message ) *message_out = message;
	if( sharesNLArr_copy ) free( sharesNLArr_copy);

	return rc;
}

void library_test()
{
}
