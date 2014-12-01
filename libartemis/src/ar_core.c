// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_codes.h"
#include "ar_util.h"

#include "ar_core.h"
#include "ec_vlong.h"
#include "ar_shamir.h"
#include "ar_util.h"
#include "rc4.h"

#include "version.h" // for version related defines

// some sanity checks
STATICASSERT( AR_MACUNITS < VL_UNITS );
STATICASSERT( AR_CRYPTKEYUNITS < VL_UNITS );
STATICASSERT( AR_SESSKEYUNITS < VL_UNITS );
STATICASSERT( AR_PRIVKEYUNITS < VL_UNITS );

#if defined(AR_DEMO)
    #define AR_HIDDEN_BYTE 1
#else
    #define AR_HIDDEN_BYTE 0
#endif

////////////////////////////

// Artemis A uri is http://<location>?tp=<auth.pubkey>&ai=<info>&as=<authsig.r>|<authsig.r>&mt=<messagetext>&mc=<messageclue>
//
// Artemis S uri is http://<location>?tp=<auth.pubkey>&si=<info>&ss=<sharesig.r>|<sharesig.s>&sh=<shareid>|<share>&sc=<shareclue>
//
// where:
// <info> == <version>|<shares>|<threshold>

// artemis
//  0 url location L is specified as an identifying tag and as a url path
//  1 version, numshares, threshold are combined into info I
//  2 random cipher key C is used on message M to crypt message (producing M')
//  3 split C into shares {C'1...C'n} as C'i
//  4 random private key Kpri is used to create a matching public key Kpub
//  5 private key Kpri is applied to hash of Kpub | T | I to produce authsignature Sa
//  6 private key Kpri is applied to hash of Kpub | T | I | { 1, ... i, ... n } | { C'1, ... C'i, ... C'n } to produce sharesignatures {S1...Sn}
//  7 private key Kpri and cipher key C are thrown away
//  8 authentication is distributed in the concatenated form L | Kpub | I | Sa | M' | Mc
//  9 shares are distributed in the concatenated form      { L | Kpub | I | Si | i | C'i | Sc }

//////////////////////////

static INLINE byte strcrc( byteptr s ) { byte x = 0x41; while( *s ) { x += (x << 1) ^ *(s++); } return x; }

static void sha1_process_vlpoint( sha1_context* c, vlPoint v )
{
	size_t deltalen = 0;
	char composebuf[ sizeof(vlPoint) +sizeof(vlunit) ]; // +sizeof(vlunit) for spare
	ar_util_u16_host2packet( &deltalen, composebuf, composebuf + sizeof(vlPoint), v+1, v[0] );
	sha1_process( c, composebuf, (unsigned)(v[0] * sizeof(vlunit)) );
	memset( composebuf, 0, sizeof(composebuf) );
}

static void sha1_process_blob16( sha1_context* c, word16 blob16 )
{
	size_t deltalen = 0;
	char composebuf[ sizeof(word16) ];
	ar_util_u16_host2packet( &deltalen, composebuf, composebuf + sizeof(word16), &blob16, 1 );
	sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
	memset( composebuf, 0, sizeof(composebuf) );
}

static void ar_core_mac_arecord( vlPoint mac, arAuthptr pa, byte version, size_t buflen )
{
	sha1Digest digest;
	sha1_context c[1];
	sha1_initial( c );

	sha1_process_vlpoint( c, pa->pubkey );
	sha1_process_blob16( c, pa->shares );
	sha1_process_blob16( c, pa->threshold );
	sha1_process_blob16( c, version );
	sha1_process( c, pa->buf, (unsigned)(buflen) );

	sha1_final( c, digest );
	vlSetWord32Ptr( mac, AR_MACUNITS, digest );

	sha1_clear( c );
}

static void ar_core_mac_srecord( vlPoint mac, arShareptr ps, byte version, size_t buflen )
{
	sha1Digest digest;
	sha1_context c[1];
	sha1_initial( c );

	sha1_process_vlpoint( c, ps->pubkey );
	sha1_process_blob16( c, ps->shares );
	sha1_process_blob16( c, ps->threshold );
	sha1_process_blob16( c, version );
	sha1_process_blob16( c, ps->shareid );
	sha1_process_vlpoint( c, ps->share );
	sha1_process( c, ps->buf, (unsigned)(buflen) );

	sha1_final( c, digest );
	vlSetWord32Ptr( mac, AR_MACUNITS, digest );

	sha1_clear( c );
}

