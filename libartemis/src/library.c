// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "platform.h"
#include "version.h"
#include "ar_codes.h"

#include "library.h"
#include "ec_field.h"
#include "ar_uricodec.h"
#include "ar_util.h"
#include "ar_core.h"

#if defined(__ANDROID__)
	#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "TAG", __VA_ARGS__);
#endif

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
	if( pss->e_record > pss->end ) { int rc = RC_BUFOVERFLOW; LOGFAIL( rc ); return rc; } // blew buffer
	*(pss->e_record) = 0; // \0 term the string

	pss->type = 0;
	if( strstr( pss->s_record, "ai=" ) )			{ pss->type = 1; }
	else if( strstr( pss->s_record, "si=" ) )		{ pss->type = 2; }
	if( pss->type == 0 ) { int rc = RC_ARG; LOGFAIL( rc ); return rc; } // bad record

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
		LOGFAIL( RC_INTERNAL );
		return 1;
	}

#else
	if ( *(word32*)x != 0x01020304 )
	{
#if defined(_DEBUG)
		printf( "# libartemis: expected LITTLE_ENDIAN, found BIG_ENDIAN\n" );
#endif
		LOGFAIL( RC_INTERNAL );
		return 1;
	}
	
#endif

	return 0;
}

/////////////////////////

const char* library_rclookup( int rc )
{
	return ar_util_rclookup( rc );
}

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
	return ar_util_istest();
}

