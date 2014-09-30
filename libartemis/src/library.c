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

int library_uri_encoder( byteptr* sharesArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, byteptr message )
{
	int rc = 0;

	*sharesArr_out = 0;
	
	arAuthptr arecord = 0;
	arSharetbl srecordtbl = 0;
	bytetbl clueTbl = 0;
	byteptr clueArr_rw = 0;

	if( threshold == 0 || shares == 0 || threshold > shares ) { ASSERT(0); rc=-1; goto FAILCRYPT; }
	
	if( !sharesArr_out || !szLocation || !clueArr || !message ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	// change delimiters of clueArr
	clueArr_rw = strdup( clueArr ); // make writable version
	if( !clueArr_rw ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t cluePtrArrLen = strlen( clueArr_rw );
	for( size_t i = 0; i < cluePtrArrLen; i++ ) { if( clueArr_rw[i]=='\n' ) { clueArr_rw[i]='\0'; } }
	if( rc = ar_util_buildByteTbl( &clueTbl, clueArr_rw, cluePtrArrLen ) ) { ASSERT(0); goto FAILCRYPT; }

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

	size_t messlen = message ? strlen( message ) : 0;
	if( messlen == 0 ) { ASSERT(0); rc=-1; goto FAILCRYPT; }

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
	rc = ar_core_create( &arecord, &srecordtbl, shares, threshold, message, (word16)messlen+1, clueTbl, szLocation );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	// size the output buffer...

	size_t outbufsize = 0;
	size_t bufsize = 0;
	ar_uri_bufsize_a( &bufsize, arecord );
	outbufsize = bufsize;
	for( int i = 0; i < shares; i++ )
	{
		size_t bufsize = 0;
		ar_uri_bufsize_s( &bufsize, srecordtbl[i] );
		outbufsize += bufsize;
	}

	if( !(*sharesArr_out = malloc( outbufsize )) ) { ASSERT(0); rc=-9; goto FAILCRYPT; }
	memset( *sharesArr_out, 0, outbufsize );

	// start concating records to the output buffer

	(*sharesArr_out)[0] = 0;
	rc = ar_uri_create_a( (*sharesArr_out), outbufsize, arecord );
	if( rc ) { ASSERT(0); goto FAILCRYPT; }

	for( word16 s=0; s!=shares; s++ )
	{
		if( rc = ar_util_strcat( (*sharesArr_out), outbufsize, "\n" ) ) { ASSERT(0); goto FAILCRYPT; }
		if( rc = ar_uri_create_s( (*sharesArr_out), outbufsize, srecordtbl[s] ) ) { ASSERT(0); goto FAILCRYPT; }
	}

FAILCRYPT:

	if( arecord ) free( arecord );
	for( word16 i=0; i<shares; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
	if( srecordtbl ) free( srecordtbl );
	if( clueArr_rw ) free( clueArr_rw );
	if( clueTbl ) free( clueTbl );

	if( rc && *sharesArr_out ) free( *sharesArr_out );
	
	return rc;
}

int library_uri_decoder( byteptr* message_out, byteptr szLocation, byteptr shareArr )
{
	int rc = 0;

	*message_out = 0;

	size_t messlen = 0;
	size_t cluelen = 0;
	word16 threshold = 0;
	word16 sharenum = 0;

	arAuth* pARecord = 0;
	arShare* pSRecord = 0;
	arShareptr* srecordTbl = 0;

	byteptr shareArr_rw = 0;

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { ASSERT(0); rc=-1; goto FAIL; }

	shareArr_rw = strdup( shareArr );
	if( !shareArr_rw ) { ASSERT(0); return -9; }

	size_t shareArrLen = strlen( shareArr );

	word16 shares = 0;
	for( size_t i = 0; i < shareArrLen; i++ ) { if( shareArr_rw[i] == '\n' ) { shareArr_rw[i]=0; shares++; } }

	byteptr end = shareArr_rw + shareArrLen;
	byteptr s_record = shareArr_rw;
	byteptr e_record = shareArr_rw;

	while( s_record < end )
	{
		while( *e_record != '\0' ) { e_record++; } // now \0 delimited
		*e_record = 0;
		byteptr inbuf = s_record;

		int type = ar_uri_parse_type( inbuf );
		if( type == -1 ) { ASSERT(0); rc=-9; goto FAIL; }

		size_t buflen = 0;
		ar_uri_parse_vardatalen( &buflen, inbuf );

		if( type == 1 )
		{
			if( !pARecord )
			{
				size_t structlen = sizeof(arAuth) + buflen;
				if( !(pARecord = malloc( structlen )) ) { ASSERT(0); rc=-9; goto FAIL; }
				memset( pARecord, 0, structlen );

				pARecord->bufmax = (word16)buflen;
			}

			if( ar_uri_parse_a( pARecord, inbuf, szLocation ) ) { ASSERT(0); rc=-9; goto FAIL; }
		}
		else
		{
			if( !srecordTbl )
			{
				size_t tbllen = sizeof(arShareptr) * shares;
				if( !(srecordTbl = malloc( tbllen )) ) { ASSERT(0); rc=-9; goto FAIL; }
				memset( srecordTbl, 0, tbllen );
			}

			size_t structlen = sizeof(arShare) + buflen;
			if( !(pSRecord = malloc( structlen )) ) { ASSERT(0); rc=-9; goto FAIL; }
			memset( pSRecord, 0, structlen );

			pSRecord->bufmax = (word16)buflen;

			srecordTbl[sharenum++] = pSRecord;

			if( ar_uri_parse_s( pSRecord, inbuf, szLocation ) ) { ASSERT(0); rc=-9; goto FAIL; }
		}

		s_record = e_record = e_record + 1; // +1 for char after \0
	}

	if( !pARecord || !srecordTbl ) { rc=-9; goto FAIL; } // no assert here

	if( rc = ar_core_decrypt( message_out, pARecord, srecordTbl, sharenum ) ) { ASSERT(0); goto FAIL; }

FAIL:

	if( pARecord ) free( pARecord );
	for( int i=0; i<sharenum; i++ ) { if( srecordTbl[i] ) { free( srecordTbl[i] ); srecordTbl[i] = 0; } }
	if( srecordTbl ) free( srecordTbl );
	if( shareArr_rw ) free( shareArr_rw );

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

int library_uri_validate( byteptr* validation_out_opt, byteptr szLocation, byteptr szARrecord, byteptr szSRecordArr_opt )
{
	int rc = 0;

	word16 srecordCount = 0;

	arAuth* pARecord = 0;

	byteptr		szSRecordArr_rw = 0;
	bytetbl		szSRecordTbl = 0;
	arSharetbl	srecordtbl = 0;

	if( validation_out_opt ) { *validation_out_opt = 0; }

	if( !szLocation ) { ASSERT(0); return -1; }
	if( !szARrecord ) { ASSERT(0); return -1; }

	// validate arecord first

	{
		size_t buflen = 0;
		ar_uri_parse_vardatalen( &buflen, szARrecord );

		size_t structsize = sizeof(arAuth) + buflen;
		if( !(pARecord = malloc( structsize )) ) { ASSERT(0); rc=-9; goto EXIT; }
		memset( pARecord, 0, structsize );

		pARecord->bufmax = (word16)buflen;

		if( rc = ar_uri_parse_a( pARecord, szARrecord, szLocation ) ) { ASSERT(0); goto EXIT; }
	}

	if( rc = ar_core_check_topic( 0, pARecord, 0, 0 ) ) { ASSERT(0); rc = (rc==-2) ? -2 : rc; goto EXIT; } // conv failure code
	if( rc = ar_core_check_signature( 0, pARecord, 0, 0 ) ) { ASSERT(0); rc = (rc==-2) ? -3 : rc; goto EXIT; } // conv failure code

	// optionally validate srecords

	if( szSRecordArr_opt )
	{
		szSRecordArr_rw = strdup( szSRecordArr_opt );
		if( !szSRecordArr_rw ) { ASSERT(0); return -9; }

		size_t shareArrLen = strlen( szSRecordArr_rw );

		srecordCount = 1;
		for( size_t i = 0; i < shareArrLen; i++ ) { if( szSRecordArr_rw[i] == '\n' ) { szSRecordArr_rw[i]=0; srecordCount++; } }

		// init/make pointers to strings

		if( rc = ar_util_buildByteTbl( &szSRecordTbl, szSRecordArr_rw, shareArrLen ) ) { ASSERT(0); goto EXIT; }

		// make pointers to objects

		size_t tblsize = sizeof(arShareptr) * srecordCount;
		if( !(srecordtbl = malloc( tblsize )) ) { ASSERT(0); rc=-9; goto EXIT; }
		memset( srecordtbl, 0, tblsize );

		// init pointers to objects

		for( size_t i = 0 ; i < srecordCount; i++ )
		{
			arShare* pSRecord = 0; // local var

			size_t buflen = 0;
			ar_uri_parse_vardatalen( &buflen, szSRecordTbl[i] );

			size_t structsize = sizeof(arShare) + buflen;
			if( !(pSRecord = malloc( structsize )) ) { ASSERT(0); rc=-9; goto EXIT; }
			memset( pSRecord, 0, structsize );

			pSRecord->bufmax = (word16)buflen;

			srecordtbl[i] = pSRecord;

			if( ar_uri_parse_s( pSRecord, szSRecordTbl[i], szLocation ) ) { ASSERT(0); rc=-9; goto EXIT; }
		}

		// check objects

		if( !(*validation_out_opt = malloc( srecordCount )) ) { ASSERT(0); rc=-9; goto EXIT; }

		if( rc = ar_core_check_topic( (*validation_out_opt), pARecord, srecordtbl, srecordCount ) ) { ASSERT(0); rc = (rc==-2) ? -4 : rc; goto EXIT; } // conv failure code

		if( rc = ar_core_check_signature( (*validation_out_opt), pARecord, srecordtbl, srecordCount ) ) { ASSERT(0); rc = (rc==-2) ? -5 : rc; goto EXIT; } // conv failure code
	}

EXIT:

	if( pARecord ) free( pARecord );
	for( int i=0; i<srecordCount; i++ ) { if( srecordtbl[i] ) { free( srecordtbl[i] ); } }
	if( srecordtbl ) free( srecordtbl );
	if( szSRecordTbl ) free( szSRecordTbl );
	if( szSRecordArr_rw ) free( szSRecordArr_rw );

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

	byteptr message_out;
	byteptr validation_out;

	rc = library_uri_encoder( &sharesArr_out, shares, threshold, szLocation, clueArr, message );
	TESTASSERT( rc == 0 );

	rc = library_uri_decoder( &message_out, szLocation, sharesArr_out );
	TESTASSERT( rc == 0 );

	TESTASSERT( strcmp( message, message_out ) == 0 );

	rc = library_uri_validate( &validation_out, szLocation, sharesArr_out, strchr( sharesArr_out, '\n' ) + 1 );
	TESTASSERT( rc == 0 );
	free( validation_out );

	// now break something
	sharesArr_out[ strlen(sharesArr_out) / 2 ] = 'X';
	rc = library_uri_validate( &validation_out, szLocation, sharesArr_out, strchr( sharesArr_out, '\n' ) + 1 );
	TESTASSERT( rc == -3 );
	free( validation_out );

	free( sharesArr_out );
	free( message_out );

#endif // _DEBUG

}
