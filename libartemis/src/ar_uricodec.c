// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"

#include "ar_uricodec.h"
#include "ec_vlong.h"
#include "ar_util.h"

#if defined(_DEBUG)
	#include "ar_core.h" // for testing
#endif

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
			if( ch == 0 || ch == '&' || ch == '?' || ch == '\n' )
			{
				if( itemnum == 0 ) { break; } // ok to hit token when looking for item 0
				return -2; // missed item
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
		if( ch == '!' || ch == '&' || ch == '?' || ch == '\n' ) { ps->seg_end_char = ch; *ps->seg_end = 0; break; }
		if( ch == 0 ) { ASSERT( ps->seg_end == ps->buf_end ); break; }
		ps->seg_end += 1;
	}
	return 0;
}

///////////////////

static void ar_uri_arglen( size_t* arglen, byteptr arg, byteptr buf )
{
	*arglen = 0;

	byteptr s = strstr( buf, arg );
	if( !s ) { return; }
	s += strlen(arg);

	byteptr e=s;
	while( *e != '\0' && *e != '!' && *e != '&' && *e != '?' ) { e++; }

	*arglen = e - s - 1; // -1 for /0
}

///////////////////

void ar_uri_bufsize_a( size_t* uribufsize, arAuth* pARecord )
{
	*uribufsize = ( sizeof(arAuth) + pARecord->bufused ) * 4 / 3; // b64 encoding
}

void ar_uri_bufsize_s( size_t* uribufsize, arShare* pSRecord )
{
	*uribufsize = ( sizeof(arShare) + pSRecord->bufused ) * 4 / 3; // b64 encoding
}

void ar_uri_parse_messlen( size_t* len, byteptr buf )
{
	ar_uri_arglen( len, "mt=", buf );
}

void ar_uri_parse_cluelen( size_t* len, byteptr buf )
{
	ar_uri_arglen( len, "mc=", buf );
	if( !len ) { ar_uri_arglen( len, "sc=", buf ); }
}

int ar_uri_parse_type( byteptr buf )
{
	if( strstr( buf, "ai=" ) )			{ return 1; }
	else if( strstr( buf, "si=" ) )		{ return 2; }
	return -1;
}

void ar_uri_parse_sharecount( word16* shares, byteptr buf )
{
	int rc = 0;
	*shares = 0;

	parsestate ss;
	ps_init( &ss, buf );

	if( strstr( buf, "ai=" ) )
	{
		if( rc = ps_scan_item( &ss, "&ai=", 0 ) ) { ASSERT(0); goto FAIL; }
	}
	else if( strstr( buf, "si=" ) )
	{
		if( rc = ps_scan_item( &ss, "&si=", 0 ) ) { ASSERT(0); goto FAIL; }
	}
	else { ASSERT(0); goto FAIL; }

	if( rc = ar_util_6Bto12B( shares, ss.seg_start ) ) { ASSERT(0); goto FAIL; }

FAIL:
	ps_cleanup( &ss );
}

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord )
{
	size_t buflen = 0;
	char sz[3] = {0,0,0};
	int rc = 0;

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strncat( buf, bufsize, pARecord->buf, pARecord->loclen ) ) { ASSERT(0); goto DONE; }
	
	if( rc = ar_util_strcat( buf, bufsize, "?" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "tp=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->topic ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "ai=" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_12Bto6B( sz, pARecord->shares ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_12Bto6B( sz, pARecord->threshold ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_12Bto6B( sz, pARecord->fieldsize ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "vf=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->verify ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "pk=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->pubkey ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "as=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.r ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.s ) ) { ASSERT(0); goto DONE; }

	if( pARecord->cluelen > 0 )
	{
		if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
		if( rc = ar_util_strcat( buf, bufsize, "mc=" ) ) { ASSERT(0); goto DONE; }
		buflen = strlen(buf);
		size_t mclen = 0;
		rc = ar_util_8BAto6BA( &mclen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen, pARecord->cluelen );
		if( rc == 0 ) { buf[ mclen + buflen ] = 0; } else { ASSERT(0); goto DONE; }
	}

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "mt=" ) ) { ASSERT(0); goto DONE; }
	buflen = strlen(buf);
	size_t mtlen = 0;
	rc = ar_util_8BAto6BA( &mtlen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen + pARecord->cluelen, pARecord->bufused - pARecord->loclen - pARecord->cluelen );
	if( rc == 0 ) { buf[ mtlen + buflen ] = 0; } else { ASSERT(0); goto DONE; }

