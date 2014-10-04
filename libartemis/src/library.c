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

typedef struct {
	byteptr start, end, s_record, e_record;
	int type;
} scanstate;
typedef scanstate* ssptr;

static void ss_init( ssptr pss, byteptr buf, size_t len )

{
	pss->start = buf;
	pss->end = buf + len;
	pss->s_record = buf;
	pss->e_record = buf;
	pss->type = 0;
}

static int ss_scan( ssptr pss )
{
	if( pss->e_record != pss->start ) { pss->s_record = pss->e_record = pss->e_record + 1; } // +1 for char after \0
	if( pss->s_record >= pss->end ) { pss->type = 0; return 0; } // stopping condition

	while( *(pss->e_record) != '\0' ) { pss->e_record++; } // scan \0 delimited string
	if( pss->e_record > pss->end ) { LOGFAIL; return -2; } // blew buffer
	*(pss->e_record) = 0; // \0 term the string

	pss->type = ar_uri_parse_type( pss->s_record );

	return 1; // continuing condition
}

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
		LOGFAIL;
		return 1;
	}

#else
	if ( *(word32*)x != 0x01020304 )
	{
#if defined(_DEBUG)
		printf( "# libartemis: expected LITTLE_ENDIAN, found BIG_ENDIAN\n" );
#endif
		LOGFAIL;
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

int library_uri_clue( byteptr* clue_out, byteptr szShare )
{
	int rc = 0;
	
	if( !clue_out ) { LOGFAIL; return -1; }
	if( !szShare ) { LOGFAIL; return -1; }

	*clue_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_clue( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto FAIL; }

	size_t clen = pLast - pFirst + 1; // but, overestimates because of b64 encoding
	if( clen > 0 )
	{
		*clue_out = malloc( clen + 1 ); // +1 for \0
		if( *clue_out == 0 ) { LOGFAIL; rc=-1; goto FAIL; }
		
		size_t deltalen = 0;
		if( rc = ar_util_6BAto8BA( &deltalen, *clue_out, clen, pFirst, clen ) ) { LOGFAIL; goto FAIL; }
		if( rc == 0 ) { (*clue_out)[ deltalen ] = 0; }
	}
	
FAIL:

	if( rc && *clue_out ) { free( *clue_out ); *clue_out = 0; }
	
	return rc;
}

int library_uri_location( byteptr* location_out, byteptr szShare )
{
	int rc = 0;
	
	if( !location_out ) { LOGFAIL; return -1; }
	if( !szShare ) { LOGFAIL; return -1; }

	*location_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_location( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto FAIL; }

	if( pFirst != 0 )
	{
		*location_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !*location_out ) { LOGFAIL; rc=-9; goto FAIL; }
	}
	
FAIL:
	
	if( rc && *location_out ) { free( *location_out ); *location_out = 0; }
	
	return rc;
}

int library_uri_topic( byteptr* topic_out, byteptr szShare )
{
	int rc = 0;
	
	if( !topic_out ) { LOGFAIL; return -1; }
	if( !szShare ) { LOGFAIL; return -1; }

	*topic_out = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( rc = ar_uri_locate_topic( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto FAIL; }

	if( pFirst != 0 )
	{
		*topic_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !*topic_out ) { LOGFAIL; rc=-9; goto FAIL; }
	}
	
FAIL:
	
	if( rc && *topic_out ) { free( *topic_out ); *topic_out = 0; }
	
	return rc;
}

int library_uri_info( word16* pType, word16* pShares, word16* pThreshold, byteptr szShare )
{
	int rc = 0;
	
	if( !pType ) { LOGFAIL; return -1; }
	if( !pShares ) { LOGFAIL; return -1; }
	if( !pThreshold ) { LOGFAIL; return -1; }
	if( !szShare ) { LOGFAIL; return -1; }

	*pType = *pShares = *pThreshold = 0;

	if( rc = ar_uri_parse_info( pType, pShares, pThreshold, szShare ) ) { LOGFAIL; goto FAIL; }
	
FAIL:
	
	return rc;
}

//////////////////

