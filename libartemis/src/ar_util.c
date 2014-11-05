// Copyright 2014 John Howard (orthopteroid@gmail.com)

#if defined(_WINDOWS)
	#define _CRT_RAND_S
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_util.h"

static int testflag = 0;

static char b16charout[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static byte b16charin[]  = {0,1,2,3,4,5,6,7,8,9,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}; // OFFSET 0x30

static char b64charout[] = {
        0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, // A - Z
        0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, // a - z
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, // 0 - 9
        0x2D, 0x5F, // - _ (url friendly)
};

static byte b64charin[]  = {
        0x3E, // -
        0xFF, 0xFF, // . - / (non-symbols)
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, // 0 - 9
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // : - @ (non-symbols)
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, // A - X
        0x18, 0x19, // W - Z
        0xFF, 0xFF, 0xFF, 0xFF, // [ - ^ (non-symbols)
        0x3F, // _
        0xFF, // ` (non-symbol)
        0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, // a - x
        0x32, 0x33, // w - z
}; // start at 0x2D

////////////////////

int ar_util_istest()
{
#if defined(_DEBUG)
	return testflag;
#else // _DEBUG
	return 0;
#endif // _DEBUG
}

void ar_util_settest()
{
#if defined(_DEBUG)
	testflag = 1;
#endif // _DEBUG
}

////////////////////

int ar_util_buildByteTbl( bytetbl* table_out, byteptr arr, size_t len )
{
	int rc = 0;

	if( !table_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	// TODO: one day, fix this to allow tables of only 0, 1 or 2 elements
	if( len < 3 ) { rc = RC_INSUFFICIENT; LOGFAIL( rc ); return rc; }

	*table_out = 0;

	size_t numentries = +1; // +1 first entry
	for( size_t i = 0; i < len; i++ ) { if( arr[i] == '\0' ) { numentries++; } } // interior \0 only, not terminating one

	size_t tablesize = numentries +1; // +1 for table terminator
	if( !(*table_out = malloc( tablesize * sizeof(byteptr) )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( *table_out, 0, tablesize * sizeof(byteptr) );

	// stuff the pointer to each sub-string into a table-slot
	size_t j = 0;
	(*table_out)[j++] = arr;
	for( size_t i = 0; i < len; i++ ) { if( arr[i] == '\0' ) { (*table_out)[j++] = &arr[ i + 1 ]; } }

	if( j != numentries ) { rc = RC_ARG; LOGFAIL( rc ); return rc; }

EXIT:

	if( rc && *table_out ) { free( *table_out ); *table_out = 0; }

	return rc;
}

////////////////////

int ar_util_30Bto6BA( byteptr out, word32 in )
{
	if( in > (1 << 30) - 1) { return RC_ARG; }
	out[0] = b64charout[ ( in >> 24 ) & 63 ];
	out[1] = b64charout[ ( in >> 18 ) & 63 ];
	out[2] = b64charout[ ( in >> 12 ) & 63 ];
	out[3] = b64charout[ ( in >>  6 ) & 63 ];
	out[4] = b64charout[ ( in       ) & 63 ];
	return 0;
}

int ar_util_6BAto30B( word32ptr out, byteptr in )
{
	*out = 0;
	*out |= (word16)( b64charin[ in[0] - 0x2D ] & 63 ) << 24;
	*out |= (word16)( b64charin[ in[1] - 0x2D ] & 63 ) << 18;
	*out |= (word16)( b64charin[ in[2] - 0x2D ] & 63 ) << 12;
	*out |= (word16)( b64charin[ in[3] - 0x2D ] & 63 ) << 6;
	*out |= (word16)( b64charin[ in[4] - 0x2D ] & 63 );
	return 0;
}

////////////////////

int ar_util_8BAto4BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize )
{
	int rc=0;
	size_t i=0, j=0;
	while( i != insize )
	{
		if( j + 1 >= bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		char c = in[i];
		buf[j++] = b16charout[ ( c >> 4 ) & 15 ];
		buf[j++] = b16charout[ ( c ) & 15 ];
		i++;
	}
	*deltalen = j;
	return rc;
}

int ar_util_4BAto8BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize )
{
	int rc=0;
	size_t i=0, j=0;
	while( i != insize )
	{
		if( j >= bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		char c = 0;
		c |= ( b16charin[ in[i++] - 0x30 ] & 15 ) << 4;
		c |= ( b16charin[ in[i++] - 0x30 ] & 15 );
		buf[j++] = c;
	}
	*deltalen = j;
	return rc;
}

////////////////////

int ar_util_8BAto6BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize )
{
	int p=0, rc=0;
	size_t j=0, i=0;
	while( i != insize )
	{
		word32 char3 = 0;
		char3 |= (word32)( in[i++] ) << 16;	if( i == insize ) { p=2; goto PAD; }
		char3 |= (word32)( in[i++] ) <<  8;	if( i == insize ) { p=1; goto PAD; }
		char3 |= (word32)( in[i++] );
PAD:
		if( j + 4 - p >= bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; } // ? for buf[0,bufsize)
		buf[j++] = b64charout[ ( char3 >> 18 ) & 63 ];
		buf[j++] = b64charout[ ( char3 >> 12 ) & 63 ];
		if( p < 2 ) { buf[j++] = b64charout[ ( char3 >>  6 ) & 63 ]; }
		if( p < 1 ) { buf[j++] = b64charout[ ( char3 ) & 63 ]; }
	}
	*deltalen = j;
	return rc;
}

int ar_util_6BAto8BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize )
{
	int p=0, rc=0;
	size_t j=0, i=0;
	while( i != insize )
	{
		word32 char3 = 0;
		char3 |= (word32)( b64charin[ in[i++] - 0x2D ] & 63 ) << 18; if( i == insize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		char3 |= (word32)( b64charin[ in[i++] - 0x2D ] & 63 ) << 12; if( i == insize ) { p=2; goto PAD; }
		char3 |= (word32)( b64charin[ in[i++] - 0x2D ] & 63 ) <<  6; if( i == insize ) { p=1; goto PAD; }
		char3 |= (word32)( b64charin[ in[i++] - 0x2D ] & 63 );
PAD:
		if( j + 3 - p > bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; } // '> only' for buf[0,bufsize)
		buf[j++] = (byte)( ( char3 >> 16 ) & 255 );
		if( p < 2 ) { buf[j++] = (byte)( ( char3 >>  8 ) & 255 ); }
		if( p < 1 ) { buf[j++] = (byte)( ( char3       ) & 255 ); }
	}
	*deltalen = j;
	return rc;
}

///////////

int ar_util_16BAto8BA( size_t* deltalen, byteptr buf, size_t bufsize, word16ptr in, size_t insize )
{
	int rc=0;
	size_t i=0, j=0;
	while( i != insize )
	{
		if( j == bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		// note: reading input memory backwards so we output high words to low memy
		buf[ j++ ] = ( in[insize-i-1] >> 8 ) & 0xFF;
		buf[ j++ ] = ( in[insize-i-1] ) & 0xFF;
		i++;
	}
	*deltalen = j;
	return rc;
}

int ar_util_8BAto16BA( size_t* deltalen, word16ptr buf, size_t bufsize, byteptr   in, size_t insize )
{
	int rc=0;
	size_t i=0, j=0;
	while( i != insize )
	{
		if( j == bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		word16 w = 0;
		// note: reading input memory backwards so we output low words to low memy
		w |= (word16)( in[insize-i-1] );		i++;	if( i == insize ) { goto EOS; }
		w |= (word16)( in[insize-i-1] ) <<  8;	i++;	if( i == insize ) { goto EOS; }
EOS:
		buf[ j++ ] = w;
	}
	*deltalen = j;
	return rc;
}

//////////////////

int ar_util_16BAto4BA( size_t* deltalen, byteptr buf, size_t bufsize, word16ptr in, size_t insize )
{
	int rc=0;
	size_t j=0, i=0;
	while( i != insize )
	{
		if( j + 3 >= bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		// note: reading input memory backwards so we output high chars to low memy
		buf[ j++ ] = b16charout[ ( in[insize-1-i] >> 12 ) & 0x0F ];
		buf[ j++ ] = b16charout[ ( in[insize-1-i] >>  8 ) & 0x0F ];
		buf[ j++ ] = b16charout[ ( in[insize-1-i] >>  4 ) & 0x0F ];
		buf[ j++ ] = b16charout[ ( in[insize-1-i] ) & 0x0F ];
		i++;
	}
	*deltalen = j;
	return rc;
}

int ar_util_4BAto16BA( size_t* deltalen, word16ptr buf, size_t bufsize, byteptr in, size_t insize )
{
	int rc=0;
	size_t i=0, j=0;
	while( i != insize ) {
		if( j == bufsize ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); break; }
		word16 w = 0;
		// note: reading input memory backwards so we output low words to low memy
		w |= (word16)( b16charin[ in[insize-i-1] - 0x30 ] & 0x0F );			i++;	if( i == insize ) { goto EOS; }
		w |= (word16)( b16charin[ in[insize-i-1] - 0x30 ] & 0x0F ) <<  4;	i++;	if( i == insize ) { goto EOS; }
		w |= (word16)( b16charin[ in[insize-i-1] - 0x30 ] & 0x0F ) <<  8;	i++;	if( i == insize ) { goto EOS; }
		w |= (word16)( b16charin[ in[insize-i-1] - 0x30 ] & 0x0F ) << 12;	i++;
EOS:
		buf[ j++ ] = w;
	}
	*deltalen = j;
	return rc;
}

//////////////////

int ar_util_txt2vl( vlPoint v, char* buf, size_t bufsize )
{
	int rc = 0;
	word16 words = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);
	vlClear( v );
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	if( rc = ar_util_6BAto8BA( &deltalen, tmp, sizeof(vlPoint), buf, bufsize ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_8BAto16BA( &deltalen, &v[1], words, tmp, deltalen ) ) { LOGFAIL( rc ); goto EXIT; }
	v[0] = (word16)deltalen;
	if( !vlIsValid( v ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); vlClear( v ); goto EXIT; }
EXIT:
	return rc;
}

// concatenates into buf
int ar_util_vl2txt( char* buf, size_t bufsize, vlPoint v )
{
	int rc = 0;
	if( !vlIsValid( v ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	size_t deltalen = 0;
	size_t buflen = strlen(buf);
	char tmp[ sizeof( vlPoint) + 2 ] = {0};
	if( rc = ar_util_16BAto8BA( &deltalen, tmp, sizeof(vlPoint), v+1, v[0] ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, tmp, deltalen ) ) { LOGFAIL( rc ); goto EXIT; }
	buf[ buflen + deltalen ] = 0;
EXIT:
	return rc;
}

int ar_util_txttow16( word16* pw, char* buf, size_t bufsize )
{
	int rc = 0;
	if( !pw ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	*pw = 0;
	size_t deltalen = 0;
	char tmp[ sizeof(word16) + 2 ] = {0};
	if( rc = ar_util_6BAto8BA( &deltalen, tmp, sizeof(word16), buf, bufsize ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_8BAto16BA( &deltalen, pw, 1, tmp, deltalen ) ) { LOGFAIL( rc ); goto EXIT; }
EXIT:
	return rc;
}

int ar_util_w16totxt( char* buf, size_t bufsize, word16* pw )
{
	int rc = 0;
	size_t deltalen = 0;
	size_t buflen = strlen(buf);
	char tmp[ sizeof(word16) + 2 ] = {0};
	if( rc = ar_util_16BAto8BA( &deltalen, tmp, sizeof(vlPoint), pw, 1 ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, tmp, deltalen ) ) { LOGFAIL( rc ); goto EXIT; }
EXIT:
	return rc;
}

//////////////////

void ar_util_test()
{

#if defined(_DEBUG)

	printf("# ar_util_test\n");

	#define BUFSIZE 100
	int rc = 0;
	char buf1[ BUFSIZE ], buf2[ BUFSIZE ];
	char* txtArr[] =
	{
		"I",
		"Me",
		"You",
		"You!",
		"You?!",
		"dog is bad.",
		"dog is good.",
		"dog is angry.",
		"dog is hungry.",
		0
	};

	// 8->6->8 roundtrip
	for( int i=0; txtArr[i] != 0; i++ )
	{
		char* txt = txtArr[i];

		{
			size_t deltalen = 0;
			byteptr appendpos = buf1;
			size_t appendsize = BUFSIZE;
			rc = ar_util_8BAto6BA( &deltalen, appendpos, appendsize, txt, strlen(txt) );
			ASSERT( rc == 0 );
			buf1[ deltalen ] = 0;
		}
		{
			size_t deltalen = 0;
			byteptr appendpos = buf2;
			size_t appendsize = BUFSIZE;
			rc = ar_util_6BAto8BA( &deltalen, appendpos, appendsize, buf1, strlen(buf1) );
			ASSERT( rc == 0 );
			buf2[ deltalen ] = 0;
		}
		ASSERT( strcmp( txt, buf2 ) == 0 );
	}

	// 16->6->16 roundtrip
	for( int i=0; i<200; i++ )
	{
		char buf[1024];
		vlPoint v0, v1;

		vlClear( v0 );
		v0[0] = ar_util_rnd32() % VL_UNITS;
		for( int j=0; j<v0[0]; j++) { v0[j+1] = (word16)ar_util_rnd32(); }
		v0[2]=0x00ff;

		buf[0]=0;
		rc = ar_util_vl2txt( buf, 1024, v0 );
		ASSERT( rc == 0 );

		rc = ar_util_txt2vl( v1, buf, strlen(buf) );
		ASSERT( rc == 0 );

		ASSERT( vlEqual( v0, v1 ) );
	}

#endif
}

///////////////////////////////////////

int ar_util_strcat( byteptr dst, size_t dstsize, byteptr src )
{
	size_t len = 0;
	while( *dst ) { dst++; if( ++len > dstsize-1 ) { return RC_BUFOVERFLOW; } }
	while( *src ) { *dst = *src; src++; dst++; if( ++len > dstsize-1 ) { return RC_BUFOVERFLOW; } }
	*dst = 0; 
	return 0;
}

int ar_util_strncat( byteptr dst, size_t dstsize, byteptr src, size_t srcsize )
{
	size_t len = 0;
	size_t cpy = 0;
	while( *dst ) { dst++; if( ++len > dstsize-1 ) { return RC_BUFOVERFLOW; } }
	while( *src ) { *dst = *src; src++; dst++; if( ++len > dstsize-1 ) { return RC_BUFOVERFLOW; } if( ++cpy == srcsize ) { break; } }
	*dst = 0; 
	return 0;
}

///////////////////////////////////////

const char* ar_util_rclookup( int rc )
{
	#undef RC_XXX
	#define RC_XXX(a,b) { #a , b },

	static struct {
		char* sz;
		int c;
	} table[] = {

#include "ar_codes.x"

	};

	if( rc >= 0 ) return "OK";

	for( int i=0; table[i].c != RC_MARKER; i++ ) { if( table[i].c == rc ) { return table[i].sz; } }

	return "ERR";
}

///////////////////////////////////////

word32 ar_util_rnd32()
{

#if defined(_WINDOWS)

	word32 r;
	rand_s( &r );
	return r;
	
#else

	return rand();

#endif

}

word16 ar_util_rnd16()
{
	return (word16)ar_util_rnd32();
}

int ar_util_isvalid7bit( byteptr szRecord )
{
	for( size_t i = 0 ; szRecord[i] != 0; i++ )
	{
		if( szRecord[i] & 0x80 ) { return 0; }
	}
	return 1;
}
