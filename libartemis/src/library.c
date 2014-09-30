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

static int testflag = 0;

/////////////////////////

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

/////////////////////////

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

int library_istest()
{
#if defined(_DEBUG)
	return testflag;
#else // _DEBUG
	return 0;
#endif // _DEBUG
}

void library_settest()
{
#if defined(_DEBUG)
	testflag = 1;
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

int library_uri_encoder( byteptr* sharesArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, size_t cluePtrArrLen, byteptr message )
{
	int rc = 0;

	*sharesArr_out = 0;
	
	arAuth* arecord = 0;
	arShareptr* srecordArr = 0;
	byteptr* cluePtrArr = 0;
	byteptr clueArr_rw = 0;

	if( threshold == 0 || shares == 0 || threshold > shares ) { ASSERT(0); rc=-1; goto FAILCRYPT; }
	
	if( !sharesArr_out || !szLocation || !clueArr || !message ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	// change delimiters of clueArr
	clueArr_rw = strdup( clueArr ); // make writable version
	if( !clueArr_rw ) { ASSERT(0); rc=-1; goto FAILCRYPT; }
	for( size_t i = 0; i < cluePtrArrLen; i++ ) { if( clueArr_rw[i]=='\n' ) { clueArr_rw[i]='\0'; } }
	if( rc = ar_util_buildBytePtrArr( &cluePtrArr, clueArr_rw, cluePtrArrLen ) ) { ASSERT(0); goto FAILCRYPT; }

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	// alloc arecord
	size_t acluelen = ( cluePtrArr && cluePtrArr[0] ) ? strlen( cluePtrArr[0] ) : 0;
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
		size_t scluelen = ( cluePtrArr && cluePtrArr[i] ) ? ( strlen( cluePtrArr[i] ) + 1 /* +1 for \0 */ ) : 0;
		size_t sbuflen = loclen + scluelen;
		size_t sstructlen = sizeof(arShare) + sbuflen;
		if( !(srecordArr[i] = malloc( sstructlen ) ) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
		memset( srecordArr[i], 0, sstructlen );
		srecordArr[i]->bufmax = (word16)sbuflen;
	}

	// +1 to include \0
	rc = ar_core_create( arecord, srecordArr, shares, threshold, message, (word16)messlen+1, cluePtrArr, szLocation );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	//////////////

	size_t outbufsize = 0;
	ar_uri_bufsize_a( &outbufsize, arecord );
	outbufsize *= shares;

	if( (*sharesArr_out = malloc( outbufsize )) == 0 ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
	memset( *sharesArr_out, 0, outbufsize );

	(*sharesArr_out)[0] = 0;
	rc = ar_uri_create_a( (*sharesArr_out), outbufsize, arecord );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	for( word16 s=0; s!=shares; s++ )
	{
		if( rc = ar_util_strcat( (*sharesArr_out), outbufsize, "\n" ) ) { ASSERT(0); goto FAILCRYPT; }
		if( rc = ar_uri_create_s( (*sharesArr_out), outbufsize, srecordArr[s] ) ) { ASSERT(0); goto FAILCRYPT; }
	}

FAILCRYPT:

	if( arecord ) free( arecord );
	for( word16 i=0; i<shares; i++ ) { if( srecordArr[i] ) free( srecordArr[i] ); }
	if( srecordArr ) free( srecordArr );
	if( clueArr_rw ) free( clueArr_rw );
	if( cluePtrArr ) free( cluePtrArr );

	if( rc && *sharesArr_out ) free( *sharesArr_out );
	
	return rc;
}

int library_uri_decoder( byteptr* message_out, byteptr szLocation, byteptr shares_arr, size_t shareArrLen )
{
	int rc = 0;

	*message_out = 0;

	size_t messlen = 0;
	size_t cluelen = 0;
	word16 threshold = 0;
	word16 shares = 0;
	word16 sharenum = 0;
	arShareptr* srecordArr = 0;
	arAuth* arecord = 0;

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAIL; }

	byteptr end = shares_arr + shareArrLen;
	byteptr s_record = shares_arr;
	byteptr e_record = shares_arr;

	while( s_record < end )
	{
		while( *e_record != '\n' && *e_record != '\0' ) { e_record++; }
		*e_record = 0;
		byteptr inbuf = s_record;

		int type = ar_uri_parse_type( inbuf );
		if( type == -1 ) { ASSERT(0); rc=-9; goto FAIL; }

		if( type == 1 )
		{
			if( !arecord )
			{
				ar_uri_parse_cluelen( &cluelen, inbuf ); // optional

				ar_uri_parse_messlen( &messlen, inbuf );
				if( !messlen ) { return 1; }

				*message_out = malloc( messlen );
				if( !*message_out ) { ASSERT(0); rc=-9; goto FAIL; }
				memset( *message_out, 0, messlen );

				size_t buflen = cluelen + messlen + loclen;
				size_t structlen = sizeof(arAuth) + buflen;

				arecord = malloc( structlen );
				if( !arecord ) { ASSERT(0); rc=-9; goto FAIL; }
				memset( arecord, 0, structlen );

				arecord->bufmax = (word16)buflen;
			}

			if( ar_uri_parse_a( arecord, inbuf, szLocation ) ) { ASSERT(0); rc=-9; goto FAIL; }
		}
		else
		{
			if( !srecordArr )
			{
				if( rc = ar_uri_parse_shareinfo( &shares, &threshold, inbuf ) ) { ASSERT(0); goto FAIL;}

				size_t arrlen = sizeof(arShareptr) * shares;

				srecordArr = malloc( arrlen );
				if( !srecordArr ) { ASSERT(0); rc=-9; goto FAIL; }
				memset( srecordArr, 0, arrlen );
			}

			ar_uri_parse_cluelen( &cluelen, inbuf ); // optional

			size_t buflen = cluelen + loclen;
			size_t structlen = sizeof(arShare) + buflen;

			arShare* pShare = malloc( structlen );
			if( !pShare ) { ASSERT(0); rc=-9; goto FAIL; }
			memset( pShare, 0, structlen );

			pShare->bufmax = (word16)buflen;

			srecordArr[sharenum++] = pShare;

			if( ar_uri_parse_s( pShare, inbuf, szLocation ) ) { ASSERT(0); rc=-9; goto FAIL; }
		}

		s_record = e_record = e_record + 1; // +1 for char after \0
	}

	if( !arecord || !srecordArr ) { rc=-9; goto FAIL; } // no assert here

	rc = ar_core_decrypt( *message_out, (word16)messlen, arecord, srecordArr, sharenum );

FAIL:

	if( rc && *message_out ) { free( *message_out ); *message_out = 0; }
	
	if( arecord ) free( arecord );
	for( int i=0; i<sharenum; i++ ) { if( srecordArr[i] ) { free( srecordArr[i] ); srecordArr[i] = 0; } }
	if( srecordArr ) free( srecordArr );

	return rc;
}

int library_uri_clue( byteptr* clue_out, byteptr szShare )
{
	int rc = 0;
	
	if( !clue_out ) { ASSERT(0); return -1; }
	if( !szShare ) { ASSERT(0); return -1; }

	*clue_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_clue( &pFirst, &pLast, szShare ) ) { ASSERT(0); goto FAIL; }

	size_t clen = pLast - pFirst + 1; // but, overestimates because of b64 encoding
	if( clen > 0 )
	{
		*clue_out = malloc( clen + 1 ); // +1 for \0
		if( *clue_out == 0 ) { ASSERT(0); rc=-1; goto FAIL; }
		
		size_t deltalen = 0;
		if( rc = ar_util_6BAto8BA( &deltalen, *clue_out, clen, pFirst, clen ) ) { ASSERT(0); goto FAIL; }
		if( rc == 0 ) { (*clue_out)[ deltalen ] = 0; }
	}
	
FAIL:

	if( rc && *clue_out ) { free( *clue_out ); *clue_out = 0; }
	
	return rc;
}

int library_uri_location( byteptr* location_out, byteptr szShare )
{
	int rc = 0;
	
	if( !location_out ) { ASSERT(0); return -1; }
	if( !szShare ) { ASSERT(0); return -1; }

	*location_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_location( &pFirst, &pLast, szShare ) ) { ASSERT(0); goto FAIL; }

	if( pFirst != pLast )
	{
		*location_out = (unsigned char*)strndup( pFirst, pLast - pFirst );
		if( !*location_out ) { ASSERT(0); rc=-9; goto FAIL; }
	}
	
FAIL:
	
	if( rc && *location_out ) { free( *location_out ); *location_out = 0; }
	
	return rc;
}

