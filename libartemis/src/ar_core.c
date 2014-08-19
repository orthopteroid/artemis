// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_core.h"
#include "ec_vlong.h"
#include "ar_shamir.h"
#include "ar_util.h"
#include "rc4.h"

#include "version.h" // for version related defines

////////////////////////////

// Artemis A uri is artemis:<topic>&vf=<verify>&si=<splitinfo>&pk=<pubkey>&as=<authsig>&mt=<messagedata>
// Artemis S uri is artemis:<topic>&sh=<share>&ss=<sharesig>

// artemis
//  1 numshares, threshold and fieldsize are combined into splitinfo I
//  2 plaintext message M is hashed to produce 32bit verification V
//  3 random cipher key C is used on message M to crypt message (producing M')
//  4 split C into shares {C'1...C'n}
//  5 crypted message M' is hashed to produce topic T
//  6 random private key Kpri is used to create a matching public key Kpub
//  7 private key Kpri is applied to hash of T | V | I | Kpub to produce authsignature Sa
//  8 private key Kpri is applied to hash of T | { 1, ... i, ... n } | { C'1, ... C'i, ... C'n } to produce sharesignatures {S1...Sn}
//  9 private key Kpri and cipher key C are thrown away
// 10 authentication is distributed in the concatenated form T | V | I | Kpub | Sa | M'
// 11 shares are distributed in the concatenated form { T | i | C'i | Si }

#define AR_RC4CRANKS 337

int ar_core_create( arAuth* pARecord, arShare* pSRecordArr, word16 numShares, word16 numThres, byteptr inbuf, word16 inbuflen )
{
	gfPoint* gfCryptCoefArr = 0;
	gfPoint* shareArr = 0;
	word16* shareIDArr = 0;
	int rc = 0;

	if( inbuflen == 0 ) { rc = -1; goto EXIT; }
	if( numShares < numThres ) { rc = -1; goto EXIT; }
	if( pARecord->bufmax < inbuflen ) { ASSERT(0); rc = -2; goto EXIT; }

	if( !(shareArr = malloc( sizeof(gfPoint) * numShares )) ) { ASSERT(0); rc=-9; goto EXIT; }
	if( !(shareIDArr = malloc( sizeof(word16) * numShares )) ) { ASSERT(0); rc=-9; goto EXIT; }
	if( !(gfCryptCoefArr = malloc( numThres * sizeof(gfPoint) )) ) { ASSERT(0); rc=-9; goto EXIT; }

	///////////
	// demo mode

#if defined( AR_DEMO )
	if( inbuflen > 1 )
	{
		size_t a, b;
		word16 numswaps = max( inbuflen>>4, 1 );
		for( word16 i=0; i<numswaps; i++ )
		{
			a = platform_rnd32() % (inbuflen-1);
			b = platform_rnd32() % (inbuflen-1);
			if( a != b )
			{
				byte tmp = inbuf[ a ];
				inbuf[ a ] = inbuf[ b ];
				inbuf[ b ] = tmp;
			}
		}
	}
#endif

	///////////
	// create verification hash of cleartext

	vlPoint verify;
	vlClear( verify );
	{
		sha1Digest digest;
		sha1_context c[1];
		sha1_initial( c );

		sha1_process( c, inbuf, (unsigned)(inbuflen) );
		sha1_final( c, digest );
		vlSetWord32( verify, digest[0] ); // verify is 32bit hash
	}

	///////////
	// create cryptcoefs (and cryptkey), cipher the cleartext and split the shares

	pARecord->bufused = ( inbuflen > pARecord->bufmax ) ? pARecord->bufmax : inbuflen;
	memcpy( pARecord->buf, inbuf, pARecord->bufused );

	for( word16 t = 0; t < numThres; t++ )
	{
		vlPoint vlTmp;
		vlClear( vlTmp );
#if AR_KEYLENGTH == 32
		vlSetWord32( vlTmp, platform_rnd32() );
#elif AR_KEYLENGTH == 64
		vlSetWord64( vlTmp, platform_rnd32(), platform_rnd32() );
#elif AR_KEYLENGTH == 128
		vlSetWord128( vlTmp, platform_rnd32(), platform_rnd32(), platform_rnd32(), platform_rnd32() );
#endif // AR_KEYLENGTH
		gfUnpack( gfCryptCoefArr[t], vlTmp );
		gfReduce( gfCryptCoefArr[t] );
	}

	{
		vlPoint vlCryptkey;
		gfPack( gfCryptCoefArr[0], vlCryptkey );

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_16BAto8BA( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, AR_RC4CRANKS, pARecord->buf, inbuflen );

		vlClear( vlCryptkey );
	}

	ar_shamir_splitsecret( shareArr, shareIDArr, numShares, gfCryptCoefArr, numThres );

	// cleanup secret coefs
	memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres );

	/////////
	// construct topic from cryptext

	vlPoint topic;
	vlClear( topic );
	{
		sha1Digest digest;
		sha1_context c[1];
		sha1_initial( c );

		sha1_process( c, pARecord->buf, (unsigned)(pARecord->bufused) );
		sha1_final( c, digest );
		vlSetWord64( topic, digest[0], digest[1] ); // topic is 64bit hash
	}

	/////////
	// construct keypair

	vlPoint priSigningkey;
	vlClear( priSigningkey );
