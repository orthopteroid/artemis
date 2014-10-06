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

////////////////////////////

// Artemis A uri is http://<location>?tp=<topic>&ai=<splitinfo>&vf=<verify>&pk=<pubkey>&as=<authsig>&mt=<messagetext>&mc=<messageclue>
// where:
// <splitinfo> == <shares|<threshold>|<fieldsize>
//
// Artemis S uri is http://<location>?tp=<topic>&si=<shareinfo>&sh=<share>&ss=<sharesig>&sc=<shareclue>
// where:
// <shareinfo> == <shares|<threshold>|<shareid>
//

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

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, word16 numThres, byteptr inbuf, word16 inbuflen, bytetbl clueTbl, byteptr location )
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

	if( numShares < 2 ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }
	if( numShares < numThres ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }

	///////////
	// alloc tmp storage

	if( !(shareArr = malloc( sizeof(gfPoint) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(shareIDArr = malloc( sizeof(word16) * numShares )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	if( !(gfCryptCoefArr = malloc( numThres * sizeof(gfPoint) )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }

	///////////
	// general vars

	size_t loclen = location ? strlen( location ) : 0;
	if( loclen == 0 ) { rc = RC_LOCATION; LOGFAIL( rc ); goto EXIT; }

	size_t acluelen = ( clueTbl && clueTbl[0] ) ? strlen( clueTbl[0] ) : 0;
	size_t msgoffset = loclen + acluelen; // msg comes after clue + location

	///////////
	// alloc return values

	size_t abufused = loclen + inbuflen + acluelen;
	size_t astructsize = sizeof(arAuth) + abufused;
	if( !(arecord_out[0] = malloc( astructsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( arecord_out[0], 0, astructsize );
	arecord_out[0]->bufmax = abufused;

	size_t stblsize = sizeof(arShareptr) * numShares;
	if( !((*srecordtbl_out) = malloc( stblsize ) )) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( (*arecord_out), 0, stblsize );

	for( int i=0; i<numShares; i++ )
	{
		size_t scluelen = ( clueTbl && clueTbl[i+1] ) ? strlen( clueTbl[i+1] ) : 0;
		size_t sbufused = loclen + scluelen;
		size_t sstructsize = sizeof(arShare) + sbufused;

		if( !((*srecordtbl_out)[i] = malloc( sstructsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
		memset( (*srecordtbl_out)[i], 0, sstructsize );
		(*srecordtbl_out)[i]->bufmax = sbufused;
	}

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
		vlClear( vlTmp );
#if AR_CRYPTKEYLENGTH == 32
		vlSetWord32( vlTmp, ar_util_rnd32() );
#elif AR_CRYPTKEYLENGTH == 64
		vlSetWord64( vlTmp, ar_util_rnd32(), ar_util_rnd32() );
#elif AR_CRYPTKEYLENGTH == 128
		vlSetWord128( vlTmp, ar_util_rnd32(), ar_util_rnd32(), ar_util_rnd32(), ar_util_rnd32() );
#endif // AR_CRYPTKEYLENGTH
		gfUnpack( gfCryptCoefArr[t], vlTmp );
		gfReduce( gfCryptCoefArr[t] );
	}

	{
		vlPoint vlCryptkey;
		gfPack( gfCryptCoefArr[0], vlCryptkey );

		word32 rc4cranks = AR_RC4CRANK_OFFSET + (AR_RC4CRANK_MASK & vlGetWord16( vlCryptkey, 0 ));

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_16BAto8BA( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, rc4cranks, arecord_out[0]->buf + msgoffset, inbuflen );

		vlClear( vlCryptkey );
	}

	ar_shamir_splitsecret( shareArr, shareIDArr, numShares, gfCryptCoefArr, numThres );

	// cleanup secret coefs
	memset( gfCryptCoefArr, 0, sizeof(gfPoint) * numThres );

	/////////
	// construct topic from location, cryptext and mclue

	vlPoint topic;
	vlClear( topic );
	{
		sha1Digest digest;
		sha1_context c[1];
		sha1_initial( c );

		sha1_process( c, arecord_out[0]->buf, (unsigned)(abufused) );
		sha1_final( c, digest );
		vlSetWord64( topic, digest[0], digest[1] ); // topic is 64bit hash
	}

	/////////
	// construct keypair

	vlPoint priSigningkey;
	vlClear( priSigningkey );
#if AR_SIGNKEYLENGTH == 32
	vlSetWord32( priSigningkey, ar_util_rnd32() );
#elif AR_SIGNKEYLENGTH == 64
	vlSetWord64( priSigningkey, ar_util_rnd32(), ar_util_rnd32() );
#elif AR_SIGNKEYLENGTH == 128
	vlSetWord128( priSigningkey, ar_util_rnd32(), ar_util_rnd32(), ar_util_rnd32(), ar_util_rnd32() );
#endif // AR_SIGNKEYLENGTH

	vlPoint pubSigningkey;
	vlClear( pubSigningkey );
	cpMakePublicKey( pubSigningkey, priSigningkey );

	//////////
	// sign the auth record

	arecord_out[0]->shares = numShares;
	arecord_out[0]->threshold = numThres;
	arecord_out[0]->fieldsize = GF_M;

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
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord_out[0]->shares, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord_out[0]->threshold, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord_out[0]->fieldsize, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pubSigningkey+1, pubSigningkey[0] );
				sha1_process( c, composebuf, (unsigned)(pubSigningkey[0] * sizeof(word16)) );
				sha1_process( c, arecord_out[0]->buf, (unsigned)(abufused) );
				memset( composebuf, 0, sizeof(composebuf) );
			}
			sha1_final( c, digest );
			vlSetWord64( authhash, digest[0], digest[1] ); // signed with 64bit hash
		}
		if( rc = ar_shamir_sign( &authsig, priSigningkey, authhash ) ) { LOGFAIL( rc ); goto EXIT; }
	}

	cpCopy( &arecord_out[0]->authsig, &authsig );
	vlCopy( arecord_out[0]->pubkey, pubSigningkey );
	vlCopy( arecord_out[0]->topic, topic );
	vlCopy( arecord_out[0]->verify, verify );

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
			cpClear( &sharesig );

			vlPoint saltedsharehash;
			vlClear( saltedsharehash );
			{
				sha1Digest digest;
				sha1_context c[1];
				sha1_initial( c );
				{
					size_t deltalen = 0;
					char composebuf[ sizeof(vlPoint) ];
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), (*srecordtbl_out)[i]->topic+1, (*srecordtbl_out)[i]->topic[0] );
					sha1_process( c, composebuf, (unsigned)((*srecordtbl_out)[i]->topic[0] * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &(*srecordtbl_out)[i]->shares, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &(*srecordtbl_out)[i]->threshold, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &(*srecordtbl_out)[i]->shareid, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), (*srecordtbl_out)[i]->share+1, (*srecordtbl_out)[i]->share[0] );
					sha1_process( c, composebuf, (unsigned)((*srecordtbl_out)[i]->share[0] * sizeof(word16)) );
					sha1_process( c, (*srecordtbl_out)[i]->buf, (unsigned)(sbufused) );
					memset( composebuf, 0, sizeof(composebuf) );
				}
				sha1_final( c, digest );
				vlSetWord64( saltedsharehash, digest[0], digest[1] ); // signed with 64bit hash
			}
			if( rc = ar_shamir_sign( &sharesig, priSigningkey, saltedsharehash ) ) { LOGFAIL( rc ); goto EXIT; }
		}

		cpClear( &(*srecordtbl_out)[i]->sharesig );
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
	
	if( numSRecords < arecord->threshold ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); goto EXIT; }
	
	if( !srecordtbl ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

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
		gfUnpack( shareArr[i], srecordtbl[i]->share );
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
		gfClear( gfCryptkey );
		ar_shamir_recoversecret( gfCryptkey, shareIDArr, shareArr, numSRecords );

		vlPoint vlCryptkey;
		gfPack( gfCryptkey, vlCryptkey );

		word32 rc4cranks = AR_RC4CRANK_OFFSET + (AR_RC4CRANK_MASK & vlGetWord16( vlCryptkey, 0 ));

		size_t deltalen = 0;
		byte cryptkeyBArr[ 16 ] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 128 bits = 16 bytes
		ar_util_16BAto8BA( &deltalen, cryptkeyBArr, 16, vlCryptkey + 1, vlCryptkey[0] );
		rc4( cryptkeyBArr, 16, rc4cranks, *buf_out, arecord->msglen );
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
			sha1_process( c, *buf_out, (unsigned)(arecord->msglen) );
			sha1_final( c, digest );
			vlSetWord32( verify, digest[0] );
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
	vlClear( topic );
	{
		sha1Digest digest;
		sha1_digest( digest, arecord->buf, arecord->msglen + arecord->loclen + arecord->cluelen );
		vlSetWord64( topic, digest[0], digest[1] );
	}
	if( !vlEqual( arecord->topic, topic ) ) { rc = RC_TOPIC; LOGFAIL( rc ); goto EXIT; }

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
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), arecord->topic+1, arecord->topic[0] );
				sha1_process( c, composebuf, (unsigned)(arecord->topic[0] * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), arecord->verify+1, arecord->verify[0] );
				sha1_process( c, composebuf, (unsigned)(arecord->verify[0] * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord->shares, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord->threshold, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &arecord->fieldsize, 1 );
				sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
				ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), arecord->pubkey+1, arecord->pubkey[0] );
				sha1_process( c, composebuf, (unsigned)(arecord->pubkey[0] * sizeof(word16)) );
				sha1_process( c, arecord->buf, (unsigned)(arecord->msglen + arecord->loclen + arecord->cluelen) );
				memset( composebuf, 0, sizeof(composebuf) );
			}
			sha1_final( c, digest );
			vlSetWord64( authhash, digest[0], digest[1] );
		}
		if( !cpVerify( arecord->pubkey, authhash, &arecord->authsig ) ) { rc = RC_SIGNATURE; LOGFAIL( rc ); goto EXIT; }
	}

	// check sharesignatures to ensure data integreity

	if( srecordtbl_opt && numSRecords > 0 )
	{
		for( int i=0; i<numSRecords; i++ )
		{
			arShareptr pSRecord = srecordtbl_opt[i];
			size_t bufused = pSRecord->loclen + pSRecord->cluelen;
			//
			vlPoint saltedsharehash;
			vlClear( saltedsharehash );
			{
				sha1Digest digest;
				sha1_context c[1];
				sha1_initial( c );
				{
					size_t deltalen = 0;
					char composebuf[ sizeof(vlPoint) ];
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pSRecord->topic+1, pSRecord->topic[0] );
					sha1_process( c, composebuf, (unsigned)(pSRecord->topic[0] * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &pSRecord->shares, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &pSRecord->threshold, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), &pSRecord->shareid, 1 );
					sha1_process( c, composebuf, (unsigned)(1 * sizeof(word16)) );
					ar_util_16BAto8BA( &deltalen, composebuf, sizeof(composebuf), pSRecord->share+1, pSRecord->share[0] );
					sha1_process( c, composebuf, (unsigned)(pSRecord->share[0] * sizeof(word16)) );
					sha1_process( c, pSRecord->buf, (unsigned)(bufused) );
					memset( composebuf, 0, sizeof(composebuf) );
				}
				sha1_final( c, digest );
				vlSetWord64( saltedsharehash, digest[0], digest[1] );
			}
			int fail = !cpVerify( arecord->pubkey, saltedsharehash, &pSRecord->sharesig );

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

#if defined(_DEBUG)

	printf("# ar_core_test\n");

	arAuthptr arecord = 0;
	arSharetbl srecordtbl = 0;
	char* reftextin = "dog food.";
	byteptr cleartextin = 0;
	byteptr cleartext_out = 0;
	char* cluetbl[3] = {"topiclue", "clue1", "clue2"};
	int rc = 0;

	byte checkarr[2] = {0,0};

	if( !(cleartextin = malloc( strlen(reftextin)+1 )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	strcpy_s( cleartextin, strlen(reftextin)+1, reftextin );

	rc = ar_core_create( &arecord, &srecordtbl, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1), (bytetbl)cluetbl, "foo.bar" ); // +1 to include \0
	TESTASSERT( rc == 0 );

	rc = ar_core_check_topic( checkarr, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );
	TESTASSERT( checkarr[0] == 0 );
	TESTASSERT( checkarr[1] == 0 );

	rc = ar_core_decrypt( &cleartext_out, arecord, srecordtbl, 2 );
	TESTASSERT( rc == 0 );

	TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );

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

EXIT:

	if( cleartextin ) free( cleartextin );
	if( cleartext_out ) free( cleartext_out );
	if( arecord ) free( arecord );
	for( size_t i = 0; i < 2; i++ ) { if( srecordtbl[i] ) free( srecordtbl[i] ); }
	if( srecordtbl ) free( srecordtbl );

#endif
}
