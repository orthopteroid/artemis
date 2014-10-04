// Implementation, Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <stdlib.h>
#include <string.h>

#include "platform.h"
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
			gfInvert( ximxjInv, ximxj );					// same as '/' in GF(2^n)
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
	gfPoint x;
	gfPoint* gfShareIDArr = 0;

	if( !(gfShareIDArr = malloc( numShares * sizeof(gfPoint) )) ) { LOGFAIL; goto EXIT; }

	for( int i=0; i< numShares; i++ )
	{
		gfSetLUnit( gfShareIDArr[i], (lunit)shareIDArr[i] );
	}

	gfSetLUnit( x, 0 ); // calc y at x = 0
	eval_lagrange( key, x, gfShareIDArr, shareArr, numShares );

EXIT:

	if( gfShareIDArr ) free( gfShareIDArr );
}

int ar_shamir_sign( cpPair* sig, const vlPoint vlPrivateKey, const vlPoint mac )
{
	const int maxLoops = 100;
	int i = 0;
	do {
		vlPoint session;
		vlSetWord64( session, ar_util_rnd32(), ar_util_rnd32() );
		cpSign( vlPrivateKey, session, mac, sig );
	} while ( sig->r[0] == 0 && i++ < maxLoops );
	return (sig->r[0] == 0) ? -1 : 0;
}

void ar_shamir_test()
{

#if defined(_DEBUG)

	#define bufSize 2000
	char buf[ bufSize ];

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

			if( !gfEqual( gfCryptCoef[0], keyRecovered ) )
			{
				*buf = 0; ar_util_8BAto4BZ( buf, bufSize, (byteptr)(shareArr[0]+1), shareArr[0][0] ); printf( "%2X,%s ", shareIDArr[0], buf );
				*buf = 0; ar_util_8BAto4BZ( buf, bufSize, (byteptr)(shareArr[1]+1), shareArr[1][0] ); printf( "%2X,%s ", shareIDArr[1], buf );
				*buf = 0; ar_util_8BAto4BZ( buf, bufSize, (byteptr)(keyRecovered+1), keyRecovered[0] ); printf( "-> %s", buf );
				*buf = 0; ar_util_8BAto4BZ( buf, bufSize, (byteptr)(gfCryptCoef[0]+1), gfCryptCoef[0][0] ); printf( " (should be %s)\n", buf );
				TESTASSERT( 0 );
			}
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

		int rc = ar_shamir_sign( &sig, pri, mac );
		TESTASSERT( rc == 0 );

		if( !cpVerify( pub, mac, &sig ) )
		{
			*buf = 0; ar_util_16BAto4BZ( buf, bufSize, pri+1, pri[0] ); printf( "pri: %s\n", buf );
			*buf = 0; ar_util_16BAto4BZ( buf, bufSize, pub+1, pub[0] ); printf( "pub: %s\n", buf );
			*buf = 0; ar_util_16BAto4BZ( buf, bufSize, sig.r+1, sig.r[0] ); printf( "r: %s\n", buf );
			*buf = 0; ar_util_16BAto4BZ( buf, bufSize, sig.s+1, sig.s[0] ); printf( "s: %s\n", buf );
			TESTASSERT( 0 );
		}
	}

#endif

}
