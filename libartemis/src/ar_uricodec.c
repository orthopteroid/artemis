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

static int txt_to_vl( vlPoint v, char* buf, size_t bufsize )
{
	int rc=0;
	word16 words = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);
	vlClear( v );
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	rc = ar_util_6BAto8BA( &deltalen, tmp, sizeof(vlPoint), buf, bufsize );
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

typedef struct
{
	byteptr buf_first;
	byteptr buf_last;
	//
	byteptr curr;
	word32	tagID;
	byteptr	tagPTR;
	//
	byte	data_item;
	byteptr	data_first;
	byteptr	data_last;
	size_t	data_len;
} parsestate2;
typedef parsestate2* ps2ptr;

static void ps2_init( ps2ptr pps, byteptr buf, size_t buflen )
{
	if( !pps ) { ASSERT(0); return; }
	pps->buf_first = pps->curr = buf;
	pps->buf_last = buf + buflen - 1;
	pps->tagID = 0;
	//
	pps->tagPTR = pps->data_last = pps->data_first = 0;
}

static void ps2_scan_private( ps2ptr pps )
{
	while( pps->curr <= pps->buf_last )
	{
		// buf can't start with tag marker, as there would be no psuedotag (+2 to ensure a 2 char psuedotag)
		// also, buf can't end with tag marker, as there would be no data (-1 to allow 1 char of data at end)
		if( pps->buf_first + 2 <= pps->curr && pps->curr <= pps->buf_last - 3 - 1 )
		{
			// look for tag patterns ?__= or &__=
			if( pps->curr[0] == '?' && pps->curr[3] == '=' ) { break; } // found front tag
			if( pps->curr[0] == '&' && pps->curr[3] == '=' ) { break; } // found interior tag
		}
		// buf can't start with tag delim, as there would be no tag marker (+7 to ensure a 2 char psuedotag, 4 char tag with markers and 1 char data before delim)
		// also, buf can't end with tag delim, as there would be no data (-1 to allow 1 char of data at end)
		if( pps->buf_first + 7 <= pps->curr && pps->curr <= pps->buf_last - 1 )
		{
			if( pps->curr[0] == '!' ) { break; } // found tag delim
		}
		if( pps->curr[0] == '\n' ) { break; } // end of line
		(pps->curr) += 1; // next char
	}
}

static word32 ps2_token( ps2ptr pps )
{
	int bPsuedoTag = ( pps->curr == pps->buf_first );

	if( pps->curr > pps->buf_last ) { return pps->tagID = 0; } // no more

	///////////////////////
	// find next tag

	ps2_scan_private( pps );
	if( pps->curr[0] == '\n' ) { pps->curr++; return pps->tagID = '\n\0\0\0'; } // end of line
	if( pps->curr > pps->buf_last ) { ASSERT(0); goto FINI; } // found buffer end without finding a tag

	///////////////////////
	// handle tag type

	if( bPsuedoTag )
	{
		if( pps->curr[0] != '?' ) { ASSERT(0); goto FINI; }
		pps->tagPTR = pps->buf_first; // return psuedotag
	}
	else if( pps->curr[0] == '&' || pps->curr[0] == '?' )
	{
		pps->tagPTR = pps->curr + 1; // construct tag from buf chars
	}
	else if( pps->curr[0] == '!' )
	{
		// no touchie tagPTR
	}
	else { ASSERT(0); goto FINI; } // check for invalid scan termination

	///////////////////////
	// scan for data

	if( bPsuedoTag )
	{
		if( pps->curr[0] != '?' ) { ASSERT(0); goto FINI; }
		pps->data_item = 0;
		pps->data_first = pps->buf_first; // data for tag starts at start of buf
		pps->data_last = pps->curr - 1;
	}
	else
	{
		if( pps->data_last == 0 ) { ASSERT(0); goto FINI; }

		if( pps->data_last[1] == '!' )
		{
			pps->curr += 1; // skip sequence delimiter
			pps->data_item++; // inc count when data is split
		}
		else
		{
			pps->curr += 4; // skip over tag to start of data
			pps->data_item = 0;
		}
		pps->data_first = pps->curr;
		ps2_scan_private( pps ); // find data for tag
		pps->data_last = pps->curr - 1; // data ends prior to stopping char (or buf end)
	}
	pps->data_len = pps->data_last - pps->data_first + 1;

	///////////////////////
	// set the return tag

	char char3 = bPsuedoTag ? '=' : pps->tagPTR[2];

	pps->tagID = 0;
#if defined(LITTLE_ENDIAN)
	pps->tagID |= ((pps->tagPTR[0])     <<24)&0xff000000;
	pps->tagID |= ((pps->tagPTR[1])     <<16)&0x00ff0000;
	pps->tagID |= ((char3)              << 8)&0x0000ff00;
	pps->tagID |= ((0x30+pps->data_item)<< 0)&0x000000ff;
#else
	pps->tagID |= ((pps->tagPTR[0])     << 0)&0x000000ff;
	pps->tagID |= ((pps->tagPTR[1])     << 8)&0x0000ff00;
	pps->tagID |= ((char3)              <<16)&0x00ff0000;
	pps->tagID |= ((0x30+pps->data_item)<<24)&0xff000000;
#endif
	return pps->tagID;

FINI:
	pps->curr = pps->buf_last;
	return pps->tagID = 0;
}