#if AR_KEYLENGTH == 32
	vlSetWord32( priSigningkey, platform_rnd32() );
#elif AR_KEYLENGTH == 64
	vlSetWord64( priSigningkey, platform_rnd32(), platform_rnd32() );
#elif AR_KEYLENGTH == 128
	vlSetWord128( priSigningkey, platform_rnd32(), platform_rnd32(), platform_rnd32(), platform_rnd32() );
#endif // AR_KEYLENGTH

	vlPoint pubSigningkey;
	vlClear( pubSigningkey );
	cpMakePublicKey( pubSigningkey, priSigningkey );

	//////////
	// sign the auth record

	pARecord->shares = numShares;
	pARecord->threshold = numThres;
	pARecord->fieldsize = GF_M;

	cpPair authsig;
	cpClear( &authsig );
	{
		vlPoint authhash;
		vlClear( authhash );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );
			{
				size_t deltalen = 0;
				char composebuf[ sizeof(vlPoint) ];
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), topic+1, topic[0] );
				sha1_process( c, composebuf, (unsigned)(topic[0] * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), verify+1, verify[0] );
				sha1_process( c, composebuf, (unsigned)(verify[0] * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->shares, (unsigned)(1 * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->threshold, (unsigned)(1 * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->fieldsize, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pubSigningkey+1, pubSigningkey[0] );
				sha1_process( c, composebuf, (unsigned)(pubSigningkey[0] * sizeof(word16)) );
				memset( composebuf, 0, sizeof(composebuf) );
			}
			sha1_final( c, digest );
			vlSetWord64( authhash, digest[0], digest[1] ); // signed with 64bit hash
		}
		rc = ar_shamir_sign( &authsig, priSigningkey, authhash );
		if( rc != 0 ) { ASSERT(0); rc = -1; goto EXIT; }
	}

	cpCopy( &pARecord->authsig, &authsig );
	vlCopy( pARecord->pubkey, pubSigningkey );
	vlCopy( pARecord->topic, topic );
	vlCopy( pARecord->verify, verify );

	/////////////
	// sign the produced share records

	for( int i=0; i<numShares; i++ )
	{
		cpPair sharesig;
		vlPoint vlShare;
		cpClear( &sharesig );
		vlClear( vlShare );
		{
			gfPack( shareArr[i], vlShare );

			//////////
			// construct sharesignature to ensure consistiency between topic, shareid and share

			vlPoint saltedsharehash;
			vlClear( saltedsharehash );
			{
				sha1Digest digest;
				sha1_context c[1];
				sha1_initial( c );
				{
					size_t deltalen = 0;
					char composebuf[ sizeof(vlPoint) ];
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), topic+1, topic[0] );
					sha1_process( c, composebuf, (unsigned)(topic[0] * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &shareIDArr[i], 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), vlShare+1, vlShare[0] );
					sha1_process( c, composebuf, (unsigned)(vlShare[0] * sizeof(word16)) );
					memset( composebuf, 0, sizeof(composebuf) );
				}
				sha1_final( c, digest );
				vlSetWord64( saltedsharehash, digest[0], digest[1] ); // signed with 64bit hash
			}
			rc = ar_shamir_sign( &sharesig, priSigningkey, saltedsharehash );
			if( rc != 0 ) { ASSERT(0); rc = -1; goto EXIT; }
		}

		vlClear( pSRecordArr[i].share );
		cpClear( &pSRecordArr[i].sharesig );

		vlCopy( pSRecordArr[i].share, vlShare );
		pSRecordArr[i].shareid = shareIDArr[i];
		cpCopy( &pSRecordArr[i].sharesig, &sharesig );
		vlCopy( pSRecordArr[i].topic, topic );
	}

