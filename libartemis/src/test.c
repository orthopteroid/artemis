// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "platform.h"
#include "sha1.h"
#include "ec_crypt.h"
#include "ar_shamir.h"
#include "ar_uricodec.h"
#include "ec_vlong.h"
#include "ec_field.h"
#include "ar_util.h"
#include "ar_core.h"
#include "test.h"
#include "library.h"

#include "version.h"

#include <stdio.h>

static INLINE byte strcrc( byteptr s ) { byte x = 0x41; while( *s ) { x += (x << 1) ^ *(s++); } return x; }

#if defined(_DEBUG) && defined(_WINDOWS)

#include <conio.h>

#endif

void test_all()
{

	DEBUGPRINT( "*************\nAR_LOCSTR %s\nAR_LOCHASH 0x%X\n*************\n", AR_LOCSTR, strcrc( AR_LOCSTR ) );
	ASSERT( strcrc( AR_LOCSTR ) == AR_LOCHASH );

#if defined(ENABLE_TESTS)

	library_settest();

	printf("# tests starting\n");
	sha1_test();
	ar_shamir_test();
	ar_util_test();
	ar_core_test();
	ar_uri_test();
	library_test();
	printf("# tests complete\n");

	byteptr shares;

	struct {
		int locks; char* message;
	} testdata[4] = { {5, "message five"}, {4, "message four"}, {3, "message three"}, {2, "message two"} };

	for( int i = 0; i < 4 ; i++ ) {
		library_uri_encoder( &shares, 5, testdata[i].locks, AR_LOCSTR, "main\none\ntwo\nthree\nfour\nfive", testdata[i].message );
		{
			_putch('"');
			for( byteptr s=shares; *s; s++)
			{
				if( *s == '\n') {
					_putch('"'); _putch(',');
					_putch( *s );
					_putch('"');
				} else {
					_putch( *s );
				}
			}
			_putch('"'); _putch(','); _putch('\n');
		}
		library_free( &shares );
	}

#else

	printf("# no tests in this build\n");

#endif
}
