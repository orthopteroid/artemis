// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_uricodec.h"
#include "ec_vlong.h"
#include "ar_util.h"

#include "ar_core.h" // for testing

//////////////////////

static int txt_to_vl( vlPoint v, char* buf )
{
	int rc=0;
	word16 words = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);
	vlClear( v );
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	rc = ar_util_6BAto8BA( &deltalen, tmp, sizeof(vlPoint), buf, strlen(buf) );
	if( !rc ) { rc = ar_util_8BAto16BA( &deltalen, &v[1], words, tmp, deltalen ); }
	if( rc == 0 ) { v[0] = (word16)deltalen; }
	return rc;
}

// concatenates into buf
static int vl_to_txt_cat( char* buf, size_t bufsize, vlPoint v )
{
	int rc=0;
	size_t deltalen = 0;
	size_t buflen = strlen(buf);
	char tmp[ sizeof( vlPoint) + 2 ] = {0};
	rc = ar_util_16BAto8BA( &deltalen, tmp, sizeof(vlPoint), v+1, v[0] );
	if( !rc ) { rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, tmp, deltalen ); }
	if( !rc ) { buf[ buflen + deltalen ] = 0; }
	return rc;
}

//////////////////////

// in-place attribute-value scanner using temporary \0 markers. split tokens is '&'
// scanner can split values by ordinal as well, as long as they are deliminated by '!'
// assumes parse string is \0 terminated but no assumptions about attribute order

typedef struct 
{
	byteptr buf_start /* 1st byte */, buf_end /* ending null */;
	byteptr seg_start, seg_end;
	char seg_end_char;
} parsestate;

static void ps_init( parsestate *ps, byteptr uribuf )
{
	ps->buf_start = uribuf;
	ps->buf_end = uribuf + strlen( uribuf );
	ps->seg_start = ps->seg_end = 0;
}

static void ps_cleanup( parsestate *ps )
{
	if( ps->seg_end == 0 ) { return; }
	if( ps->seg_end != ps->buf_end ) { *ps->seg_end = ps->seg_end_char; }
	ps->seg_end = 0;
}

static int ps_scan_item( parsestate *ps, byteptr prefix, int itemnum )
{
	ps_cleanup( ps );

	// find prefix
	ps->seg_start = strstr( ps->buf_start, prefix );
	if( ps->seg_start == 0 ) { return -1; }
	ps->seg_start += strlen( prefix );

	// jump to itemnum
	while( itemnum )
	{
		while( 1 )
		{
			ps->seg_start += 1;
			byte ch = *ps->seg_start;
			if( ch == 0 || ch == '&' || ch == '?' )
			{
				if( itemnum == 0 ) { break; } // ok to hit token when looking for item 0
				return -1; // missed item
			}
			if( ch == '!' ) { break; } // found item
		}
		ps->seg_start += 1;
		itemnum--;
	}

	// find suffix
	ps->seg_end = ps->seg_start;
	while( 1 )
	{
		byte ch = *ps->seg_end;
		if( ch == '!' || ch == '&' || ch == '?' ) { ps->seg_end_char = ch; *ps->seg_end = 0; break; }
		if( ch == 0 ) { ASSERT( ps->seg_end == ps->buf_end ); break; }
		ps->seg_end += 1;
	}
	return 0;
}

///////////////////

void ar_uri_estimatebufsize( size_t* uribufsize, arAuth* pARecord )
{
	*uribufsize = 16; // padding
	*uribufsize += 8; // verify
	*uribufsize += 3 * 4; // info
	*uribufsize += pARecord->pubkey[0] * 4; // key
	*uribufsize += pARecord->authsig.r[0] * 4 + pARecord->authsig.s[0] * 4; // signature
	*uribufsize += pARecord->bufused; // message
	*uribufsize = *uribufsize * 4 / 3; // b64 encoding
}

void ar_uri_estimatemessagesize( size_t* uribufsize, byteptr buf )
{
	*uribufsize = 0;

	if( strstr( buf, "&mt=" ) )
	{
		*uribufsize = strlen( buf ) - (strstr( buf, "&mt=" ) - buf) + 16; // +16 for padding
	}
}