///////////////////

static void ar_uri_arglen( size_t* pArglen, byteptr arg, byteptr buf )
{
	*pArglen = 0;

	byteptr s = strstr( buf, arg );
	if( !s ) { return; }
	s += strlen(arg);

	byteptr e=s;
	while( *e != '\n' && *e != '\0' && *e != '!' && *e != '&' && *e != '?' ) { e++; }

	*pArglen = e - s;
}

///////////////////

void ar_uri_bufsize_a( size_t* pUribufsize, arAuth* pARecord )
{
	*pUribufsize = ( sizeof(arAuth) + pARecord->bufused ) * 4 / 3; // b64 encoding
}

void ar_uri_bufsize_s( size_t* pUribufsize, arShare* pSRecord )
{
	*pUribufsize = ( sizeof(arShare) + pSRecord->bufused ) * 4 / 3; // b64 encoding
}

int ar_uri_locate_clue( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord )
{
	int rc = 0;
	
	*ppFirst = *ppLast = 0;
	
	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, szRecord, strlen( szRecord ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'mc=0' || token == 'sc=0' )
		{ *ppFirst = ss.data_first; *ppLast = ss.data_last; break; }
	}

	return rc;	
}

int ar_uri_locate_topic( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord )
{
	int rc = 0;
	
	*ppFirst = *ppLast = 0;
	
	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, szRecord, strlen( szRecord ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'tp=0' )
		{ *ppFirst = ss.data_first; *ppLast = ss.data_last; break; }
	}

	return rc;	
}