static int ar_core_sign( cpPair* sig, const vlPoint session, const vlPoint vlPublicKey, const vlPoint vlPrivateKey, const vlPoint mac )
{
	int rc = 0;

	// ensure pubkey is valid, sign mac and verify
	ecPoint t2;
	if( ecUnpack( &t2, vlPublicKey ) ) { rc = RC_PRIVATEKEY; LOGFAIL( rc ); goto EXIT; }
	cpSign( vlPrivateKey, session, mac, sig );
	if( vlIsZero( sig->r ) ) { rc = RC_PRIVATEKEY; LOGFAIL( rc ); goto EXIT; }
	if( !cpVerify( vlPublicKey, mac, sig ) ) { rc = RC_PRIVATEKEY; LOGFAIL( rc ); goto EXIT; }
EXIT:
	return rc;
}

static int ar_core_makekeypair( vlPoint pub, vlPoint pri )
{
	size_t i = 0;
	int rc = RC_PRIVATEKEY;
	while( rc == RC_PRIVATEKEY )
	{
		if( ++i == 100 ) { break; } // 100 is big and will create failures in lieu of lockups

		vlSetRandom( pri, AR_PRIVKEYUNITS, &ar_util_rnd16 );
		cpMakePublicKey( pub, pri );

		vlPoint mac;
		vlSetRandom( mac, AR_MACUNITS, &ar_util_rnd16 );

		vlPoint session;
		vlSetRandom( session, AR_SESSKEYUNITS, &ar_util_rnd16 );

		cpPair sig;
		rc = ar_core_sign( &sig, session, pub, pri, mac );

		vlClear(mac);
		vlClear(session);
	}
	if( rc ) { LOGFAIL( rc ); goto EXIT; }
EXIT:
	return rc;
}

static void ar_core_rc4( byteptr buf, size_t buflen, gfPoint gfKey )
{
	vlPoint vlKey;
	gfPack( gfKey, vlKey );

	word32 rc4cranks = 1000;
	size_t deltalen = 0;
	byte packetEndianBArr[ sizeof(vlPoint) +sizeof(vlunit) ]; // +sizeof(vlunit) for spare

	ar_util_u16_host2packet( &deltalen, packetEndianBArr, packetEndianBArr + sizeof(vlPoint), vlKey + 1, vlKey[0] );
	rc4( packetEndianBArr, vlKey[0] *sizeof(vlunit), rc4cranks, buf, (word32)buflen );

	vlClear( vlKey );
}

//////////////

