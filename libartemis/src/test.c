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

void test_all()
{

	DEBUGPRINT( "*************\nAR_LOCSTR %s\nAR_LOCHASH 0x%X\n*************\n", AR_LOCSTR, ar_util_strcrc( AR_LOCSTR ) );
	ASSERT( ar_util_strcrc( AR_LOCSTR ) == AR_LOCHASH );

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
	
	library_uri_encoder( &shares, 5, 5, AR_LOCSTR, "main\none\ntwo\nthree\nfour\nfive", "message five" );  printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 4, AR_LOCSTR, "main\none\ntwo\nthree\nfour\nfive", "message four" );  printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 3, AR_LOCSTR, "main\none\ntwo\nthree\nfour\nfive", "message three" ); printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 2, AR_LOCSTR, "main\none\ntwo\nthree\nfour\nfive", "message two" );   printf( "%s\n", shares ); library_free( &shares );

#else

	printf("# no tests in this build\n");

#endif
}
