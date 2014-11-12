// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_core.h"
#include "ec_vlong.h"
#include "ar_shamir.h"
#include "ar_util.h"
#include "rc4.h"

#include "version.h" // for version related defines

// some sanity checks
STATICASSERT( AR_VERIFYUNITS < AR_UNITS );
STATICASSERT( AR_SHARECOEFUNITS < AR_UNITS );
STATICASSERT( AR_SESSIONUNITS < AR_UNITS );
STATICASSERT( AR_CRYPTKEYUNITS < AR_UNITS );
STATICASSERT( AR_SIGNKEYUNITS < AR_UNITS );

#if defined(AR_DEMO)
    #define AR_HIDDEN_BYTE 1
#else
    #define AR_HIDDEN_BYTE 0
#endif

////////////////////////////

// Artemis A uri is http://<location>?tp=<topic>&ai=<info>&vf=<verify>&pk=<pubkey>&as=<authsig>&mt=<messagetext>&mc=<messageclue>
//
// Artemis S uri is http://<location>?tp=<topic>&si=<info>&sh=<shareid>|<share>&ss=<sharesig>&sc=<shareclue>
//
// where:
// <info> == <version>|<shares>|<threshold>

// artemis
//  0 url location L is specified as an identifying tag and as a url path
//  1 numshares, threshold and fieldsize are combined into splitinfo I
//  2 plaintext message M is hashed to produce N-bit verification V
//  3 random cipher key C is used on message M to crypt message (producing M')
//  4 split C into shares {C'1...C'n}
//  5 crypted message M' is hashed to produce topic T
//  6 random private key Kpri is used to create a matching public key Kpub
//  7 private key Kpri is applied to hash of L | T | V | I | Kpub to produce authsignature Sa
//  8 private key Kpri is applied to hash of L | T | { 1, ... i, ... n } | { C'1, ... C'i, ... C'n } to produce sharesignatures {S1...Sn}
//  9 private key Kpri and cipher key C are thrown away
// 10 authentication is distributed in the concatenated form L | T | V | I | Kpub | Sa | M' | Mc
// 11 shares are distributed in the concatenated form { L | T | i | C'i | Si | Sc }

// rc4 is cranked between 256 and 511 times before being used on stream
// 256 + (LS 8 bits of cipher key)
#define AR_RC4CRANK_OFFSET	256
#define AR_RC4CRANK_MASK	255

//////////////////////////

static void sha1_process_vlpoint( sha1_context* c, size_t* pdeltalen, byteptr composebuf, vlPoint v )
{
	ar_util_u16_host2packet( pdeltalen, composebuf, sizeof(composebuf), v+1, v[0] );
	sha1_process( c, composebuf, (unsigned)(v[0] * sizeof(word16)) );
}

static void sha1_process_blob16( sha1_context* c, size_t* pdeltalen, byteptr composebuf, word16 blob16 )
{
	ar_util_u16_host2packet( pdeltalen, composebuf, sizeof(composebuf), &blob16, 1 );
	sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
}

//////

static void ar_core_mac_arecord( sha1_context* c, arAuthptr pa, byte version, size_t buflen )
{
	size_t deltalen = 0;
	char composebuf[ sizeof(vlPoint) ];

	sha1_process_vlpoint( c, &deltalen, composebuf, pa->topic );
	sha1_process_vlpoint( c, &deltalen, composebuf, pa->verify );
	sha1_process_blob16( c, &deltalen, composebuf, pa->shares );
	sha1_process_blob16( c, &deltalen, composebuf, pa->threshold );
	sha1_process_blob16( c, &deltalen, composebuf, version );
	sha1_process_vlpoint( c, &deltalen, composebuf, pa->pubkey );
	sha1_process( c, pa->buf, (unsigned)(buflen) );

	memset( composebuf, 0, sizeof(composebuf) );
}

static void ar_core_mac_srecord( sha1_context* c, arShareptr ps, byte version, size_t buflen )
{
	size_t deltalen = 0;
	char composebuf[ sizeof(vlPoint) ];

	sha1_process_vlpoint( c, &deltalen, composebuf, ps->topic );
	sha1_process_blob16( c, &deltalen, composebuf, ps->shares );
	sha1_process_blob16( c, &deltalen, composebuf, ps->threshold );
	sha1_process_blob16( c, &deltalen, composebuf, version );
	sha1_process_blob16( c, &deltalen, composebuf, ps->shareid );
	sha1_process_vlpoint( c, &deltalen, composebuf, ps->share );
	sha1_process( c, ps->buf, (unsigned)(buflen) );

	memset( composebuf, 0, sizeof(composebuf) );
}

