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

#include <stdio.h>

void test_all()
{

#if defined(_DEBUG)

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
	
	library_uri_encoder( &shares, 5, 5, "foo.bar", "main\none\ntwo\nthree\nfour\nfive", "message five" );  printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 4, "foo.bar", "main\none\ntwo\nthree\nfour\nfive", "message four" );  printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 3, "foo.bar", "main\none\ntwo\nthree\nfour\nfive", "message three" ); printf( "%s\n", shares ); library_free( &shares );
	library_uri_encoder( &shares, 5, 2, "foo.bar", "main\none\ntwo\nthree\nfour\nfive", "message two" );   printf( "%s\n", shares ); library_free( &shares );

#else // _DEBUG

	printf("# no tests in this build\n");

#endif // _DEBUG
}