void ar_uri_estimateshares( word16* shares, byteptr buf )
{
	*shares = 0;

	parsestate ss;
	ps_init( &ss, buf );

	int rc = 0;
	if( ps_scan_item( &ss, "&si=", 0 ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( shares, ss.seg_start ) ) { ASSERT(0); goto FAIL; }
FAIL:
	ps_cleanup( &ss );
}

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord )
{
	char sz[3] = {0,0,0};
	int rc = 0;

	strcat_s( buf, bufsize, "artemis:" );
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->topic ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "?vf=" );
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->verify ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "&si=" );
	if( rc = ar_util_12Bto6B( sz, pARecord->shares ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, sz );
	strcat_s( buf, bufsize, "!" );
	if( rc = ar_util_12Bto6B( sz, pARecord->threshold ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, sz );
	strcat_s( buf, bufsize, "!" );
	if( rc = ar_util_12Bto6B( sz, pARecord->fieldsize ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, sz );

	strcat_s( buf, bufsize, "&pk=" );
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->pubkey ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "&as=" );
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.r ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, "!" );
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.s ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "&mt=" );
	size_t deltalen = 0;
	size_t buflen = strlen(buf);
	rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, pARecord->buf, pARecord->bufused );
	if( rc == 0 ) { buf[ deltalen + buflen ] = 0; } else { ASSERT(0); goto DONE; }

DONE:

	return rc;
}

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord )
{
	char sz[3] = {0,0,0};
	int rc = 0;

	strcat_s( buf, bufsize, "artemis:" );
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->topic ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "?sh=" );
	if( rc = ar_util_12Bto6B( sz, pSRecord->shareid ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, sz );
	strcat_s( buf, bufsize, "!" );
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->share ) ) { ASSERT(0); goto DONE; }

	strcat_s( buf, bufsize, "&ss=" );
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.r ) ) { ASSERT(0); goto DONE; }
	strcat_s( buf, bufsize, "!" );
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.s ) ) { ASSERT(0); goto DONE; }

DONE:

	return rc;
}