//////////////////////////

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, byte numThres, byteptr inbuf, word16 inbuflen, bytetbl clueTbl, byteptr location )
{
	int rc = 0;
	
	gfPoint* gfCryptCoefArr = 0;
	gfPoint* shareArr = 0;
	word16* shareIDArr = 0;

	if( !arecord_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }
	if( !srecordtbl_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*arecord_out = 0;
	*srecordtbl_out = 0;

	if( !inbuf ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !clueTbl ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( inbuflen == 0 ) { rc = RC_NULL; goto EXIT; }

	if( numThres < 2 ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }
	if( numShares < 2 ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }
	if( numShares < numThres ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }

    {
        int clueCount = 0;
        for( byteptr* ppClue = clueTbl; *ppClue; ppClue++ ) { clueCount++; }
        if( clueCount != (numShares +1) ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; } // +1 for message clue
    }

    size_t makeoffbyonebug = 0;

#if defined(AR_DEMO)

    // when length is odd and ( shares > 7 or threshold > 3 ) make bug
    makeoffbyonebug =
        ( ((size_t)inbuflen) >> 1 )
        &
        ( ( (numShares >> 3) | (numThres >> 2) ) == 0 ? 0 : 1 );

#endif

	///////////
	// general vars

	size_t loclen = location ? strlen( location ) : 0;
	if( loclen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	size_t acluelen = strlen( clueTbl[ 0 ] ); // 0 for message clue
	size_t msgoffset = loclen + acluelen + makeoffbyonebug; // msg comes after clue + location

	///////////
	// alloc tmp storage

	if( !(shareArr = malloc( sizeof(gfPoint) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(shareIDArr = malloc( sizeof(word16) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(gfCryptCoefArr = malloc( numThres * sizeof(gfPoint) )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	///////////
	// alloc return values

	size_t abufused = loclen + inbuflen + acluelen;
	size_t astructsize = sizeof(arAuth) + abufused;
	if( !(arecord_out[0] = malloc( astructsize + AR_HIDDEN_BYTE )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( arecord_out[0], 0, astructsize + AR_HIDDEN_BYTE );
	arecord_out[0]->bufmax = abufused;

	size_t stblsize = sizeof(arShareptr) * numShares;
	if( !((*srecordtbl_out) = malloc( stblsize ) )) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( (*arecord_out), 0, stblsize );

	for( int i=0; i<numShares; i++ )
	{
		size_t scluelen = strlen( clueTbl[ i + 1 ] ); // +1 to skip mesage clue
		size_t sbufused = loclen + scluelen;
		size_t sstructsize = sizeof(arShare) + sbufused;

		if( !((*srecordtbl_out)[i] = malloc( sstructsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
		memset( (*srecordtbl_out)[i], 0, sstructsize );
		(*srecordtbl_out)[i]->bufmax = sbufused;
	}

	///////////
	// create verification hash of cleartext

	vlPoint verify;
	{
		sha1Digest digest;
		sha1_context c[1];
		sha1_initial( c );
		sha1_process( c, inbuf, (unsigned)(inbuflen) );
		sha1_final( c, digest );
		vlSetWord32Ptr( verify, AR_VERIFYUNITS, digest );
	}

	///////////
	// create cryptcoefs (and cryptkey), cipher the cleartext and split the shares

	arecord_out[0]->loclen = loclen;
	arecord_out[0]->cluelen = acluelen;
	arecord_out[0]->msglen = inbuflen;

	// fill buf with location, clue (if applic) then message
	memcpy_s( arecord_out[0]->buf, arecord_out[0]->bufmax, location, loclen );
	memcpy_s( arecord_out[0]->buf + loclen, arecord_out[0]->bufmax - loclen, clueTbl[0], acluelen );
	memcpy_s( arecord_out[0]->buf + msgoffset, arecord_out[0]->bufmax - msgoffset, inbuf, inbuflen );

	for( word16 t = 0; t < numThres; t++ )
	{
		vlPoint vlTmp;
		vlSetRandom( vlTmp, AR_SHARECOEFUNITS, &ar_util_rnd16 );
		gfUnpack( gfCryptCoefArr[t], vlTmp );
		gfReduce( gfCryptCoefArr[t] );
	}

	{
		vlPoint vlCryptkey;
		gfPack( gfCryptCoefArr[0], vlCryptkey );

		word32 rc4cranks = AR_RC4CRANK_OFFSET + (AR_RC4CRANK_MASK & vlGetWord16( vlCryptkey, 0 ));

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_u16_host2packet( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, rc4cranks, arecord_out[0]->buf + msgoffset, inbuflen );

		vlClear( vlCryptkey );
	}

	ar_shamir_splitsecret( shareArr, shareIDArr, numShares, gfCryptCoefArr, numThres );

	// cleanup secret coefs
	memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres );

	/////////
	// construct topic from location, cryptext and mclue

	vlPoint topic;
	{
		sha1Digest digest;
		sha1_context c[1];
		sha1_initial( c );

		sha1_process( c, arecord_out[0]->buf, (unsigned)(abufused) );
		sha1_final( c, digest );
		vlSetWord32Ptr( topic, AR_TOPICUNITS, digest );
	}

	/////////
	// construct keypair

	vlPoint priSigningkey;
	vlSetRandom( priSigningkey, AR_SIGNKEYUNITS, &ar_util_rnd16 );

	vlPoint pubSigningkey;
	cpMakePublicKey( pubSigningkey, priSigningkey );

	//////////
	// sign the auth record

	arecord_out[0]->shares = numShares;
	arecord_out[0]->threshold = numThres;

	vlCopy( arecord_out[0]->pubkey, pubSigningkey );
	vlCopy( arecord_out[0]->topic, topic );
	vlCopy( arecord_out[0]->verify, verify );

	cpPair authsig;
	{
		vlPoint mac;
		vlClear( mac );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );

			ar_core_mac_arecord( c, arecord_out[0], AR_VERSION, abufused );

			sha1_final( c, digest );
			vlSetWord32Ptr( mac, AR_MACUNITS, digest );
		}

		for( size_t i = 0; i < 100; i++ )
		{
			vlPoint session;
			vlSetRandom( session, AR_SESSIONUNITS, &ar_util_rnd16 );
			rc = ar_shamir_sign( &authsig, session, priSigningkey, mac );
			if( !rc ) { break; }
			if( rc == RC_ARG ) { continue; }
			if( rc ) { LOGFAIL( rc ); goto EXIT; }
		}
		if( rc ) { LOGFAIL( rc ); goto EXIT; }
	}

	cpCopy( &arecord_out[0]->authsig, &authsig );

	/////////////
	// sign the produced share records

	for( int i=0; i<numShares; i++ )
	{
		vlClear( (*srecordtbl_out)[i]->topic );
		vlCopy( (*srecordtbl_out)[i]->topic, topic );

		(*srecordtbl_out)[i]->shares = numShares;
		(*srecordtbl_out)[i]->threshold = numThres;
		(*srecordtbl_out)[i]->shareid = shareIDArr[i];

		vlPoint vlShare;
		{
			vlClear( vlShare );
			gfPack( shareArr[i], vlShare );
		}
		vlCopy( (*srecordtbl_out)[i]->share, vlShare );

		////////////

		size_t scluelen = ( clueTbl && clueTbl[i+1] ) ? strlen( clueTbl[i+1] ) : 0;
		size_t sbufused = loclen + scluelen;

		(*srecordtbl_out)[i]->loclen = loclen;
		(*srecordtbl_out)[i]->cluelen = scluelen;
			
		memcpy_s( (*srecordtbl_out)[i]->buf,			(*srecordtbl_out)[i]->bufmax,				location,		loclen );
		memcpy_s( (*srecordtbl_out)[i]->buf + loclen,	(*srecordtbl_out)[i]->bufmax - loclen,		clueTbl[i+1],	scluelen );

		//////////
		// construct sharesignature to ensure consistiency between topic, shareid, share and clue

		cpPair sharesig;
		{
			vlPoint mac;
			vlClear( mac );
			{
				sha1Digest digest;
				sha1_context c[1];
				sha1_initial( c );

				ar_core_mac_srecord( c, (*srecordtbl_out)[i], AR_VERSION, sbufused );

				sha1_final( c, digest );
				vlSetWord32Ptr( mac, AR_MACUNITS, digest );
			}

			for( size_t i = 0; i < 100; i++ )
			{
				vlPoint session;
				vlSetRandom( session, AR_SESSIONUNITS, &ar_util_rnd16 );
				rc = ar_shamir_sign( &sharesig, session, priSigningkey, mac );
				if( !rc ) { break; }
				if( rc == RC_ARG ) { continue; }
				if( rc ) { LOGFAIL( rc ); goto EXIT; }
			}
			if( rc ) { LOGFAIL( rc ); goto EXIT; }
		}

		cpCopy( &(*srecordtbl_out)[i]->sharesig, &sharesig );
	}

	// double-check

	if( rc = ar_core_check_topic( 0, *arecord_out, (*srecordtbl_out), numShares ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_core_check_signature( 0, *arecord_out, (*srecordtbl_out), numShares ) ) { LOGFAIL( rc ); goto EXIT; }

EXIT:

	if( gfCryptCoefArr ) { memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres ); free( gfCryptCoefArr ); }
	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numShares ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numShares ); free( shareIDArr ); }

	if( rc && arecord_out && *arecord_out ) { free( *arecord_out ); *arecord_out = 0; }
	if( rc && srecordtbl_out && *srecordtbl_out ) {
		for( word16 i=0; i<numShares; i++ ) { if( (*srecordtbl_out)[i] ) free( (*srecordtbl_out)[i] ); }
		free( *srecordtbl_out );
	}
	
	// check baked-location and cause a signature-fail when loc hashes wrong
	if( arecord_out && *arecord_out )
	{
		(*arecord_out)->authsig.r[1] += ( ar_util_strcrc( location ) ^ AR_LOCHASH ) ? 1 : 0;
	}

	return rc;
}

int ar_core_decrypt( byteptr* buf_out, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords )
{
	int rc = 0;

	gfPoint* shareArr = 0;
	word16* shareIDArr = 0;

	if( !buf_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*buf_out = 0;

	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !srecordtbl ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	for( int i = 0; i < numSRecords; i++ )
	{
		if( !srecordtbl[i] ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	}

	if( numSRecords < arecord->threshold ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }
	
	///////////
	// check topic consistiency: internally to ARecord, then compared to all SRecords

	if( rc = ar_core_check_topic( 0, arecord, srecordtbl, numSRecords ) ) { LOGFAIL( rc ); goto EXIT; }

	////////////
	// check authsignature to ensure location, clue, topic and pubkey have consistient pairing

	if( rc = ar_core_check_signature( 0, arecord, srecordtbl, numSRecords ) ) { LOGFAIL( rc ); goto EXIT; }

	///

	if( !(shareArr = malloc( sizeof(gfPoint) * numSRecords )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(shareIDArr = malloc( sizeof(word16) * numSRecords )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	
	for( int i=0; i<numSRecords; i++ )
	{
		if( 1 == gfUnpack( shareArr[i], srecordtbl[i]->share ) ) { rc = RC_SHARD; LOGFAIL( rc ); goto EXIT; }
		shareIDArr[i] = srecordtbl[i]->shareid;
	}

	///

	if( !(*buf_out = malloc( arecord->msglen )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	{	
		byteptr cryptext = arecord->buf + arecord->loclen + arecord->cluelen;
		memcpy_s( *buf_out, arecord->msglen, cryptext, arecord->msglen );
	}

	{
		gfPoint gfCryptkey;
		ar_shamir_recoversecret( gfCryptkey, shareIDArr, shareArr, numSRecords );

		vlPoint vlCryptkey;
		gfPack( gfCryptkey, vlCryptkey );

		word32 rc4cranks = AR_RC4CRANK_OFFSET + (AR_RC4CRANK_MASK & vlGetWord16( vlCryptkey, 0 ));

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_u16_host2packet( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, rc4cranks, *buf_out, arecord->msglen );

		gfClear( gfCryptkey );
	}

	///////////
	// check if shares decoded properly

	{
		vlPoint verify;
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );
			sha1_process( c, *buf_out, (unsigned)(arecord->msglen) );
			sha1_final( c, digest );
			vlSetWord32Ptr( verify, AR_VERIFYUNITS, digest );
		}
		if( !vlEqual( verify, arecord->verify ) ) { rc = RC_MESSAGE; LOGFAIL( rc ); goto EXIT; }
	}

EXIT:

	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numSRecords ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numSRecords ); free( shareIDArr ); }

	if( rc && buf_out && *buf_out ) { free( *buf_out ); *buf_out = 0; }

	return rc;
}

int ar_core_check_topic( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords )
{
	int rc = 0;

	// add marking for 'fail' to all
	if( buf_opt ) { for( int i=0; i<numSRecords; i++ ) { buf_opt[i] = 0xFF; } }

	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	// check internal topic consistiency for ARecord

	vlPoint topic;
	{
		sha1Digest digest;
		sha1_digest( digest, arecord->buf, arecord->msglen + arecord->loclen + arecord->cluelen );
		vlSetWord32Ptr( topic, AR_TOPICUNITS, digest );
	}
	if( !vlEqual( arecord->topic, topic ) ) { rc = RC_AUTH_TOPIC; LOGFAIL( rc ); goto EXIT; }

	if( srecordtbl_opt && numSRecords > 0 )
	{
		// compare ARecord topic to all specified SRecords

		for( int i=0; i<numSRecords; i++ )
		{
			int fail = !vlEqual( topic, srecordtbl_opt[i]->topic );

			// return nz rc if there is any failure
			if( fail ) { rc = RC_TOPIC; LOGFAIL( rc ); if( !buf_opt ) { goto EXIT; } }
			else {
				if( buf_opt ) { buf_opt[i] = 0; } // clear individual fail markings
			}
		}
	}

EXIT:
	return rc;
}

int ar_core_check_signature( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords )
{
	int rc = 0;

	// set 'fail' markings
	if( buf_opt ) { for( int i=0; i<numSRecords; i++ ) { buf_opt[i] = 0xFF; } }

	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	// check authsignature to ensure data integreity

	size_t abufused = arecord->msglen + arecord->loclen + arecord->cluelen;

	{
		vlPoint mac;
		vlClear( mac );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );

			ar_core_mac_arecord( c, arecord, AR_VERSION, abufused );

			sha1_final( c, digest );
			vlSetWord32Ptr( mac, AR_MACUNITS, digest );
		}
		if( !cpVerify( arecord->pubkey, mac, &arecord->authsig ) ) { rc = RC_AUTH_SIGNATURE; LOGFAIL( rc ); goto EXIT; }
	}

	// check sharesignatures to ensure data integreity

	if( srecordtbl_opt && numSRecords > 0 )
	{
		for( int i=0; i<numSRecords; i++ )
		{
			arShareptr pSRecord = srecordtbl_opt[i];
			size_t sbufused = pSRecord->loclen + pSRecord->cluelen;
			//
			vlPoint mac;
			vlClear( mac );
			{
				sha1Digest digest;
				sha1_context c[1];
				sha1_initial( c );

				ar_core_mac_srecord( c, srecordtbl_opt[i], AR_VERSION, sbufused );

				sha1_final( c, digest );
				vlSetWord32Ptr( mac, AR_MACUNITS, digest );
			}
			int fail = !cpVerify( arecord->pubkey, mac, &pSRecord->sharesig );

			// return nz rc if there is any failure
			if( fail ) { rc = RC_SIGNATURE; LOGFAIL( rc ); if( !buf_opt ) { goto EXIT; } }
			else {
				if( buf_opt ) { buf_opt[i] = 0; } // remove individual fail markings
			}
		}
	}

EXIT:
	return rc;
}

void ar_core_test()
{

#if defined(ENABLE_TESTS)

	printf("# ar_core_test\n");

	arAuthptr arecord = 0;
	arSharetbl srecordtbl = 0;
	char* reftextin = "dog food.";
	byteptr cleartextin = 0;
	byteptr cleartext_out = 0;
	char* cluetbl[4] = {"topiclue", "clue1", "clue2", 0}; // is a table, must end with 0
	int rc = 0;

	byte checkarr[2] = {0,0};

	if( !(cleartextin = malloc( strlen(reftextin)+1 )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	strcpy_s( cleartextin, strlen(reftextin)+1, reftextin );

	rc = ar_core_create( &arecord, &srecordtbl, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1), (bytetbl)cluetbl, AR_LOCATION ); // +1 to include \0
	TESTASSERT( rc == 0 );

	rc = ar_core_check_topic( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );
	TESTASSERT( checkarr[0] == 0 );
	TESTASSERT( checkarr[1] == 0 );

	rc = ar_core_decrypt( &cleartext_out, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );

	TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );

#if defined(ENABLE_FUZZING)

	///////////////////
	// now start breaking things....

	srecordtbl[1]->topic[0] = 0; // break topic of share 2

	rc = ar_core_check_topic( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );
	TESTASSERT( !checkarr[0] );
	TESTASSERT( checkarr[1] );

	rc = ar_core_check_signature( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );
	TESTASSERT( !checkarr[0] );
	TESTASSERT( checkarr[1] );

	arecord->topic[0] = 0; // break auth record topic

	rc = ar_core_check_topic( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );
	TESTASSERT( checkarr[0] ); // all fail
	TESTASSERT( checkarr[1] );

	rc = ar_core_check_signature( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );
	TESTASSERT( checkarr[0] ); // all fail
	TESTASSERT( checkarr[1] );

#endif // ENABLE_FUZZING

EXIT:

	if( cleartextin ) free( cleartextin );
	if( cleartext_out ) free( cleartext_out );
	if( arecord ) free( arecord );
	for( size_t i = 0; i < 2; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
	if( srecordtbl ) free( srecordtbl );

#endif
}