int ar_core_check_arecord( byteptr szLocation, arAuthptr arecord )
{
	int rc = 0;

	if( !szLocation ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( memcmp( szLocation, arecord->buf, arecord->loclen ) ) { rc = RC_LOCATION; LOGFAIL( rc ); goto EXIT; }

	vlPoint mac;
	ar_core_mac_arecord( mac, arecord, AR_VERSION, arecord->msglen + arecord->loclen + arecord->cluelen );

	if( !cpVerify( arecord->pubkey, mac, &arecord->authsig ) ) { rc = RC_SIGNATURE; LOGFAIL( rc ); goto EXIT; }

EXIT:
	return rc;
}

int ar_core_check_srecord( byteptr szLocation, arShareptr srecord )
{
	int rc = 0;

	if( !szLocation ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !srecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( memcmp( szLocation, srecord->buf, srecord->loclen ) ) { rc = RC_LOCATION; LOGFAIL( rc ); goto EXIT; }

	vlPoint mac;
	ar_core_mac_srecord( mac, srecord, AR_VERSION, srecord->loclen + srecord->cluelen );

	if( !cpVerify( srecord->pubkey, mac, &srecord->sharesig ) ) { rc = RC_SIGNATURE; LOGFAIL( rc ); goto EXIT; }

EXIT:
	return rc;
}

//////////////////////////

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, byte numThres, byteptr inbuf, word16 inbuflen, bytetbl clueTbl, byteptr szLocation )
{
	int rc = 0;
	STACKGAP();
	
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

	if( rc = ar_util_checkTbl( (void**)clueTbl, (numShares +1) ) ) { LOGFAIL( rc ); goto EXIT; }

	size_t bug = 0;

#if defined(AR_DEMO)

#define BIT_EQUAL(a,b) ( ~( (a) ^ (b) ) & (b) )
#define BIT_MASK(a,b)  ( ~( (a) & (b) ) ^ (b) )

	// anti-crack: raise bug when we're called outside demo limits and address has bits 5 & 2
    bug = ( ((numShares >> 3) | (numThres >> 2)) & BIT_MASK( (size_t)inbuf, 0x22) ) ? 1 : 0;

#endif

	///////////
	// general vars

	size_t loclen = szLocation ? strlen( szLocation ) : 0;
	if( loclen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	size_t acluelen = strlen( clueTbl[ 0 ] ); // 0 for message clue
	size_t msgoffset = loclen + acluelen +bug; // msg comes after clue + location, +bug to conditionally prefix message with \0

	///////////
	// alloc tmp storage

	{
		word32 ordering[3] = {0,1,2};

#if !defined(_DEBUG) || defined(ENABLE_FUZZING)
		ar_util_rnd32_reorder( ordering, sizeof(ordering)/sizeof(word32) );
#endif

		for( size_t i=0; i<sizeof(ordering)/sizeof(word32); i++ )
		{
			switch( ordering[i] )
			{
			case 0: if( !(shareArr = malloc( sizeof(gfPoint) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; } break;
			case 1: if( !(shareIDArr = malloc( sizeof(word16) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; } break;
			case 2: if( !(gfCryptCoefArr = malloc( numThres * sizeof(gfPoint) )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; } break;
			}
		}
	}

	///////////
	// alloc return values

	size_t abufused = loclen + inbuflen + acluelen;
	if( abufused > 0xFFFF ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	size_t astructsize = sizeof(arAuth) + abufused;
	if( !(arecord_out[0] = malloc( astructsize + AR_HIDDEN_BYTE )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( arecord_out[0], 0, astructsize + AR_HIDDEN_BYTE );
	arecord_out[0]->bufmax = (word16)abufused;

	size_t stblsize = sizeof(arShareptr) * numShares;
	if( !((*srecordtbl_out) = malloc( stblsize ) )) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( (*srecordtbl_out), 0, stblsize );

	for( int i=0; i<numShares; i++ )
	{
		size_t scluelen = strlen( clueTbl[ i +1 ] ); // +1 to skip mesage clue
		size_t sbufused = loclen + scluelen;
		size_t sstructsize = sizeof(arShare) + sbufused;

		if( !((*srecordtbl_out)[i] = malloc( sstructsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
		memset( (*srecordtbl_out)[i], 0, sstructsize );
		(*srecordtbl_out)[i]->bufmax = (word16)sbufused;
	}

	arecord_out[0]->loclen = (word16)loclen;
	arecord_out[0]->cluelen = (word16)acluelen;
	arecord_out[0]->msglen = inbuflen;

	// fill buf with location, clue (if applic) then message
	{
		byteptr bufend = arecord_out[0]->buf + arecord_out[0]->bufmax;
		if( rc = ar_util_memcpy( arecord_out[0]->buf,				bufend, szLocation, loclen ) ) { LOGFAIL( rc ); goto EXIT; }
		if( rc = ar_util_memcpy( arecord_out[0]->buf + loclen,		bufend, clueTbl[0], acluelen ) ) { LOGFAIL( rc ); goto EXIT; }
		if( rc = ar_util_memcpy( arecord_out[0]->buf + msgoffset,	bufend, inbuf, inbuflen ) ) { LOGFAIL( rc ); goto EXIT; }
	}

	// add cleartext to entropypool, prior to gen of cryptkey
	ar_util_rndcrank( arecord_out[0]->buf, abufused );

	// create cryptcoefs (and cryptkey)
	for( word16 t = 0; t < numThres; t++ )
	{
		vlPoint vlTmp;
		vlSetRandom( vlTmp, AR_CRYPTKEYUNITS, &ar_util_rnd16 );
		gfUnpack( gfCryptCoefArr[t], vlTmp );
		gfReduce( gfCryptCoefArr[t] );
		vlClear(vlTmp);
	}

	// cipher the cleartext
	ar_core_rc4( arecord_out[0]->buf + msgoffset, inbuflen, gfCryptCoefArr[0] );

	// split the shares
	ar_shamir_splitsecret( shareArr, shareIDArr, numShares, gfCryptCoefArr, numThres );

	// cleanup secret coefs
	memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres );

	/////////
	// construct a valid keypair

	vlPoint pubSigningkey, priSigningkey;

	if( rc = ar_core_makekeypair( pubSigningkey, priSigningkey ) ) { LOGFAIL( rc ); goto EXIT; }

	//////////
	// sign the auth record

	arecord_out[0]->shares = numShares;
	arecord_out[0]->threshold = numThres;

	vlCopy( arecord_out[0]->pubkey, pubSigningkey );

	{
		vlPoint mac;
		ar_core_mac_arecord( mac, arecord_out[0], AR_VERSION, abufused );

		vlPoint session;
		vlSetRandom( session, AR_SESSKEYUNITS, &ar_util_rnd16 );

		if( rc = ar_core_sign( &arecord_out[0]->authsig, session, pubSigningkey, priSigningkey, mac ) )
		{ vlClear(session); LOGFAIL( rc ); goto EXIT; }

		vlClear(session); 
	}

	/////////////
	// sign the produced share records

	for( int i=0; i<numShares; i++ )
	{
		vlCopy( (*srecordtbl_out)[i]->pubkey, pubSigningkey );

		(*srecordtbl_out)[i]->shares = numShares;
		(*srecordtbl_out)[i]->threshold = numThres;
		(*srecordtbl_out)[i]->shareid = shareIDArr[i];

		gfPack( shareArr[i], (*srecordtbl_out)[i]->share );

		////////////

		size_t scluelen = ( clueTbl && clueTbl[i+1] ) ? strlen( clueTbl[i+1] ) : 0;
		size_t sbufused = loclen + scluelen;
		if( sbufused > 0xFFFF ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

		(*srecordtbl_out)[i]->loclen = (word16)loclen;
		(*srecordtbl_out)[i]->cluelen = (word16)scluelen;

		{
			byteptr bufend = (*srecordtbl_out)[i]->buf + (*srecordtbl_out)[i]->bufmax;
			if( rc = ar_util_memcpy( (*srecordtbl_out)[i]->buf,				bufend,	szLocation,		loclen ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_memcpy( (*srecordtbl_out)[i]->buf + loclen,	bufend,	clueTbl[i +1],	scluelen ) ) { LOGFAIL( rc ); goto EXIT; } // +1 skips message clue
		}

		//////////
		// construct sharesignature to ensure consistiency between topic, shareid, share and clue

		{
			vlPoint mac;
			ar_core_mac_srecord( mac, (*srecordtbl_out)[i], AR_VERSION, sbufused );

			vlPoint session;
			vlSetRandom( session, AR_SESSKEYUNITS, &ar_util_rnd16 );

			if( rc = ar_core_sign( &(*srecordtbl_out)[i]->sharesig, session, pubSigningkey, priSigningkey, mac ) )
			{ vlClear(session); LOGFAIL( rc ); goto EXIT; }

			vlClear(session);
		}
	}

	// double-check signatures

	if( rc = ar_core_check_recordset( szLocation, *arecord_out, *srecordtbl_out, numShares ) ) { LOGFAIL( rc ); goto EXIT; }

EXIT:

	// add entropy to pool, so we don't leave it available for easy snooping
	ar_util_rndcrank(0,0);

	if( gfCryptCoefArr ) { memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres ); free( gfCryptCoefArr ); }
	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numShares ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numShares ); free( shareIDArr ); }

	if( rc && arecord_out && *arecord_out ) { free( *arecord_out ); *arecord_out = 0; }
	if( rc && srecordtbl_out && *srecordtbl_out ) {
		for( word16 i=0; i<numShares; i++ ) { if( (*srecordtbl_out)[i] ) free( (*srecordtbl_out)[i] ); }
		free( *srecordtbl_out );
	}
	
	// anti-crack: check baked-location and cause a signature-fail when loc hashes wrong
	if( arecord_out && *arecord_out )
	{
		(*arecord_out)->authsig.r[1] += ( strcrc( szLocation ) ^ AR_LOCHASH );
	}

	return rc;
}

int ar_core_decrypt( byteptr* buf_out, byteptr szLocation, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords )
{
	int rc = 0;
	STACKGAP();

	gfPoint* shareArr = 0;
	word16* shareIDArr = 0;

	if( !buf_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*buf_out = 0;

	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !srecordtbl ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( numSRecords < arecord->threshold ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }

	if( rc = ar_util_checkTbl( (void**)srecordtbl, numSRecords ) ) { LOGFAIL( rc ); goto EXIT; }

	// check signatures to ensure location, clue, topic / pubkey have consistient pairing

	if( rc = ar_core_check_recordset( szLocation, arecord, srecordtbl, numSRecords ) ) { LOGFAIL( rc ); goto EXIT; }

	if( !(shareArr = malloc( sizeof(gfPoint) * numSRecords )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(shareIDArr = malloc( sizeof(word16) * numSRecords )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	
	for( int i=0; i<numSRecords; i++ )
	{
		if( 1 == gfUnpack( shareArr[i], srecordtbl[i]->share ) ) { rc = RC_SHARD; LOGFAIL( rc ); goto EXIT; }
		shareIDArr[i] = srecordtbl[i]->shareid;
	}

	// alloc buffer, copy and decipher

	if( !(*buf_out = malloc( arecord->msglen )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	{	
		byteptr cryptext = arecord->buf + arecord->loclen + arecord->cluelen;
		byteptr bufend = *buf_out + arecord->msglen;

		if( rc = ar_util_memcpy( *buf_out, bufend, cryptext, arecord->msglen ) ) { LOGFAIL( rc ); goto EXIT; }

		gfPoint gfCryptkey;
		ar_shamir_recoversecret( gfCryptkey, shareIDArr, shareArr, numSRecords );

		ar_core_rc4( *buf_out, arecord->msglen, gfCryptkey );

		gfClear( gfCryptkey );
	}

EXIT:

	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numSRecords ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numSRecords ); free( shareIDArr ); }

	if( rc && buf_out && *buf_out ) { free( *buf_out ); *buf_out = 0; }

	return rc;
}

int ar_core_check_recordset( byteptr szLocation, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords )
{
	int rc = 0;

	if( !szLocation ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !arecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !srecordtbl ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( rc = ar_util_checkTbl( (void**)srecordtbl, numSRecords ) ) { LOGFAIL( rc ); goto EXIT; }

	for( int i=0; i<numSRecords; i++ )
	{
		if( !vlEqual( arecord->pubkey, srecordtbl[i]->pubkey ) ) { rc = RC_TOPIC; LOGFAIL( rc ); goto EXIT; }
	}

	if( rc = ar_core_check_arecord( szLocation, arecord ) ) { LOGFAIL( rc ); goto EXIT; }

	for( int i=0; i<numSRecords; i++ )
	{
		if( rc = ar_core_check_srecord( szLocation, srecordtbl[i] ) ) { LOGFAIL( rc ); goto EXIT; }
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

	size_t buflen = strlen(reftextin) +1; // +1 for \0
	if( !(cleartextin = malloc( buflen )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	if( rc = ar_util_strcpy( cleartextin, cleartextin + buflen, reftextin ) ) { LOGFAIL( rc ); goto EXIT; }

	rc = ar_core_create( &arecord, &srecordtbl, 2, 2, cleartextin, (word16)(buflen), (bytetbl)cluetbl, AR_LOCSTR );
	TESTASSERT( rc == 0 );

	rc = ar_core_check_recordset( AR_LOCSTR, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );

	rc = ar_core_decrypt( &cleartext_out, AR_LOCSTR, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );

	TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );

#if defined(ENABLE_FUZZING)

	///////////////////
	// now start breaking things....

	srecordtbl[1]->pubkey[0] = 0; // break topic of share 2

	rc = ar_core_check_recordset( AR_LOCSTR, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );

	arecord->pubkey[0] = 0; // break auth record topic

	rc = ar_core_check_recordset( AR_LOCSTR, arecord, srecordtbl, 2 );
	TESTASSERT( rc != 0 );

#endif // ENABLE_FUZZING

EXIT:

	if( cleartextin ) free( cleartextin );
	if( cleartext_out ) free( cleartext_out );
	if( arecord ) free( arecord );
	for( size_t i = 0; i < 2; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
	if( srecordtbl ) free( srecordtbl );

#endif
}