void library_settest()
{
	ar_util_settest();
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
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( !clue_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }
	
	*clue_out = 0;
	
	if( !szShare ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( rc = ar_uri_locate_clue( &pFirst, &pLast, szShare ) ) { LOGFAIL( rc ); goto EXIT; }

	size_t clen = pLast - pFirst + 1; // but, overestimates because of b64 encoding
	if( clen > 0 )
	{
		if( !(*clue_out = malloc( clen + 1 )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; } // +1 for \0
		
		size_t deltalen = 0;
		if( rc = ar_util_6BAto8BA( &deltalen, *clue_out, clen, pFirst, clen ) ) { LOGFAIL( rc ); goto EXIT; }

		(*clue_out)[ deltalen ] = 0;
	}
	
EXIT:

	if( rc && clue_out && *clue_out ) { free( *clue_out ); *clue_out = 0; }
	
	return rc;
}

int library_uri_location( byteptr* location_out, byteptr szShare )
{
	int rc = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( !location_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*location_out = 0;
	
	if( !szShare ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	
	if( rc = ar_uri_locate_location( &pFirst, &pLast, szShare ) ) { LOGFAIL( rc ); goto EXIT; }

	if( pFirst != 0 )
	{
		*location_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !(*location_out) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	}
	
EXIT:
	
	if( rc && location_out && *location_out ) { free( *location_out ); *location_out = 0; }
	
	return rc;
}

int library_uri_topic( byteptr* topic_out, byteptr szShare )
{
	int rc = 0;
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( !topic_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*topic_out = 0;
	
	if( !szShare ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	
	if( rc = ar_uri_locate_topic( &pFirst, &pLast, szShare ) ) { LOGFAIL( rc ); goto EXIT; }

	if( pFirst != 0 )
	{
		*topic_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !(*topic_out) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	}
	
EXIT:
	
	if( rc && topic_out && *topic_out ) { free( *topic_out ); *topic_out = 0; }
	
	return rc;
}

int library_uri_info( word16* pType, word16* pShares, word16* pThreshold, byteptr szShare )
{
	int rc = 0;
	
	if( !pType ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !pShares ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !pThreshold ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !szShare ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	*pType = *pShares = *pThreshold = 0;

	if( rc = ar_uri_parse_info( pType, pShares, pThreshold, szShare ) ) { LOGFAIL( rc ); goto EXIT; }
	
EXIT:
	
	return rc;
}

//////////////////

int library_uri_encoder( byteptr* recordArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, byteptr message )
{
	int rc = 0;

	arAuthptr arecord = 0;
	arSharetbl srecordtbl = 0;
	bytetbl clueTbl = 0;
	byteptr clueArr_rw = 0;

	if( !recordArr_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*recordArr_out = 0;
	
	if( !szLocation ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !clueArr ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !message ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( threshold == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( shares == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( threshold > shares ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	
	// change delimiters of clueArr
	if( !(clueArr_rw = strdup( clueArr )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	size_t cluePtrArrLen = strlen( clueArr_rw );
	for( size_t i = 0; i < cluePtrArrLen; i++ ) { if( clueArr_rw[i]=='\n' ) { clueArr_rw[i]='\0'; } }
	if( rc = ar_util_buildByteTbl( &clueTbl, clueArr_rw, cluePtrArrLen ) ) { LOGFAIL( rc ); goto EXIT; }

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

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
	if( rc = ar_core_create( &arecord, &srecordtbl, shares, threshold, message, (word16)messlen + 1, clueTbl, szLocation ) ) { LOGFAIL( rc ); goto EXIT; }

	// calc outbuf size

	size_t bufsize = 0;
	ar_uri_bufsize_a( &bufsize, arecord );
	for( int i = 0; i < shares; i++ ) { size_t s=0; ar_uri_bufsize_s( &s, srecordtbl[i] ); bufsize += s; }

	if( !(*recordArr_out = malloc( bufsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( *recordArr_out, 0, bufsize );

	// concat to output buffer

	(*recordArr_out)[0] = 0;
	if( rc = ar_uri_create_a( (*recordArr_out), bufsize, arecord ) ) { LOGFAIL( rc ); goto EXIT; }
	for( word16 s=0; s!=shares; s++ )
	{
		if( rc = ar_util_strcat( (*recordArr_out), bufsize, "\n" ) ) { LOGFAIL( rc ); goto EXIT; }
		if( rc = ar_uri_create_s( (*recordArr_out), bufsize, srecordtbl[s] ) ) { LOGFAIL( rc ); goto EXIT; }
	}

EXIT:

	if( arecord ) free( arecord );
	if( srecordtbl ) {
		for( word16 i=0; i<shares; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( clueArr_rw ) free( clueArr_rw );
	if( clueTbl ) free( clueTbl );

	if( rc && recordArr_out && *recordArr_out ) { free( *recordArr_out); *recordArr_out = 0; }

	return rc;
}

int library_uri_decoder( byteptr* message_out, byteptr location, byteptr recordArr )
{
	int rc = 0;
	scanstate ss;

	arAuth* pARecord = 0;
	arShareptr* srecordtbl = 0;
	byteptr arecordLoc = 0;
	byteptr recordArr_rw = 0;

	word16 arecordCount = 0;
	word16 srecordCount = 0;
	word16 srecordInit = 0;

	if( !message_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*message_out = 0;

	if( !location ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !recordArr ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	{
		size_t shareArrLen = strlen( recordArr );

		if( shareArrLen < 10 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

		// dup and change delim 

		if( !(recordArr_rw = strdup( recordArr )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

		for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

		// count shares

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
		}
		if( rc < 0 ) { LOGFAIL( rc ); goto EXIT; }

		if( arecordCount != 1 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
		if( srecordCount < 2 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

		// create share-objects

		{
			size_t tblsize = sizeof(arShareptr) * srecordCount;
			if( !(srecordtbl = malloc( tblsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
			memset( srecordtbl, 0, tblsize );
		}

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) {
				if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
				if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
			} else {
				if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
			}
		}
		if( rc < 0 ) { LOGFAIL( rc ); goto EXIT; }
		if( srecordInit != srecordCount ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	}
	
	if( strcmp( arecordLoc, location ) != 0 ) { rc = RC_LOCATION; LOGFAIL( rc ); goto EXIT; }

	// decrypt

	if( rc = ar_core_decrypt( message_out, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL( rc ); goto EXIT; }

EXIT:

	if( arecordLoc ) free( arecordLoc );
	if( pARecord ) free( pARecord );
	if( srecordtbl )
	{
		for( int i = 0; i < srecordInit; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( recordArr_rw ) free( recordArr_rw );

	if( rc && message_out && *message_out ) { free( *message_out ); *message_out = 0; }

	return rc;
}

int library_uri_validate( byteptr* invalidBoolArr_out_opt, byteptr szLocation, byteptr szRecordArr )
{
	int rc = 0;
	scanstate ss;

	arAuth* pARecord = 0;
	arShareptr* srecordtbl = 0;
	byteptr arecordLoc = 0;
	byteptr recordArr_rw = 0;

	word16 arecordCount = 0;
	word16 srecordCount = 0;
	word16 srecordInit = 0;

	if( invalidBoolArr_out_opt ) { *invalidBoolArr_out_opt = 0; }
	
	if( !szLocation ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !szRecordArr ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	{
		size_t shareArrLen = strlen( szRecordArr );

		if( shareArrLen < 10 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

		// dup and change delim 

		if( !(recordArr_rw = strdup( szRecordArr )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

		for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

		// count shares

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
		}
		if( rc < 0 ) { LOGFAIL( rc ); goto EXIT; }

		if( arecordCount != 1 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
		if( srecordCount < 2 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

		// create share-objects

		{
			size_t tblsize = sizeof(arShareptr) * srecordCount;
			if( !(srecordtbl = malloc( tblsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
			memset( srecordtbl, 0, tblsize );
		}

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) {
				if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
				if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
			} else {
				if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL( rc ); goto EXIT; }
			}
		}
		if( rc < 0 ) { LOGFAIL( rc ); goto EXIT; }
		if( srecordInit != srecordCount ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	}

	// validate

	if( strcmp( arecordLoc, szLocation ) != 0 ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }

	if( rc = ar_core_check_topic( 0, pARecord, 0, 0 ) ) { LOGFAIL( rc ); goto EXIT; } // conv failure code

	if( rc = ar_core_check_signature( 0, pARecord, 0, 0 ) ) { LOGFAIL( rc ); goto EXIT; } // conv failure code

	{
		byteptr pBoolArr = 0;
		
		if( invalidBoolArr_out_opt )
		{
			if( !(pBoolArr = malloc( srecordCount )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
		}
		*invalidBoolArr_out_opt = pBoolArr; // reassign before possible failures below
		
		if( rc = ar_core_check_topic( pBoolArr, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL( rc ); goto EXIT; } // conv failure code
		
		if( rc = ar_core_check_signature( pBoolArr, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL( rc ); goto EXIT; } // conv failure code
	}

EXIT:

	if( arecordLoc ) free( arecordLoc );
	if( pARecord ) free( pARecord );
	if( srecordtbl )
	{
		for( int i = 0; i < srecordInit; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
		free( srecordtbl );
	}
	if( recordArr_rw ) free( recordArr_rw );

	if( rc && invalidBoolArr_out_opt && *invalidBoolArr_out_opt ) { free( *invalidBoolArr_out_opt ); *invalidBoolArr_out_opt = 0; }

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
	
#if defined(ENABLE_TESTFAIL)

	// now break something
	recordArr[ 3 + strlen(recordArr) / 2 ]++;
	rc = library_uri_validate( &validation, location, recordArr );
	TESTASSERT( rc != 0 );
	free( validation );

#endif // ENABLE_TESTFAIL

	free( recordArr );
	free( message );

#endif // _DEBUG

}