int library_uri_encoder( byteptr* recordArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, byteptr message )
{
	int rc = 0;

	if( !recordArr_out ) { LOGFAIL; return -1; }

	*recordArr_out = 0;
	
	if( !szLocation || !clueArr || !message ) { LOGFAIL; return -1; }

	if( threshold == 0 || shares == 0 || threshold > shares ) { LOGFAIL; return -1; }
	
	arAuthptr arecord = 0;
	arSharetbl srecordtbl = 0;
	bytetbl clueTbl = 0;
	byteptr clueArr_rw = 0;

	// change delimiters of clueArr
	clueArr_rw = strdup( clueArr ); // make writable version
	if( !clueArr_rw ) { LOGFAIL; rc=-1; goto FAILCRYPT; }

	size_t cluePtrArrLen = strlen( clueArr_rw );
	for( size_t i = 0; i < cluePtrArrLen; i++ ) { if( clueArr_rw[i]=='\n' ) { clueArr_rw[i]='\0'; } }
	if( rc = ar_util_buildByteTbl( &clueTbl, clueArr_rw, cluePtrArrLen ) ) { LOGFAIL; goto FAILCRYPT; }

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { LOGFAIL; rc=-1; goto FAILCRYPT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { LOGFAIL; rc=-1; goto FAILCRYPT; }

	// alloc arecord
	size_t acluelen = ( clueTbl && clueTbl[0] ) ? strlen( clueTbl[0] ) : 0;
	size_t abuflen = ( loclen + acluelen + messlen + 1 /* +1 for \0 */ );
	size_t astructlen = sizeof(arAuth) + abuflen;

	// alloc srecord arr and the srecords themselves
	for( word16 i=0; i<shares; i++ )
	{
		size_t scluelen = ( clueTbl && clueTbl[i] ) ? ( strlen( clueTbl[i] ) + 1 /* +1 for \0 */ ) : 0;
		size_t sbuflen = loclen + scluelen;
		size_t sstructlen = sizeof(arShare) + sbuflen;
	}

	// +1 to include \0
	if( rc = ar_core_create( &arecord, &srecordtbl, shares, threshold, message, (word16)messlen + 1, clueTbl, szLocation ) ) { LOGFAIL; goto FAILCRYPT; }

	// calc outbuf size

	size_t bufsize = 0;
	ar_uri_bufsize_a( &bufsize, arecord );
	for( int i = 0; i < shares; i++ ) { size_t s=0; ar_uri_bufsize_s( &s, srecordtbl[i] ); bufsize += s; }

	if( !(*recordArr_out = malloc( bufsize )) ) { LOGFAIL; rc=-9; goto FAILCRYPT; }
	memset( *recordArr_out, 0, bufsize );

	// concat to output buffer

	(*recordArr_out)[0] = 0;
	if( rc = ar_uri_create_a( (*recordArr_out), bufsize, arecord ) ) { LOGFAIL; goto FAILCRYPT; }
	for( word16 s=0; s!=shares; s++ )
	{
		if( rc = ar_util_strcat( (*recordArr_out), bufsize, "\n" ) ) { LOGFAIL; goto FAILCRYPT; }
		if( rc = ar_uri_create_s( (*recordArr_out), bufsize, srecordtbl[s] ) ) { LOGFAIL; goto FAILCRYPT; }
	}

FAILCRYPT:

	if( arecord ) free( arecord );
	if( srecordtbl ) {
		for( word16 i=0; i<shares; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( clueArr_rw ) free( clueArr_rw );
	if( clueTbl ) free( clueTbl );

	return rc;
}

int library_uri_decoder( byteptr* message_out, byteptr location, byteptr recordArr )
{
	int rc = 0;

	if( !message_out ) { LOGFAIL; return -1; }

	*message_out = 0;

	if( !location ) { LOGFAIL; return -1; }
	if( !recordArr ) { LOGFAIL; return -1; }

	size_t shareArrLen = strlen( recordArr );

	if( shareArrLen < 10 ) { LOGFAIL; return -1; }

	arAuth* pARecord = 0;
	arShareptr* srecordtbl = 0;
	scanstate ss;

	byteptr arecordLoc = 0;
	byteptr recordArr_rw = 0;

	// dup and change delim 

	if( !(recordArr_rw = strdup( recordArr )) ) { LOGFAIL; rc=-9; goto FAIL; }

	for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

	// count shares

	word16 arecordCount = 0;
	word16 srecordCount = 0;
	word16 srecordInit = 0;

	ss_init( &ss, recordArr_rw, shareArrLen );
	while( (rc = ss_scan( &ss )) == 1 )
	{
		if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
	}
	if( rc < 0 ) { LOGFAIL; goto FAIL; }

	if( arecordCount != 1 ) { LOGFAIL; rc=-2; goto FAIL; }
	if( srecordCount < 2 ) { LOGFAIL; rc=-2; goto FAIL; }

	// create share-objects

	size_t tblsize = sizeof(arShareptr) * srecordCount;
	if( !(srecordtbl = malloc( tblsize )) ) { LOGFAIL; rc=-9; goto FAIL; }
	memset( srecordtbl, 0, tblsize );

	ss_init( &ss, recordArr_rw, shareArrLen );
	while( (rc = ss_scan( &ss )) == 1 )
	{
		if( ss.type == 1 ) {
			if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL; goto FAIL; }
			if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL; goto FAIL; }
		} else {
			if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL; goto FAIL; }
		}
	}
	if( rc < 0 ) { LOGFAIL; goto FAIL; }
	if( srecordInit != srecordCount ) { LOGFAIL; rc=-2; goto FAIL; }

	if( strcmp( arecordLoc, location ) != 0 ) { LOGFAIL; rc=-2; goto FAIL; }

	// decrypt

	if( rc = ar_core_decrypt( message_out, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; goto FAIL; }

FAIL:

	if( arecordLoc ) free( arecordLoc );
	if( pARecord ) free( pARecord );
	if( srecordtbl )
	{
		for( int i = 0; i < srecordInit; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( recordArr_rw ) free( recordArr_rw );

	return rc;
}

int library_uri_validate( byteptr* invalidBoolArr_out, byteptr szLocation, byteptr szRecordArr )
{
	int rc = 0;

	if( invalidBoolArr_out ) { *invalidBoolArr_out = 0; }
	if( !szLocation ) { LOGFAIL; return -1; }
	if( !szRecordArr ) { LOGFAIL; return -1; }

	size_t shareArrLen = strlen( szRecordArr );

	if( shareArrLen < 10 ) { LOGFAIL; return -1; }

	arAuth* pARecord = 0;
	arShareptr* srecordtbl = 0;
	scanstate ss;

	byteptr arecordLoc = 0;
	byteptr recordArr_rw = 0;

	// dup and change delim 

	if( !(recordArr_rw = strdup( szRecordArr )) ) { LOGFAIL; rc=-9; goto FAIL; }

	for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

	// count shares

	word16 arecordCount = 0;
	word16 srecordCount = 0;
	word16 srecordInit = 0;

	ss_init( &ss, recordArr_rw, shareArrLen );
	while( (rc = ss_scan( &ss )) == 1 )
	{
		if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
	}
	if( rc < 0 ) { LOGFAIL; goto FAIL; }

	if( arecordCount != 1 ) { LOGFAIL; rc=-2; goto FAIL; }
	if( srecordCount < 2 ) { LOGFAIL; rc=-2; goto FAIL; }

	// create share-objects

	size_t tblsize = sizeof(arShareptr) * srecordCount;
	if( !(srecordtbl = malloc( tblsize )) ) { LOGFAIL; rc=-9; goto FAIL; }
	memset( srecordtbl, 0, tblsize );

	ss_init( &ss, recordArr_rw, shareArrLen );
	while( (rc = ss_scan( &ss )) == 1 )
	{
		if( ss.type == 1 ) {
			if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL; goto FAIL; }
			if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL; goto FAIL; }
		} else {
			if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL; goto FAIL; }
		}
	}
	if( rc < 0 ) { LOGFAIL; goto FAIL; }
	if( srecordInit != srecordCount ) { LOGFAIL; rc=-2; goto FAIL; }
	
	// validate

	if( strcmp( arecordLoc, szLocation ) != 0 ) { LOGFAIL; rc=-2; goto FAIL; }

	if( rc = ar_core_check_topic( 0, pARecord, 0, 0 ) ) { LOGFAIL; rc = (rc==-2) ? -2 : rc; goto FAIL; } // conv failure code

	if( rc = ar_core_check_signature( 0, pARecord, 0, 0 ) ) { LOGFAIL; rc = (rc==-2) ? -3 : rc; goto FAIL; } // conv failure code

	if( !(*invalidBoolArr_out = malloc( srecordCount )) ) { LOGFAIL; rc=-9; goto FAIL; }

	if( rc = ar_core_check_topic( (*invalidBoolArr_out), pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; rc = (rc==-2) ? -4 : rc; goto FAIL; } // conv failure code

	if( rc = ar_core_check_signature( (*invalidBoolArr_out), pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; rc = (rc==-2) ? -5 : rc; goto FAIL; } // conv failure code

FAIL:

	if( arecordLoc ) free( arecordLoc );
	if( pARecord ) free( pARecord );
	if( srecordtbl )
	{
		for( int i = 0; i < srecordInit; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( recordArr_rw ) free( recordArr_rw );

	return rc;
}

void library_test()
{

#if defined(_DEBUG)

	printf("# library_test\n");

	int rc = 0;

	byteptr recordArr;
	byteptr message;
	byteptr validation;

	int shares = 2;
	int threshold = 2;
	byteptr location = "foo.bar";
	byteptr clueArr = "main clue\nfirst clue\nsecond clue";
	byteptr message_in = "something shared between friends";

	rc = library_uri_encoder( &recordArr, shares, threshold, location, clueArr, message_in );
	TESTASSERT( rc == 0 );

	rc = library_uri_decoder( &message, location, recordArr );
	TESTASSERT( rc == 0 );

	TESTASSERT( strcmp( message_in, message ) == 0 );

	rc = library_uri_validate( &validation, location, recordArr );
	TESTASSERT( rc == 0 );
	free( validation );

	// now break something
	recordArr[ 3 + strlen(recordArr) / 2 ]++;
	rc = library_uri_validate( &validation, location, recordArr );
	TESTASSERT( rc == -3 );
	free( validation );

	free( recordArr );
	free( message );

#endif // _DEBUG

}