int ar_uri_locate_location( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord )
{
	int rc = 0;
	
	if( !ppFirst || !ppLast || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	*ppFirst = *ppLast = 0;

	byteptr s, e;
	s = e = 0;
	s = strstr( szRecord, "http://" );
	if( s == 0 ) { ASSERT(0); return -1; }
	s += 7;
	e = strchr( s, '?' );
	if( e == 0 ) { ASSERT(0); return -1; }
	e--;

	if( s > e ) { ASSERT(0); return -1; }

	*ppFirst = s;
	*ppLast = e;

	return rc;
}

void ar_uri_parse_messlen( size_t* pLen, byteptr buf )
{
	ar_uri_arglen( pLen, "mt=", buf );
}

void ar_uri_parse_cluelen( size_t* pLen, byteptr buf )
{
	ar_uri_arglen( pLen, "mc=", buf );
	if( !*pLen ) { ar_uri_arglen( pLen, "sc=", buf ); }
}

int ar_uri_parse_type( byteptr buf )
{
	if( strstr( buf, "ai=" ) )			{ return 1; }
	else if( strstr( buf, "si=" ) )		{ return 2; }
	return -1;
}

int ar_uri_parse_shareinfo( word16* pShares, word16* pThreshold, byteptr szRecord )
{
	int rc = 0;

	*pShares = *pThreshold = 0;

	byteptr pShare_start, pThreshold_start;
	pShare_start = pThreshold_start = 0;

	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, szRecord, strlen( szRecord ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'ai=0' )			{ pShare_start = ss.data_first; }
		else if( token == 'ai=1' )	{ pThreshold_start = ss.data_first; }
		else if( token == 'si=0' )	{ pShare_start = ss.data_first; }
		else if( token == 'si=1' )	{ pThreshold_start = ss.data_first; }
		if( pShare_start != 0 && pThreshold_start != 0 ) { break; }
	}
	if( pShare_start == 0 && pThreshold_start == 0 ) { ASSERT(0); return -1; }
	if( rc = ar_util_6Bto12B( pShares, pShare_start ) ) { ASSERT(0); return -1; }
	if( rc = ar_util_6Bto12B( pThreshold, pThreshold_start ) ) { ASSERT(0); return -1; }

	return rc;
}

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord )
{
	int rc = 0;

	size_t buflen = 0;
	char sz[3] = {0,0,0};

	size_t tokenlen = 0;
	word32 stateArr[] = {'tp\0\0', 'ai\0\0', 'vf\0\0', 'pk\0\0', 'as\0\0', 'mt\0\0', 'mc\0\0', 0 };

#if defined(_DEBUG)

	// impose some robustness on the order of the tags....
	int k = sizeof(stateArr) / sizeof(word32) -1 -1; // -1 conv from size to count, -1 to omit EOA marker
	for( int i = 0; i < k; i++ )
	{
		int j = ar_util_rnd32() % k;
		word32 t = stateArr[j];
		stateArr[j] = stateArr[i]; stateArr[i] = t;
	}

#endif

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strncat( buf, bufsize, pARecord->buf, pARecord->loclen ) ) { ASSERT(0); goto FAIL; }

	word32 state = 0;
	while( stateArr[ state ] )
	{
		// no clue, skip
		if( stateArr[ state ] == 'mc\0\0' && pARecord->cluelen == 0 ) { state++; continue; }
		//
		char* szDelim = ( state == 0 ) ? "?" : "&";
		if( rc = ar_util_strcat( buf, bufsize, szDelim ) ) { ASSERT(0); goto FAIL; }
		//
		switch( stateArr[ state ] )
		{
		default:
			ASSERT(0); break;
		case 'tp\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "tp=" ) ) { ASSERT(0); goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->topic ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ai\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "ai=" ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pARecord->shares ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pARecord->threshold ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pARecord->fieldsize ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'vf\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "vf=" ) ) { ASSERT(0); goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->verify ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'pk\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "pk=" ) ) { ASSERT(0); goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->pubkey ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'as\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "as=" ) ) { ASSERT(0); goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.r ) ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.s ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'mc\0\0':
			if( pARecord->cluelen == 0 ) { ASSERT(0); goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "mc=" ) ) { ASSERT(0); goto FAIL; }
			buflen = strlen(buf);
			rc = ar_util_8BAto6BA( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen, pARecord->cluelen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { ASSERT(0); goto FAIL; }
			break;
		case 'mt\0\0':
			if( rc = ar_util_strcat( buf, bufsize, "mt=" ) ) { ASSERT(0); goto FAIL; }
			buflen = strlen(buf);
			rc = ar_util_8BAto6BA( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen + pARecord->cluelen, pARecord->bufused - pARecord->loclen - pARecord->cluelen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { ASSERT(0); goto FAIL; }
			break;
		}
		state++;
	}