EXIT:

	if( gfCryptCoefArr ) { memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres ); free( gfCryptCoefArr ); }
	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numShares ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numShares ); free( shareIDArr ); }

	return rc;
}

int ar_core_decrypt( byteptr outbuf, word16 outbuflen, arAuth* pARecord, arShare* pSRecordArr, word16 numSRecords )
{
	gfPoint* shareArr = 0;
	word16* shareIDArr = 0;
	int rc = 0;

	if( outbuflen == 0 ) { ASSERT(0); rc = -1; goto EXIT; }
	if( outbuflen < pARecord->bufused ) { ASSERT(0); rc = -2; goto EXIT; }
	if( numSRecords < pARecord->threshold ) { ASSERT(0); rc = -7; goto EXIT; }

	///////////
	// check topic consistiency between ARecord, cryptext and all SRecords

	vlPoint topic;
	vlClear( topic );
	{
		sha1Digest digest;
		sha1_digest( digest, pARecord->buf, pARecord->bufused );
		vlSetWord64( topic, digest[0], digest[1] );
	}
	if( !vlEqual( pARecord->topic, topic ) ) { ASSERT(0); rc = -6; goto EXIT; }

	for( int i=0; i<numSRecords; i++ )
	{
		if( !vlEqual( topic, pSRecordArr[i]->topic ) ) { ASSERT(0); rc = -6; goto EXIT; }
	}

	////////////
	// check authsignature to ensure topic and pubkey have consistient pairing

	{
		vlPoint authhash;
		vlClear( authhash );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );
			{
				size_t deltalen = 0;
				char composebuf[ sizeof(vlPoint) ];
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), topic+1, topic[0] );
				sha1_process( c, composebuf, (unsigned)(topic[0] * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pARecord->verify+1, pARecord->verify[0] );
				sha1_process( c, composebuf, (unsigned)(pARecord->verify[0] * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->shares, (unsigned)(1 * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->threshold, (unsigned)(1 * sizeof(word16)) );
				sha1_process( c, (byteptr)&pARecord->fieldsize, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pARecord->pubkey+1, pARecord->pubkey[0] );
				sha1_process( c, composebuf, (unsigned)(pARecord->pubkey[0] * sizeof(word16)) );
				memset( composebuf, 0, sizeof(composebuf) );
			}
			sha1_final( c, digest );
			vlSetWord64( authhash, digest[0], digest[1] );
		}
		if( !cpVerify( pARecord->pubkey, authhash, &pARecord->authsig ) ) { ASSERT(0); rc = -4; goto EXIT; }
	}

	////////////
	// check sharesignatures to ensure topic, shareid and share have consistient grouping

	for( int i=0; i<numSRecords; i++ )
	{
		vlPoint saltedsharehash;
		vlClear( saltedsharehash );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );
			{
				size_t deltalen = 0;
				char composebuf[ sizeof(vlPoint) ];
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), topic+1, topic[0] );
				sha1_process( c, composebuf, (unsigned)(topic[0] * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &pSRecordArr[i].shareid, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pSRecordArr[i].share+1, pSRecordArr[i].share[0] );
				sha1_process( c, composebuf, (unsigned)(pSRecordArr[i].share[0] * sizeof(word16)) );
				memset( composebuf, 0, sizeof(composebuf) );
			}
			sha1_final( c, digest );
			vlSetWord64( saltedsharehash, digest[0], digest[1] );
		}
		if( !cpVerify( pARecord->pubkey, saltedsharehash, &pSRecordArr[i]->sharesig ) ) { ASSERT(0); rc = -3; goto EXIT; }
	}

	//////////////

	if( (shareArr = malloc( sizeof(gfPoint) * numSRecords )) == 0 ) { ASSERT(0); rc=-9; goto EXIT; }
	if( (shareIDArr = malloc( sizeof(word16) * numSRecords )) == 0 ) { ASSERT(0); rc=-9; goto EXIT; }

	for( int i=0; i<numSRecords; i++ )
	{
		gfUnpack( shareArr[i], pSRecordArr[i].share );
		shareIDArr[i] = pSRecordArr[i].shareid;
	}

	memcpy( outbuf, pARecord->buf, pARecord->bufused );

	{
		gfPoint gfCryptkey;
		gfClear( gfCryptkey );
		ar_shamir_recoversecret( gfCryptkey, shareIDArr, shareArr, numSRecords );

		vlPoint vlCryptkey;
		gfPack( gfCryptkey, vlCryptkey );

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_16BAto8BA( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, AR_RC4CRANKS, outbuf, pARecord->bufused );
	}

	///////////
	// check if shares decoded properly

	{
		vlPoint verify;
		vlClear( verify );
		{
			sha1Digest digest;
			sha1_context c[1];
			sha1_initial( c );
			sha1_process( c, outbuf, (unsigned)(pARecord->bufused) );
			sha1_final( c, digest );
			vlSetWord32( verify, digest[0] );
		}
		if( !vlEqual( verify, pARecord->verify ) ) { ASSERT(0); rc = -5; goto EXIT; }
	}