int ar_uri_parse( arAuth* pARecord, arShare* pSRecord, byteptr szRecord )
{
	if( !pARecord || !pSRecord || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	parsestate ss;
	parsestate* pss = &ss;
	ps_init( pss, szRecord );

	int rc = 0;
	int isARecord = strstr( szRecord, "&mt=" ) ? 1 : 0;

	if( isARecord )
	{
		size_t cryptlen = strlen( szRecord ) - (strstr( szRecord, "&mt=" ) - szRecord);
		if( cryptlen >= pARecord->bufmax ) { ASSERT(0); rc=-2; goto FAIL; }

		if( ps_scan_item( pss, "artemis:", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pARecord->topic, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "vf=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pARecord->verify, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "si=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = ar_util_6Bto12B( &pARecord->shares, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
		if( ps_scan_item( pss, "si=", 1 ) ) { ASSERT(0); goto FAIL; }
		if( rc = ar_util_6Bto12B( &pARecord->threshold, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
		if( ps_scan_item( pss, "si=", 2 ) ) { ASSERT(0); goto FAIL; }
		if( rc = ar_util_6Bto12B( &pARecord->fieldsize, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "pk=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pARecord->pubkey, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "as=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pARecord->authsig.r, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
		if( ps_scan_item( pss, "as=", 1 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pARecord->authsig.s, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "mt=", 0 ) ) { ASSERT(0); goto FAIL; }
		size_t deltalen = 0;
		if( rc = ar_util_6BAto8BA( &deltalen, pARecord->buf, pARecord->bufmax, pss->seg_start, strlen( pss->seg_start ) ) ) { ASSERT(0); goto FAIL; }
		pARecord->bufused = (word16)deltalen;
	} else {
		if( ps_scan_item( pss, "artemis:", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pSRecord->topic, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "sh=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = ar_util_6Bto12B( &pSRecord->shareid, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
		if( ps_scan_item( pss, "sh=", 1 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pSRecord->share, pss->seg_start ) ) { ASSERT(0); goto FAIL; }

		if( ps_scan_item( pss, "ss=", 0 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pSRecord->sharesig.r, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
		if( ps_scan_item( pss, "ss=", 1 ) ) { ASSERT(0); goto FAIL; }
		if( rc = txt_to_vl( pSRecord->sharesig.s, pss->seg_start ) ) { ASSERT(0); goto FAIL; }
	}

	ASSERT( rc == 0 );

	ps_cleanup( pss );
	return isARecord ? 1 : 2;

FAIL:

	ps_cleanup( pss );
	return rc;
}

void ar_uri_test()
{

#if defined(_DEBUG)

	printf("# ar_uri_test: ");

	char* bufa = 0;
	char* bufs0 = 0;
	char* bufs1 = 0;

	arAuth* arecord = 0;
	arShare srecords[2];

	arAuth* arecord_ = 0;
	arShare srecords_[2];

	char cleartextin[20];
	char cleartextout[80];

	if( (arecord = malloc( sizeof(arAuth) + 80 )) == 0 ) { ASSERT(0); goto EXIT; }
	if( (arecord_ = malloc( sizeof(arAuth) + 80 )) == 0 ) { ASSERT(0); goto EXIT; }

	if( (bufa = malloc( 2048 )) == 0 ) { ASSERT(0); goto EXIT; }
	if( (bufs0 = malloc( 2048 )) == 0 ) { ASSERT(0); goto EXIT; }
	if( (bufs1 = malloc( 2048 )) == 0 ) { ASSERT(0); goto EXIT; }

	int numtests = 100;
	for( int i=0; i<numtests; i++ )
	{
		int rc = 0;

		for( int j=0; j<20; j++ ) { cleartextin[j] = (char)(platform_rnd32() % (122 - 32) + 32); }
		cleartextin[ platform_rnd32() % 20 ] = 0;

		memset( arecord, 0, sizeof(arAuth) + 80 );
		memset( srecords, 0, sizeof(arShare) * 2 );
		arecord->bufmax = 80;

		rc = ar_core_create( arecord, srecords, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1) ); // +1 to include \0
		ASSERT( rc == 0 );

		bufa[0] = 0;
		rc = ar_uri_create_a( bufa, 2048, arecord );
		ASSERT( rc == 0 );

		bufs0[0] = 0;
		rc = ar_uri_create_s( bufs0, 2048, &srecords[0] );
		ASSERT( rc == 0 );

		bufs1[0] = 0;
		rc = ar_uri_create_s( bufs1, 2048, &srecords[1] );
		ASSERT( rc == 0 );

		if(0)
		{
			printf("%s\n",bufa);
			printf("%s\n",bufs0);
			printf("%s\n",bufs1);
			ASSERT(0);
		}

		memset( arecord_, 0, sizeof(arAuth) + 80 );
		memset( srecords_, 0, sizeof(arShare) * 2 );
		arecord_->bufmax = 80;

		rc = ar_uri_parse( arecord_, &srecords_[0], bufa );
		ASSERT( rc == 1 );

		rc = ar_uri_parse( arecord_, &srecords_[0], bufs0 );
		ASSERT( rc == 2 );

		rc = ar_uri_parse( arecord_, &srecords_[1], bufs1 );
		ASSERT( rc == 2 );

		cleartextout[0]=0;
		rc = ar_core_decrypt( cleartextout, 80, arecord_, srecords_, 2 );
		ASSERT( rc == 0 );

		ASSERT( strcmp( cleartextin, cleartextout ) == 0 );

		if(i > 0 &&  i % 10 == 0 ) { printf("%d",9 - i / (numtests / 9)); }
	}
	putchar('\n');

EXIT:

	if( bufa )	free( bufa );
	if( bufs0 )	free( bufs0 );
	if( bufs1 )	free( bufs1 );
	if( arecord )	free( arecord );
	if( arecord_ )	free( arecord_ );

#endif

}