int library_uri_topic( byteptr* topic_out, byteptr szShare )
{
	int rc = 0;
	
	if( !topic_out ) { ASSERT(0); return -1; }
	if( !szShare ) { ASSERT(0); return -1; }

	*topic_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_topic( &pFirst, &pLast, szShare ) ) { ASSERT(0); goto FAIL; }

	if( pFirst != pLast )
	{
		*topic_out = (unsigned char*)strndup( pFirst, pLast - pFirst );
		if( !*topic_out ) { ASSERT(0); rc=-9; goto FAIL; }
	}
	
FAIL:
	
	if( rc && *topic_out ) { free( *topic_out ); *topic_out = 0; }
	
	return rc;
}

int library_uri_shareinfo( word16* pShares, word16* pThreshold, byteptr szShare )
{
	int rc = 0;
	
	if( !pShares ) { ASSERT(0); return -1; }
	if( !pThreshold ) { ASSERT(0); return -1; }
	if( !szShare ) { ASSERT(0); return -1; }

	*pShares = *pThreshold = 0;

	if( rc = ar_uri_parse_shareinfo( pShares, pThreshold, szShare ) ) { ASSERT(0); goto FAIL; }
	
FAIL:
	
	return rc;
}

int library_uri_sharetype( word16* pType, byteptr szShare )
{
	int rc = 0;
	
	if( !pType ) { ASSERT(0); return -1; }

	*pType = 0;
	
	if( (*pType = ar_uri_parse_type( szShare )) == -1 ) { ASSERT(0); rc=-1; goto FAIL; }
	
FAIL:
	
	return rc;
}

void library_test()
{

#if defined(_DEBUG)

	printf("# library_test\n");

	int rc = 0;

	byteptr sharesArr_out;
	int shares = 2;
	int threshold = 2;
	byteptr szLocation = "foo.bar";
	byteptr clueArr = "main clue\nfirst clue\nsecond clue";
	byteptr message = "something shared between friends";

	rc = library_uri_encoder( &sharesArr_out, shares, threshold, szLocation, clueArr, strlen(clueArr), message );
	ASSERT( rc == 0 );

	byteptr message_out;
	rc = library_uri_decoder( &message_out, szLocation, sharesArr_out, strlen(sharesArr_out) );
	ASSERT( rc == 0 );

	ASSERT( strcmp( message, message_out ) == 0 );

	free( sharesArr_out );
	free( message_out );

#endif // _DEBUG

}