EXIT:

	if( shareArr ) { memset( shareArr, 0, sizeof(gfPoint) * numSRecords ); free( shareArr ); }
	if( shareIDArr ) { memset( shareIDArr, 0, sizeof(word16) * numSRecords ); free( shareIDArr ); }

	return rc;
}

void ar_core_test()
{

#if defined(_DEBUG)

	printf("# ar_core_test\n");

	arAuth* arecord = 0;
	arShare srecords[2];
	char* reftextin = "dog food.";
	char* cleartextin = 0;
	char cleartextout[80];
	int rc = 0;

	if( !(arecord = malloc( sizeof(arAuth) + 80 )) ) { ASSERT(0); goto EXIT; }
	if( !(cleartextin = malloc( strlen(reftextin)+1 )) ) { ASSERT(0); goto EXIT; }
	strcpy_s( cleartextin, strlen(reftextin)+1, reftextin );

	memset( arecord, 0, sizeof(arAuth) + 80 );
	memset( srecords, 0, sizeof(arShare) * 2 );
	arecord->bufmax = 80;

	rc = ar_core_create( arecord, srecords, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1) ); // +1 to include \0
	ASSERT( rc == 0 );

	rc = ar_core_decrypt( cleartextout, 80, arecord, srecords, 2 );
	ASSERT( rc == 0 );

EXIT:

	if( cleartextin ) free( cleartextin );
	if( arecord ) free( arecord );

#endif
}