FAIL:

	return rc;
}

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord )
{
	size_t buflen = 0;
	char sz[3] = {0,0,0};
	int rc = 0;

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strncat( buf, bufsize, pSRecord->buf, pSRecord->loclen ) ) { ASSERT(0); goto FAIL; }
	
	if( rc = ar_util_strcat( buf, bufsize, "?" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "tp=" ) ) { ASSERT(0); goto FAIL; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->topic ) ) { ASSERT(0); goto FAIL; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "si=" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_12Bto6B( sz, pSRecord->shares ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_12Bto6B( sz, pSRecord->threshold ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_12Bto6B( sz, pSRecord->shareid ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { ASSERT(0); goto FAIL; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "sh=" ) ) { ASSERT(0); goto FAIL; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->share ) ) { ASSERT(0); goto FAIL; }

	if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "ss=" ) ) { ASSERT(0); goto FAIL; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.r ) ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { ASSERT(0); goto FAIL; }
	if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.s ) ) { ASSERT(0); goto FAIL; }

	if( pSRecord->bufused - pSRecord->loclen > 0 )
	{
		if( rc = ar_util_strcat( buf, bufsize, "&" ) ) { ASSERT(0); goto FAIL; }
		if( rc = ar_util_strcat( buf, bufsize, "sc=" ) ) { ASSERT(0); goto FAIL; }
		size_t deltalen = 0;
		buflen = strlen(buf);
		rc = ar_util_8BAto6BA( &deltalen, buf + buflen, bufsize - buflen, pSRecord->buf + pSRecord->loclen, pSRecord->bufused - pSRecord->loclen );
		if( rc == 0 ) { buf[ deltalen + buflen ] = 0; } else { ASSERT(0); goto FAIL; }
	}

FAIL:

	return rc;
}