DONE:

	return rc;
}

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord )
{
	size_t buflen = 0;
	char sz[3] = {0,0,0};
	int rc = 0;

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strncat( buf, bufsize, pSRecord->buf, pSRecord->loclen ) ) { ASSERT(0); goto DONE; }
	
	if( rc = ar_util_strcat( buf, bufsize, "?" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "tp=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->topic ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "si=" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_12Bto6B( sz, pSRecord->shares ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_12Bto6B( sz, pSRecord->shareid ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "sh=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->share ) ) { ASSERT(0); goto DONE; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "ss=" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.r ) ) { ASSERT(0); goto DONE; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto DONE; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.s ) ) { ASSERT(0); goto DONE; }

	if( pSRecord->bufused > 0 )
	{
		if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto DONE; }
		if( rc = ar_util_strcat( buf, bufsize, "sc=" ) ) { ASSERT(0); goto DONE; }
		size_t deltalen = 0;
		buflen = strlen(buf);
		rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, pSRecord->buf + pSRecord->loclen, pSRecord->bufused - pSRecord->loclen );
		if( rc == 0 ) { buf[ deltalen + buflen ] = 0; } else { ASSERT(0); goto DONE; }
	}

DONE:

	return rc;
}

int ar_uri_parse_a( arAuth* pARecord, byteptr szRecord, byteptr location )
{
	int rc = 0;

	if( !pARecord || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	parsestate ss;
	parsestate* pss = &ss;
	ps_init( pss, szRecord );

	size_t cryptlen = 0;
	ar_uri_arglen( &cryptlen, "mt=", szRecord );
	if( cryptlen >= pARecord->bufmax ) { ASSERT(0); rc=-2; goto FAIL; }

	if( rc = ps_scan_item( pss, "tp=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pARecord->topic, pss->seg_start ) ) 			{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "vf=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pARecord->verify, pss->seg_start ) ) 			{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "ai=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( &pARecord->shares, pss->seg_start ) ) 	{ ASSERT(0); goto FAIL; }
	if( rc = ps_scan_item( pss, "ai=", 1 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( &pARecord->threshold, pss->seg_start ) ) 	{ ASSERT(0); goto FAIL; }
	if( rc = ps_scan_item( pss, "ai=", 2 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( &pARecord->fieldsize, pss->seg_start ) ) 	{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "pk=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pARecord->pubkey, pss->seg_start ) ) 			{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "as=", 0 ) )							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pARecord->authsig.r, pss->seg_start ) ) 		{ ASSERT(0); goto FAIL; }
	if( rc = ps_scan_item( pss, "as=", 1 ) )							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pARecord->authsig.s, pss->seg_start ) ) 		{ ASSERT(0); goto FAIL; }

	byteptr bufloc = pARecord->buf;
	size_t buflen = 0;

	size_t loclen = 0;
	if( rc = ps_scan_item( pss, location, 0 ) )							{ ASSERT(0); goto FAIL; }
	loclen = strlen( location );
	strcpy_s( bufloc, pARecord->bufmax, location );
	bufloc += loclen;
	buflen += loclen;

	size_t mclen = 0;
	if( ps_scan_item( pss, "mc=", 0 ) ) { /* optional */ } else
	{
		if( rc = ar_util_6BAto8BA( &mclen, bufloc, pARecord->bufmax - buflen, pss->seg_start, strlen( pss->seg_start ) ) ) { ASSERT(0); goto FAIL; }
		bufloc += mclen;
		buflen += mclen;
	}

	size_t mtlen = 0;
	if( rc = ps_scan_item( pss, "mt=", 0 ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_6BAto8BA( &mtlen, bufloc, pARecord->bufmax - buflen, pss->seg_start, strlen( pss->seg_start ) ) ) { ASSERT(0); goto FAIL; }
	bufloc += mtlen;
	buflen += mtlen;

	pARecord->loclen = (word16)(loclen);
	pARecord->cluelen = (word16)(mclen);
	pARecord->bufused = (word16)(buflen);

FAIL:

	ps_cleanup( pss );
	return rc;
}

int ar_uri_parse_s( arShare* pSRecord, byteptr szRecord, byteptr location )
{
	int rc = 0;

	if( !pSRecord || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	parsestate ss;
	parsestate* pss = &ss;
	ps_init( pss, szRecord );

	if( rc = ps_scan_item( pss, "tp=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pSRecord->topic, pss->seg_start ) ) 			{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "si=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( &pSRecord->shares, pss->seg_start ) ) 	{ ASSERT(0); goto FAIL; }
	if( rc = ps_scan_item( pss, "si=", 1 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = ar_util_6Bto12B( &pSRecord->shareid, pss->seg_start ) ) 	{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "sh=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pSRecord->share, pss->seg_start ) ) 			{ ASSERT(0); goto FAIL; }

	if( rc = ps_scan_item( pss, "ss=", 0 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pSRecord->sharesig.r, pss->seg_start ) ) 		{ ASSERT(0); goto FAIL; }
	if( rc = ps_scan_item( pss, "ss=", 1 ) ) 							{ ASSERT(0); goto FAIL; }
	if( rc = txt_to_vl( pSRecord->sharesig.s, pss->seg_start ) ) 		{ ASSERT(0); goto FAIL; }

	byteptr bufloc = pSRecord->buf;
	size_t buflen = 0;

	size_t loclen = 0;
	if( rc = ps_scan_item( pss, location, 0 ) )							{ ASSERT(0); goto FAIL; }
	loclen = strlen( location );
	strcpy_s( bufloc, pSRecord->bufmax, location );
	bufloc += loclen;
	buflen += loclen;

	size_t sclen = 0;
	if( ps_scan_item( pss, "sc=", 0 ) ) { /* optional */ } else
	{
		if( rc = ar_util_6BAto8BA( &sclen, bufloc, pSRecord->bufmax - buflen, pss->seg_start, strlen( pss->seg_start ) ) ) { ASSERT(0); goto FAIL; }
		bufloc += sclen;
		buflen += sclen;
	}

	pSRecord->loclen = (word16)(loclen);
	pSRecord->bufused = (word16)(buflen);

FAIL:

	ps_cleanup( pss );
	return rc;
}

void ar_uri_test()
{

#if defined(_DEBUG)

	printf("# ar_uri_test: ");

	typedef struct arAuth80_
	{
		arAuth x;
		byte y[80];
	} arAuth80;

	typedef struct arShare80_
	{
		arShare x;
		byte y[80];
	} arShare80;

	typedef arShare80* arShare80ptr;

	arAuth80		arecord;
	arShare80		srecords[2];
	arShare80ptr	srecordarr[2] = { &srecords[0], &srecords[1] };

	memset( &arecord, 0, sizeof(arAuth80) );
	memset( &srecords[0], 0, sizeof(arShare80) );
	memset( &srecords[1], 0, sizeof(arShare80) );
	arecord.x.bufmax = srecords[0].x.bufmax = srecords[1].x.bufmax = 80;

	arAuth80		arecord_;
	arShare80		srecords_[2];
	arShare80ptr	srecordarr_[2] = { &srecords_[0], &srecords_[1] };

	memset( &arecord_, 0, sizeof(arAuth80) );
	memset( &srecords_[0], 0, sizeof(arShare80) );
	memset( &srecords_[1], 0, sizeof(arShare80) );
	arecord_.x.bufmax = srecords_[0].x.bufmax = srecords_[1].x.bufmax = 80;

	char* clues[3] = {"topiclue", "clue1", "clue2"};
	char* location = "foo.bar";

	char cleartextin[20];
	char cleartextout[80];

	typedef byte byte2048[2048];

	byte2048 bufa, bufs0, bufs1;

	int numtests = 100;
	for( int i=0; i<numtests; i++ )
	{
		int rc = 0;

		for( int j=0; j<20; j++ ) { cleartextin[j] = (char)(ar_util_rnd32() % (122 - 32) + 32); }
		cleartextin[ ar_util_rnd32() % 20 ] = 0;

		rc = ar_core_create( &arecord.x, (arShareptr*)srecordarr, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1), (byteptr*)clues, location ); // +1 to include \0
		ASSERT( rc == 0 );

		bufa[0] = 0;
		rc = ar_uri_create_a( bufa, 2048, &arecord.x );
		ASSERT( rc == 0 );

		bufs0[0] = 0;
		rc = ar_uri_create_s( bufs0, 2048, &srecords[0].x );
		ASSERT( rc == 0 );

		bufs1[0] = 0;
		rc = ar_uri_create_s( bufs1, 2048, &srecords[1].x );
		ASSERT( rc == 0 );

		if(0)
		{
			printf("%s\n",bufa);
			printf("%s\n",bufs0);
			printf("%s\n",bufs1);
		}

		rc = ar_uri_parse_a( &arecord_.x, bufa, location );
		ASSERT( rc == 0 );

		rc = ar_uri_parse_s( &srecords_[0].x, bufs0, location );
		ASSERT( rc == 0 );

		rc = ar_uri_parse_s( &srecords_[1].x, bufs1, location );
		ASSERT( rc == 0 );

		cleartextout[0]=0;
		rc = ar_core_decrypt( cleartextout, 80, &arecord_.x, (arShareptr*)srecordarr_, 2 );
		ASSERT( rc == 0 );

		ASSERT( strcmp( cleartextin, cleartextout ) == 0 );

		if(i > 0 &&  i % 10 == 0 ) { printf("%d",9 - i / (numtests / 9)); }
	}
	putchar('\n');

#endif

}
