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
	if( pss->e_record > pss->end ) { LOGFAIL; return RC_BUFOVERFLOW; } // blew buffer
	*(pss->e_record) = 0; // \0 term the string

	pss->type = 0;
	if( strstr( pss->s_record, "ai=" ) )			{ pss->type = 1; }
	else if( strstr( pss->s_record, "si=" ) )		{ pss->type = 2; }
	if( pss->type == 0 ) { LOGFAIL; return RC_ARG; } // bad record

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
	
	byteptr pFirst = 0;
	byteptr pLast = 0;
	
	if( !clue_out ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	
	*clue_out = 0;
	
	if( !szShare ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	if( rc = ar_uri_locate_clue( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto EXIT; }

	size_t clen = pLast - pFirst + 1; // but, overestimates because of b64 encoding
	if( clen > 0 )
	{
		if( !(*clue_out = malloc( clen + 1 )) ) { LOGFAIL; rc = RC_MALLOC; goto EXIT; } // +1 for \0
		
		size_t deltalen = 0;
		if( rc = ar_util_6BAto8BA( &deltalen, *clue_out, clen, pFirst, clen ) ) { LOGFAIL; goto EXIT; }
		if( rc == 0 ) { (*clue_out)[ deltalen ] = 0; }
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
	
	if( !location_out ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	*location_out = 0;
	
	if( !szShare ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	
	if( rc = ar_uri_locate_location( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto EXIT; }

	if( pFirst != 0 )
	{
		*location_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !*location_out ) { LOGFAIL; rc=-9; goto EXIT; }
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
	
	if( !topic_out ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	*topic_out = 0;
	
	if( !szShare ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	
	if( rc = ar_uri_locate_topic( &pFirst, &pLast, szShare ) ) { LOGFAIL; goto EXIT; }

	if( pFirst != 0 )
	{
		*topic_out = (unsigned char*)strndup( pFirst, pLast - pFirst +1 ); // +1 converts to length
		if( !*topic_out ) { LOGFAIL; rc=-9; goto EXIT; }
	}
	
EXIT:
	
	if( rc && topic_out && *topic_out ) { free( *topic_out ); *topic_out = 0; }
	
	return rc;
}

int library_uri_info( word16* pType, word16* pShares, word16* pThreshold, byteptr szShare )
{
	int rc = 0;
	
	if( !pType ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !pShares ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !pThreshold ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !szShare ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	*pType = *pShares = *pThreshold = 0;

	if( rc = ar_uri_parse_info( pType, pShares, pThreshold, szShare ) ) { LOGFAIL; goto EXIT; }
	
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

	if( !recordArr_out ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	*recordArr_out = 0;
	
	if( !szLocation ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !clueArr ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !message ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	if( threshold == 0 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }
	if( shares == 0 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }
	if( threshold > shares ) { LOGFAIL; rc = RC_ARG; goto EXIT; }
	
	// change delimiters of clueArr
	if( !(clueArr_rw = strdup( clueArr )) ) { LOGFAIL; rc=-9; goto EXIT; }

	size_t cluePtrArrLen = strlen( clueArr_rw );
	for( size_t i = 0; i < cluePtrArrLen; i++ ) { if( clueArr_rw[i]=='\n' ) { clueArr_rw[i]='\0'; } }
	if( rc = ar_util_buildByteTbl( &clueTbl, clueArr_rw, cluePtrArrLen ) ) { LOGFAIL; goto EXIT; }

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { LOGFAIL; rc = RC_LOCATION; goto EXIT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { LOGFAIL; rc = RC_MESSAGE; goto EXIT; }

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
	if( rc = ar_core_create( &arecord, &srecordtbl, shares, threshold, message, (word16)messlen + 1, clueTbl, szLocation ) ) { LOGFAIL; goto EXIT; }

	// calc outbuf size

	size_t bufsize = 0;
	ar_uri_bufsize_a( &bufsize, arecord );
	for( int i = 0; i < shares; i++ ) { size_t s=0; ar_uri_bufsize_s( &s, srecordtbl[i] ); bufsize += s; }

	if( !(*recordArr_out = malloc( bufsize )) ) { LOGFAIL; rc=-9; goto EXIT; }
	memset( *recordArr_out, 0, bufsize );

	// concat to output buffer

	(*recordArr_out)[0] = 0;
	if( rc = ar_uri_create_a( (*recordArr_out), bufsize, arecord ) ) { LOGFAIL; goto EXIT; }
	for( word16 s=0; s!=shares; s++ )
	{
		if( rc = ar_util_strcat( (*recordArr_out), bufsize, "\n" ) ) { LOGFAIL; goto EXIT; }
		if( rc = ar_uri_create_s( (*recordArr_out), bufsize, srecordtbl[s] ) ) { LOGFAIL; goto EXIT; }
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

	if( !message_out ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	*message_out = 0;

	if( !location ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !recordArr ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	{
		size_t shareArrLen = strlen( recordArr );

		if( shareArrLen < 10 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }

		// dup and change delim 

		if( !(recordArr_rw = strdup( recordArr )) ) { LOGFAIL; rc=-9; goto EXIT; }

		for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

		// count shares

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
		}
		if( rc < 0 ) { LOGFAIL; goto EXIT; }

		if( arecordCount != 1 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }
		if( srecordCount < 2 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }

		// create share-objects

		{
			size_t tblsize = sizeof(arShareptr) * srecordCount;
			if( !(srecordtbl = malloc( tblsize )) ) { LOGFAIL; rc=-9; goto EXIT; }
			memset( srecordtbl, 0, tblsize );
		}

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) {
				if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL; goto EXIT; }
				if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL; goto EXIT; }
			} else {
				if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL; goto EXIT; }
			}
		}
		if( rc < 0 ) { LOGFAIL; goto EXIT; }
		if( srecordInit != srecordCount ) { LOGFAIL; rc = RC_INTERNAL; goto EXIT; }
	}
	
	if( strcmp( arecordLoc, location ) != 0 ) { LOGFAIL; rc = RC_LOCATION; goto EXIT; }

	// decrypt

	if( rc = ar_core_decrypt( message_out, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; goto EXIT; }

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
	
	if( !szLocation ) { LOGFAIL; rc = RC_NULL; goto EXIT; }
	if( !szRecordArr ) { LOGFAIL; rc = RC_NULL; goto EXIT; }

	{
		size_t shareArrLen = strlen( szRecordArr );

		if( shareArrLen < 10 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }

		// dup and change delim 

		if( !(recordArr_rw = strdup( szRecordArr )) ) { LOGFAIL; rc=-9; goto EXIT; }

		for( size_t i = 0; i < shareArrLen; i++ ) { if( recordArr_rw[i] == '\n' ) { recordArr_rw[i]=0; } }

		// count shares

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) { arecordCount++; } else { srecordCount++; }
		}
		if( rc < 0 ) { LOGFAIL; goto EXIT; }

		if( arecordCount != 1 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }
		if( srecordCount < 2 ) { LOGFAIL; rc = RC_INSUFFICIENT; goto EXIT; }

		// create share-objects

		{
			size_t tblsize = sizeof(arShareptr) * srecordCount;
			if( !(srecordtbl = malloc( tblsize )) ) { LOGFAIL; rc=-9; goto EXIT; }
			memset( srecordtbl, 0, tblsize );
		}

		ss_init( &ss, recordArr_rw, shareArrLen );
		while( (rc = ss_scan( &ss )) == 1 )
		{
			if( ss.type == 1 ) {
				if( rc = ar_uri_parse_a( &pARecord, ss.s_record ) ) { LOGFAIL; goto EXIT; }
				if( rc = library_uri_location( &arecordLoc, ss.s_record ) ) { LOGFAIL; goto EXIT; }
			} else {
				if( rc = ar_uri_parse_s( &(srecordtbl[ srecordInit++ ]), ss.s_record ) ) { LOGFAIL; goto EXIT; }
			}
		}
		if( rc < 0 ) { LOGFAIL; goto EXIT; }
		if( srecordInit != srecordCount ) { LOGFAIL; rc = RC_INTERNAL; goto EXIT; }
	}

	// validate

	if( strcmp( arecordLoc, szLocation ) != 0 ) { LOGFAIL; rc = RC_INTERNAL; goto EXIT; }

	if( rc = ar_core_check_topic( 0, pARecord, 0, 0 ) ) { LOGFAIL; goto EXIT; } // conv failure code

	if( rc = ar_core_check_signature( 0, pARecord, 0, 0 ) ) { LOGFAIL; goto EXIT; } // conv failure code

	{
		byteptr pBoolArr = 0;
		
		if( invalidBoolArr_out_opt )
		{
			if( !(pBoolArr = malloc( srecordCount )) ) { LOGFAIL; rc=-9; goto EXIT; }
		}
		*invalidBoolArr_out_opt = pBoolArr; // reassign before possible failures below
		
		if( rc = ar_core_check_topic( pBoolArr, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; goto EXIT; } // conv failure code
		
		if( rc = ar_core_check_signature( pBoolArr, pARecord, srecordtbl, srecordCount ) ) { LOGFAIL; goto EXIT; } // conv failure code
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
	
	// now break something
	recordArr[ 3 + strlen(recordArr) / 2 ]++;
	rc = library_uri_validate( &validation, location, recordArr );
	TESTASSERT( rc != 0 );
	free( validation );
	
	free( recordArr );
	free( message );

#endif // _DEBUG

}
