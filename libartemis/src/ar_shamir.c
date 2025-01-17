// Implementation, Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_util.h"
#include "ar_shamir.h"

#include "ec_crypt.h"	// for testing
#include "ar_util.h"	// for testing

#if defined(ENABLE_TESTS)
#include "version.h" // for testing
#endif

///////////

static void eval_horner( gfPoint* solnArr, word16 solnN, gfPoint* coefArr, word16 coefM )
	/* Evaluate polynomial with coefs c { 0 ... M-1 } at x { 1 ... N } (Horner's Method) */
{
	gfPoint g, x, tmp;
    for( word16 s = 0; s != solnN; s++ )
	{
		gfSetUnit( x, s + 1 ); // x = 1 ... N
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
        gfSetUnit( l, 1 );
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
		gfSetUnit( gfShareIDArr[i], (gfunit)shareIDArr[i] );
	}

	gfSetUnit( x, 0 ); // calc y at x = 0
	eval_lagrange( key, x, gfShareIDArr, shareArr, numShares );

	if( gfShareIDArr ) free( gfShareIDArr );
}

void ar_shamir_test()
{
	int equal = 0;

#if defined(ENABLE_TESTS)

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
				vlSetRandom( vlTmp, &ar_util_rnd16, AR_CRYPTKEYBYTES );
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
		printf("# test public-key point-compression\n");

		for( int i=0; i<100; i++ )
		{
			vlPoint pri, vlp0, vlp1;
			vlSetRandom( pri, &ar_util_rnd16, AR_AUTHKEYBYTES );

			// reposition onto curve
			cpMakePublicKey( vlp0, pri );

			// compress
			ecPoint compressedPoint;
			TESTASSERT( 0 == ecUnpack( &compressedPoint, vlp0 ) );
			int ybit = ecYbit( &compressedPoint );
			gfClear( compressedPoint.y );

			// uncompress
			ecPoint ecp1;
			ecClear( &ecp1 );
			gfCopy( ecp1.x, compressedPoint.x );
			TESTASSERT( 0 == ecCalcY( &ecp1, ybit ) );
			ecPack( vlp1, &ecp1 );

			TESTASSERT( vlEqual( vlp0, vlp1 ) );
		}
	}

	{
		printf("# test signing\n");

		vlPoint mac;
		vlSetRandom( mac, &ar_util_rnd16, AR_MACBYTES );

#if defined(ENABLE_FUZZING)

		for( size_t j=0; j < 500; j++ )

#else

		for( size_t j=0; j < 100; j++ )

#endif

		{
			vlPoint pub, pri;
			cpPair sig;
			vlPoint session;

			int i = 0;
			int rc = RC_PRIVATEKEY;
			while( rc == RC_PRIVATEKEY )
			{
				if( ++i == 100 ) { break; } // 100 is big and will create failures in lieu of lockups

				// so, it seems that some pri-keys wont work...
				vlSetRandom( pri, &ar_util_rnd16, AR_AUTHKEYBYTES );

				cpMakePublicKey( pub, pri );

				ecPoint t2;
				if( ecUnpack( &t2, pub ) ) { continue; }

				vlSetRandom( session, &ar_util_rnd16, AR_AUTHKEYBYTES );
				if( rc = cpSign( &sig, pri, session, mac ) ) { continue; }
				if( vlIsZero( sig.r ) ) { continue; }
				equal = 0;
				if( rc = cpVerify( &equal, &sig, pub, mac ) ) { continue; }
				if( !equal ) { continue; }
				rc = 0;
			}
			TESTASSERT( rc == 0 );

			if(1) {
				if( i > 1 ) { DEBUGPRINT( "%d ", i); }
			}

			if(0) {
				size_t len;
				static char buf[1024];
				ar_util_u16_hexencode( &len, buf, buf + 1024, pri+1, pri[0] ); buf[len]=0; DEBUGPRINT( "pri %s\n", buf );
				ar_util_u16_hexencode( &len, buf, buf + 1024, pub+1, pub[0] ); buf[len]=0; DEBUGPRINT( "pub %s\n", buf );
				ar_util_u16_hexencode( &len, buf, buf + 1024, session+1, session[0] ); buf[len]=0; DEBUGPRINT( "session %s\n\n", buf );
			}

			TESTASSERT( 0 == cpVerify( &equal, &sig, pub, mac ) );
			TESTASSERT( equal );

#if defined(ENABLE_FUZZING)

			// can we be sure that once we can sign a mac, we can sign any mac?
			for( int i=0; i<20; i++)
			{
				vlPoint mac;
				vlSetRandom( mac, &ar_util_rnd16, AR_MACBYTES );

				TESTASSERT( 0 == cpSign( &sig, pri, session, mac ) );
				TESTASSERT( 0 == vlIsZero( sig.r ) );
				TESTASSERT( 0 == cpVerify( &equal, &sig, pub, mac ) );
				TESTASSERT( equal );
			}

#endif // ENABLE_FUZZING

		}
	}

#endif

}
