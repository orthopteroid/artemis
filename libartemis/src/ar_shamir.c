// Implementation, Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_util.h"
#include "ar_shamir.h"

#include "ec_crypt.h"	// for testing
#include "ar_util.h"	// for testing

static void eval_horner( gfPoint* solnArr, word16 solnN, gfPoint* coefArr, word16 coefM )
	/* Evaluate polynomial with coefs c { 0 ... M-1 } at x { 1 ... N } (Horner's Method) */
{
	gfPoint g, x, tmp;
    for( word16 s = 0; s != solnN; s++ )
	{
		gfSetLUnit( x, s + 1 ); // x = 1 ... N
		gfCopy( g, coefArr[ coefM - 1 ] );
	    for( word16 c = 1; c != coefM; c++ )
		{
			gfMultiply( tmp, g, x );
			gfAdd( g, tmp, coefArr[ coefM - c - 1 ] );
		}
        gfCopy( solnArr[ s ], g );
    }
};

static void eval_lagrange( gfPoint y, gfPoint x, gfPoint* xArr, gfPoint* yArr, word16 K )
    /* Interpolate polynomial at x (Lagrange's Method) */
{
	// for Shamir... should ensure 0 excluded from YArr and x excluded from XArr
    gfPoint l, tmp, ximxj, ximxjInv, prod;
	gfClear( y );
    for( int i = 0; i < K; ++i )
	{
        //calculate the constant term of lagrange interpolation polynomial
        gfSetLUnit( l, 1 );
        for( int j = 0; j < K; ++j )
		{
            if( i == j ) continue;
			gfAdd( ximxj, xArr[ i ], xArr[ j ] );			// same as '-' in GF(2^n)
			if( 1 == gfInvert( ximxjInv, ximxj ) ) { LOGFAIL( RC_INTERNAL ); return; }	// same as '/' in GF(2^n)
			gfMultiply( prod, xArr[ j ], ximxjInv );
			gfCopy( tmp, l ); gfMultiply( l, tmp, prod );	// l *= ...
        }
		gfMultiply( prod, yArr[ i ], l );
		gfCopy( tmp, y ); gfAdd( y, tmp, prod );			// y += ...
    }
}

///////////////////////

void ar_shamir_splitsecret( gfPoint* shareArr, word16* shareIDArr, word16 numShares, gfPoint* gfCryptCoefArr, word16 numThres )
	/* Create numbered shares from a key */
{
	word16 r = 0;

	eval_horner( shareArr, numShares, gfCryptCoefArr, numThres );

	for( int s = 0; s < numShares; s++ )
	{
		shareIDArr[s] = s + 1; // numbered from 1...
    }
}

void ar_shamir_recoversecret( gfPoint key, word16* shareIDArr, gfPoint* shareArr, word16 numShares )
	/* Calc key from numbered shares */
{
	key[0] = 0;

	for( int i=0; i< numShares; i++ )
	{
		if( !gfIsValid( shareArr[i] ) ) { LOGFAIL( RC_INTERNAL ); return; }
	}

	gfPoint x;
	gfPoint* gfShareIDArr = 0;

	if( !(gfShareIDArr = malloc( numShares * sizeof(gfPoint) )) ) { int rc = RC_MALLOC; LOGFAIL( rc ); return; }

	for( int i=0; i< numShares; i++ )
	{
		gfSetLUnit( gfShareIDArr[i], (lunit)shareIDArr[i] );
	}

	gfSetLUnit( x, 0 ); // calc y at x = 0
	eval_lagrange( key, x, gfShareIDArr, shareArr, numShares );

	if( gfShareIDArr ) free( gfShareIDArr );
}

int ar_shamir_sign( cpPair* sig, const vlPoint session, const vlPoint vlPrivateKey, const vlPoint mac )
{
	int rc = 0;

	if( !vlIsValid( vlPrivateKey ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( session ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( mac ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( vlIsZero( mac ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }

	cpSign( vlPrivateKey, session, mac, sig );
	if( sig->r[0] == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	if( vlIsZero( sig->r ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }

EXIT:

	return rc;
}

void ar_shamir_test()
{

#if defined(_DEBUG)

	{
		printf("# test shamir roundtrip\n");

		gfPoint shareArr[2];
		word16 shareIDArr[2];
		word16 sharesN = 2;
		word16 thresholdM = 2;
		vlPoint vlTmp;
		gfPoint gfCryptCoef[2];

		for( int i=0; i<50; i++ )
		{
			for( int t=0;t<2;t++ )
			{
				vlSetWord64( vlTmp, ar_util_rnd32(), ar_util_rnd32() );
				gfUnpack( gfCryptCoef[t], vlTmp );
				gfReduce( gfCryptCoef[t] );
			}

			ar_shamir_splitsecret( shareArr, shareIDArr, sharesN, gfCryptCoef, thresholdM );

			gfPoint keyRecovered;
			ar_shamir_recoversecret( keyRecovered, shareIDArr, shareArr, sharesN );

			TESTASSERT( gfEqual( gfCryptCoef[0], keyRecovered ) );
		}
	}

	{
		printf("# test signing\n");

		vlPoint pub, pri, mac;
		cpPair sig;

		vlSetWord64( pri, ar_util_rnd32(), ar_util_rnd32() );

		cpMakePublicKey( pub, pri );

		sha1Digest digest;
		sha1_digest( digest, "themessage", strlen("themessage") );
		vlSetWord64( mac, digest[0], digest[1] );

		int rc = 0;
		for( size_t i = 0; i < 100; i++ )
		{
			vlPoint session;
			vlSetRandom( session, VL_UNITS, &ar_util_rnd16 );
			rc = ar_shamir_sign( &sig, session, pri, mac );
			if( rc == RC_ARG ) { continue; }
			TESTASSERT( rc == 0 );
		}

		TESTASSERT( cpVerify( pub, mac, &sig ) );
	}

#endif

}