int ar_uri_parse_a( arAuth* pARecord, byteptr szRecord, byteptr location )
{
	int rc = 0;

	if( !pARecord || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	// cached, as order is important when bundling
	size_t uLocation = 0, uClue = 0, uMessage = 0;
	byteptr pLocation = 0, pClue = 0, pMessage = 0;

	// vl conversion vars
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	const word16 VL_WORD_COUNT = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);

	parsestate2 ss;
	ps2ptr pss = &ss;
	ps2_init( pss, szRecord, strlen(szRecord) );

	word32 token = 0;
	while( token = ps2_token( pss ) )
	{
		if( token == '\n\0\0\0' ) { break; }

		// reset temps
		deltalen = 0; tmp[0] = 0;

		switch( token )
		{
		default: // unrecognized token
			break;
		case 'ht=0': // http psuedotoken
			if( pss->data_len <= 7 ) { ASSERT(0); goto FAIL; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = txt_to_vl( pARecord->topic, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ai=0': // arecord info - shares
			if( rc = ar_util_6Bto12B( &pARecord->shares, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ai=1': // arecord info - threshold
			if( rc = ar_util_6Bto12B( &pARecord->threshold, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ai=2': // arecord info - fieldsize
			if( rc = ar_util_6Bto12B( &pARecord->fieldsize, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'vf=0': // <verify>
			if( rc = txt_to_vl( pARecord->verify, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'pk=0': // <pubkey>
			if( rc = txt_to_vl( pARecord->pubkey, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'as=0': // <authsig> - r
			if( rc = txt_to_vl( pARecord->authsig.r, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'as=1': // <authsig> - s
			if( rc = txt_to_vl( pARecord->authsig.s, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'mt=0': // <messagetext>
			pMessage = pss->data_first;
			uMessage = pss->data_len;
			break;
		case 'mc=0': // <messageclue>
			pClue = pss->data_first;
			uClue = pss->data_len;
			break;
		}
	}

	// assemble order-dependent text body

	if( uLocation + uClue + uMessage > pARecord->bufmax ) { ASSERT(0); goto FAIL; }

	byteptr bufloc = pARecord->buf;
	size_t buflen = 0;

	if( uLocation == 0 ) { ASSERT(0); goto FAIL; }
	memcpy_s( bufloc, pARecord->bufmax, pLocation, uLocation );
	pARecord->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_6BAto8BA( &deltalen, bufloc, pARecord->bufmax - buflen, pClue, uClue ) ) { ASSERT(0); goto FAIL; }
		pARecord->cluelen = (word16)(deltalen);
		bufloc += deltalen;
		buflen += deltalen;
	}

	if( uMessage == 0 ) { ASSERT(0); goto FAIL; }
	if( rc = ar_util_6BAto8BA( &deltalen, bufloc, pARecord->bufmax - buflen, pMessage, uMessage ) ) { ASSERT(0); goto FAIL; }
	bufloc += deltalen;
	buflen += deltalen;

	pARecord->bufused = (word16)(buflen);

FAIL:

	return rc;
}

int ar_uri_parse_s( arShare* pSRecord, byteptr szRecord, byteptr location )
{
	int rc = 0;

	if( !pSRecord || !szRecord ) { ASSERT(0); return -1; }
	if( strlen( szRecord ) < 10 ) { ASSERT(0); return -1; }

	// cached, as order is important when bundling
	size_t uLocation = 0, uClue = 0;
	byteptr pLocation = 0, pClue = 0;

	// vl conversion vars
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	const word16 VL_WORD_COUNT = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);

	parsestate2 ss;
	ps2ptr pss = &ss;
	ps2_init( pss, szRecord, strlen(szRecord) );

	word32 token = 0;
	while( token = ps2_token( pss ) )
	{
		if( token == '\n\0\0\0' ) { break; }

		// reset temps
		deltalen = 0; tmp[0] = 0;

		switch( token )
		{
		default: // unrecognized token
			break;
		case 'ht=0': // http psuedotoken
			if( pss->data_len <= 7 ) { ASSERT(0); goto FAIL; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = txt_to_vl( pSRecord->topic, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'si=0': // srecord info - shares
			if( rc = ar_util_6Bto12B( &pSRecord->shares, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'si=1': // srecord info - threshold
			if( rc = ar_util_6Bto12B( &pSRecord->threshold, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'si=2': // srecord info - shareid
			if( rc = ar_util_6Bto12B( &pSRecord->shareid, pss->data_first ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'sh=0': // share
			if( rc = txt_to_vl( pSRecord->share, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ss=0': // <asharesig> - r
			if( rc = txt_to_vl( pSRecord->sharesig.r, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'ss=1': // <sharesig> - s
			if( rc = txt_to_vl( pSRecord->sharesig.s, pss->data_first, pss->data_len ) ) { ASSERT(0); goto FAIL; }
			break;
		case 'sc=0': // <shareclue>
			pClue = pss->data_first;
			uClue = pss->data_len;
			break;
		}
	}

	// assemble order-dependent text body

	if( uLocation + uClue > pSRecord->bufmax ) { ASSERT(0); goto FAIL; }

	byteptr bufloc = pSRecord->buf;
	size_t buflen = 0;

	if( uLocation == 0 ) { ASSERT(0); goto FAIL; }
	memcpy_s( bufloc, pSRecord->bufmax, pLocation, uLocation );
	pSRecord->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_6BAto8BA( &deltalen, bufloc, pSRecord->bufmax - buflen, pClue, uClue ) ) { ASSERT(0); goto FAIL; }
		bufloc += deltalen;
		buflen += deltalen;
	}

	pSRecord->bufused = (word16)(buflen);

FAIL:

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

	arAuth80		arecord_;
	arShare80		srecords_[2];
	arShare80ptr	srecordarr_[2] = { &srecords_[0], &srecords_[1] };

	char* clues_r[3] = {"topiclue", "clue1", "clue2"};
	char* location = "foo.bar";

	char cleartextin[20];
	char cleartextout[80];

	typedef byte byte2048[2048];

	byte2048 bufa, bufs0, bufs1;
	word16 shares, threshold;

	int numtests = 100;
	for( int i=0; i<numtests; i++ )
	{
		int rc = 0;

		for( int j=0; j<20; j++ ) { cleartextin[j] = (char)(ar_util_rnd32() % (122 - 32) + 32); }
		cleartextin[ ar_util_rnd32() % 20 ] = 0;

		// make clue optional
		char* clues_rw[3] = {clues_r[0], clues_r[1], clues_r[2]};
		if( ar_util_rnd32() % 9 > 5 ) {	clues_rw[ ar_util_rnd32() % 3 ] = ""; }
		if( ar_util_rnd32() % 9 > 5 ) {	clues_rw[ ar_util_rnd32() % 3 ] = ""; }

		// clear
		{
			memset( &bufa, 0, sizeof(byte2048) );
			memset( &bufs0, 0, sizeof(byte2048) );
			memset( &bufs1, 0, sizeof(byte2048) );

			memset( &arecord, 0, sizeof(arAuth80) );
			memset( &srecords[0], 0, sizeof(arShare80) );
			memset( &srecords[1], 0, sizeof(arShare80) );
			arecord.x.bufmax = srecords[0].x.bufmax = srecords[1].x.bufmax = 80;

			memset( &arecord_, 0, sizeof(arAuth80) );
			memset( &srecords_[0], 0, sizeof(arShare80) );
			memset( &srecords_[1], 0, sizeof(arShare80) );
			arecord_.x.bufmax = srecords_[0].x.bufmax = srecords_[1].x.bufmax = 80;
		}

		rc = ar_core_create( &arecord.x, (arShareptr*)srecordarr, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1), (byteptr*)clues_rw, location ); // +1 to include \0
		ASSERT( rc == 0 );

		bufa[0] = 0;
		rc = ar_uri_create_a( bufa, 2048, &arecord.x );
		ASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufa );
			ASSERT( rc == 0 );
			ASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufa );
			ASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				ASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			ASSERT( strcmp( clue, clues_rw[0] ) == 0 );
		}

		bufs0[0] = 0;
		rc = ar_uri_create_s( bufs0, 2048, &srecords[0].x );
		ASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufs0 );
			ASSERT( rc == 0 );
			ASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufs0 );
			ASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				ASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			ASSERT( strcmp( clue, clues_rw[1] ) == 0 );
		}

		bufs1[0] = 0;
		rc = ar_uri_create_s( bufs1, 2048, &srecords[1].x );
		ASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufs1 );
			ASSERT( rc == 0 );
			ASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufs1 );
			ASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				ASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			ASSERT( strcmp( clue, clues_rw[2] ) == 0 );
		}

		if(0)
		{
			printf("%s\n",bufa);
			printf("%s\n",bufs0);
			printf("%s\n",bufs1);
		}

		rc = ar_uri_parse_a( &arecord_.x, bufa, location );
		ASSERT( rc == 0 );

		rc = ar_uri_parse_shareinfo( &shares, &threshold, bufa );
		ASSERT( rc == 0 );
		ASSERT( shares == 2 );
		ASSERT( threshold == 2 );

		rc = ar_uri_parse_s( &srecords_[0].x, bufs0, location );
		ASSERT( rc == 0 );

		rc = ar_uri_parse_shareinfo( &shares, &threshold, bufs0 );
		ASSERT( rc == 0 );
		ASSERT( shares == 2 );
		ASSERT( threshold == 2 );

		rc = ar_uri_parse_s( &srecords_[1].x, bufs1, location );
		ASSERT( rc == 0 );

		rc = ar_uri_parse_shareinfo( &shares, &threshold, bufs1 );
		ASSERT( rc == 0 );
		ASSERT( shares == 2 );
		ASSERT( threshold == 2 );

		cleartextout[0]=0;
		rc = ar_core_decrypt( cleartextout, 80, &arecord_.x, (arShareptr*)srecordarr_, 2 );
		ASSERT( rc == 0 );

		ASSERT( strcmp( cleartextin, cleartextout ) == 0 );

		if(i > 0 &&  i % 10 == 0 ) { printf("%d",9 - i / (numtests / 9)); }
	}
	putchar('\n');

#endif

}
