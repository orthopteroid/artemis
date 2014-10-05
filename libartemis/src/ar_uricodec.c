// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_uricodec.h"
#include "ec_vlong.h"
#include "ar_util.h"

#if defined(_DEBUG)
	#include "ar_core.h" // for testing
#endif

#define DELIM_INIT( f, t ) { (f) ? '?' : '&', 0xff & ((t) >> 24), 0xff & ((t) >> 16), 0xff & ((t) >> 8), 0 }

//////////////////////

static int txt_to_vl( vlPoint v, char* buf, size_t bufsize )
{
	int rc=0;
	word16 words = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);
	vlClear( v );
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	rc = ar_util_6BAto8BA( &deltalen, tmp, sizeof(vlPoint), buf, bufsize ); // TODO: add 'goto' and leave output in known state
	if( !rc ) { rc = ar_util_8BAto16BA( &deltalen, &v[1], words, tmp, deltalen ); }
	if( !rc ) { v[0] = (word16)deltalen; }
	return rc;
}

// concatenates into buf
static int vl_to_txt_cat( char* buf, size_t bufsize, vlPoint v )
{
	int rc=0;
	size_t deltalen = 0;
	size_t buflen = strlen(buf);
	char tmp[ sizeof( vlPoint) + 2 ] = {0};
	rc = ar_util_16BAto8BA( &deltalen, tmp, sizeof(vlPoint), v+1, v[0] ); // TODO: add 'goto' and leave output in known state
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
	if( !pps ) { LOGFAIL; return; }
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
	if( pps->curr > pps->buf_last ) { LOGFAIL; goto FINI; } // found buffer end without finding a tag
	if( bPsuedoTag && pps->curr[0] != '?' ) { LOGFAIL; goto FINI; } // first delim not '?'

	///////////////////////
	// handle tag type

	if( bPsuedoTag )
	{
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
	else { LOGFAIL; goto FINI; } // check for invalid scan termination

	///////////////////////
	// scan for data

	if( bPsuedoTag )
	{
		if( pps->curr[0] != '?' ) { LOGFAIL; goto FINI; }
		pps->data_item = 0;
		pps->data_first = pps->buf_first; // data for tag starts at start of buf
		pps->data_last = pps->curr - 1;
	}
	else
	{
		if( pps->data_last == 0 ) { LOGFAIL; goto FINI; }

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
	size_t bufused = pARecord->msglen + pARecord->loclen + pARecord->cluelen;
	*pUribufsize = ( sizeof(arAuth) + bufused ) * 4 / 3; // crazy overkill + b64 encoding
}

void ar_uri_bufsize_s( size_t* pUribufsize, arShare* pSRecord )
{
	size_t bufused = pSRecord->loclen + pSRecord->cluelen;
	*pUribufsize = ( sizeof(arShare) + bufused ) * 4 / 3; // crazy overkill + b64 encoding
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
	
	if( !ppFirst ) { LOGFAIL; return RC_NULL; }
	if( !ppLast ) { LOGFAIL; return RC_NULL; }
	if( !szRecord ) { LOGFAIL; return RC_NULL; }

	if( strlen( szRecord ) < 10 ) { LOGFAIL; return RC_INSUFFICIENT; }

	*ppFirst = *ppLast = 0;

	byteptr s, e;
	s = e = 0;
	s = strstr( szRecord, "http://" );
	if( s == 0 ) { LOGFAIL; return RC_ARG; }
	s += 7;
	e = strchr( s, '?' );
	if( e == 0 ) { LOGFAIL; return RC_ARG; }
	e--;

	if( s > e ) { LOGFAIL; return RC_ARG; }

	*ppFirst = s;
	*ppLast = e;

	return rc;
}

void ar_uri_parse_vardatalen( size_t* pLen, byteptr buf )
{
	*pLen = 0;

	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, buf, strlen( buf ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'ht=0' || token == 'mc=0' || token == 'sc=0' )		{ *pLen += ss.data_len; } // raw text
		else if( token == 'mt=0' || token == 'st=0' )					{ *pLen += ss.data_len; } // overest the b64 text
	}
}

int ar_uri_parse_info( word16* pType, word16* pShares, word16* pThreshold, byteptr szRecord )
{
	int rc = 0;

	if( !pType ) { LOGFAIL; return RC_NULL; }
	if( !pShares ) { LOGFAIL; return RC_NULL; }
	if( !pThreshold ) { LOGFAIL; return RC_NULL; }
	if( !szRecord ) { LOGFAIL; return RC_NULL; }

	*pType = *pShares = *pThreshold = 0;

	byteptr pShare_start, pThreshold_start;
	pShare_start = pThreshold_start = 0;

	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, szRecord, strlen( szRecord ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'ai=0' )		{ *pType = 1; pShare_start = ss.data_first; }
		else if( token == 'ai=1' )	{             pThreshold_start = ss.data_first; }
		else if( token == 'si=0' )	{ *pType = 2; pShare_start = ss.data_first; }
		else if( token == 'si=1' )	{             pThreshold_start = ss.data_first; }
		if( pShare_start != 0 && pThreshold_start != 0 ) { break; }
	}
	if( pShare_start == 0 && pThreshold_start == 0 ) { LOGFAIL; return RC_ARG; }

	if( rc = ar_util_6Bto12B( pShares, pShare_start ) ) { LOGFAIL; return rc; }
	if( rc = ar_util_6Bto12B( pThreshold, pThreshold_start ) ) { LOGFAIL; return rc; }

	return rc;
}

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord )
{
	int rc = 0;

	if( !buf ) { LOGFAIL; rc = RC_NULL; goto FAIL; }
	if( !pARecord ) { LOGFAIL; rc = RC_NULL; goto FAIL; }

	size_t msgoffset = pARecord->loclen + pARecord->cluelen;

	size_t buflen = 0;
	char sz[3] = {0,0,0};

	size_t tokenlen = 0;
	word32 stateArr[] = {'tp=\0', 'ai=\0', 'vf=\0', 'pk=\0', 'as=\0', 'mt=\0', 'mc=\0', 0 };

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

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { LOGFAIL; goto FAIL; }
	if( rc = ar_util_strncat( buf, bufsize, pARecord->buf, pARecord->loclen ) ) { LOGFAIL; goto FAIL; }

	word32 state = 0;
	while( stateArr[ state ] )
	{
		// no clue, skip
		if( stateArr[ state ] == 'mc=\0' && pARecord->cluelen == 0 ) { state++; continue; }
		//
#if 1
		char delim[5] = DELIM_INIT( state == 0, stateArr[ state ] );
#else
		char delim[5];
		delim[0] = ( state == 0 ) ? '?' : '&';
		delim[1] = 0xff & (stateArr[ state ] >> 24);
		delim[2] = 0xff & (stateArr[ state ] >> 16);
		delim[3] = 0xff & (stateArr[ state ] >> 8);
		delim[4] = 0;
#endif
		if( rc = ar_util_strcat( buf, bufsize, delim ) ) { LOGFAIL; goto FAIL; }
		//
		switch( stateArr[ state ] )
		{
		default:
			LOGFAIL; break;
		case 'tp=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->topic ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ai=\0':
			if( rc = ar_util_12Bto6B( sz, pARecord->shares ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pARecord->threshold ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pARecord->fieldsize ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'vf=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->verify ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'pk=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->pubkey ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'as=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.r ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pARecord->authsig.s ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'mc=\0':
			if( pARecord->cluelen == 0 ) { LOGFAIL; goto FAIL; }
			buflen = strlen(buf);
			rc = ar_util_8BAto6BA( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen, pARecord->cluelen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { LOGFAIL; goto FAIL; }
			break;
		case 'mt=\0':
			buflen = strlen(buf);
			rc = ar_util_8BAto6BA( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + msgoffset, pARecord->msglen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { LOGFAIL; goto FAIL; }
			break;
		}
		state++;
	}

FAIL:

	return rc;
}

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord )
{
	int rc = 0;

	size_t buflen = 0;
	char sz[3] = {0,0,0};

	size_t tokenlen = 0;
	word32 stateArr[] = {'tp=\0', 'si=\0', 'sh=\0', 'ss=\0', 'sc=\0', 0 };

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

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { LOGFAIL; goto FAIL; }
	if( rc = ar_util_strncat( buf, bufsize, pSRecord->buf, pSRecord->loclen ) ) { LOGFAIL; goto FAIL; }

	word32 state = 0;
	while( stateArr[ state ] )
	{
		// no clue, skip
		if( stateArr[ state ] == 'sc=\0' && pSRecord->cluelen == 0 ) { state++; continue; }
		//
#if 1
		char delim[5] = DELIM_INIT( state == 0, stateArr[ state ] );
#else
		char delim[5];
		delim[0] = ( state == 0 ) ? '?' : '&';
		delim[1] = 0xff & (stateArr[ state ] >> 24);
		delim[2] = 0xff & (stateArr[ state ] >> 16);
		delim[3] = 0xff & (stateArr[ state ] >> 8);
		delim[4] = 0;
#endif
		if( rc = ar_util_strcat( buf, bufsize, delim ) ) { LOGFAIL; goto FAIL; }
		//
		switch( stateArr[ state ] )
		{
		default:
			LOGFAIL; break;
		case 'tp=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->topic ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'si=\0':
			if( rc = ar_util_12Bto6B( sz, pSRecord->shares ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pSRecord->threshold ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_12Bto6B( sz, pSRecord->shareid ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'sh=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->share ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ss=\0':
			if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.r ) ) { LOGFAIL; goto FAIL; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL; goto FAIL; }
			if( rc = vl_to_txt_cat( buf, bufsize, pSRecord->sharesig.s ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'sc=\0':
			if( pSRecord->cluelen == 0 ) { LOGFAIL; goto FAIL; }
			buflen = strlen(buf);
			rc = ar_util_8BAto6BA( &tokenlen, buf + buflen, bufsize - buflen, pSRecord->buf + pSRecord->loclen, pSRecord->cluelen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { LOGFAIL; goto FAIL; } // TODO: add 'goto' and leave output in known state
			break;
		}
		state++;
	}

FAIL:

	return rc;
}


int ar_uri_parse_a( arAuthptr* arecord_out, byteptr szRecord )
{
	int rc = 0;

	if( !arecord_out ) { LOGFAIL; return RC_NULL; }

	*arecord_out = 0;

	if( !szRecord ) { LOGFAIL; return RC_NULL; }

	if( strlen( szRecord ) < 10 ) { LOGFAIL; return RC_INSUFFICIENT; }

	size_t bufsize = 0;
	ar_uri_parse_vardatalen( &bufsize, szRecord );
	size_t structsize = sizeof(arAuth) + bufsize;
	if( !((*arecord_out) = malloc( structsize )) ) { LOGFAIL; rc = RC_MALLOC; goto FAIL; }
	memset( (*arecord_out), 0, structsize );

	(*arecord_out)->bufmax = bufsize;

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
			if( pss->data_len <= 7 ) { LOGFAIL; goto FAIL; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = txt_to_vl( (*arecord_out)->topic, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ai=0': // arecord info - shares
			if( rc = ar_util_6Bto12B( &(*arecord_out)->shares, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ai=1': // arecord info - threshold
			if( rc = ar_util_6Bto12B( &(*arecord_out)->threshold, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ai=2': // arecord info - fieldsize
			if( rc = ar_util_6Bto12B( &(*arecord_out)->fieldsize, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'vf=0': // <verify>
			if( rc = txt_to_vl( (*arecord_out)->verify, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'pk=0': // <pubkey>
			if( rc = txt_to_vl( (*arecord_out)->pubkey, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'as=0': // <authsig> - r
			if( rc = txt_to_vl( (*arecord_out)->authsig.r, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'as=1': // <authsig> - s
			if( rc = txt_to_vl( (*arecord_out)->authsig.s, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
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

	if( uLocation + uClue + uMessage > (*arecord_out)->bufmax ) { LOGFAIL; goto FAIL; }

	byteptr bufloc = (*arecord_out)->buf;
	size_t buflen = 0;

	if( uLocation == 0 ) { LOGFAIL; goto FAIL; }
	memcpy_s( bufloc, (*arecord_out)->bufmax, pLocation, uLocation );
	(*arecord_out)->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_6BAto8BA( &deltalen, bufloc, (*arecord_out)->bufmax - buflen, pClue, uClue ) ) { LOGFAIL; goto FAIL; }
		(*arecord_out)->cluelen = (word16)(deltalen);
		bufloc += deltalen;
		buflen += deltalen;
	}

	if( uMessage == 0 ) { LOGFAIL; goto FAIL; }
	if( rc = ar_util_6BAto8BA( &deltalen, bufloc, (*arecord_out)->bufmax - buflen, pMessage, uMessage ) ) { LOGFAIL; goto FAIL; }
	(*arecord_out)->msglen = (word16)(deltalen);

FAIL:

	return rc;
}

int ar_uri_parse_s( arShareptr* srecord_out, byteptr szRecord )
{
	int rc = 0;

	if( !srecord_out ) { LOGFAIL; return RC_NULL; }

	*srecord_out = 0;

	if( !szRecord ) { LOGFAIL; return RC_NULL; }

	if( strlen( szRecord ) < 10 ) { LOGFAIL; return RC_INSUFFICIENT; }

	size_t bufsize = 0;
	ar_uri_parse_vardatalen( &bufsize, szRecord );
	size_t structsize = sizeof(arAuth) + bufsize;
	if( !((*srecord_out) = malloc( structsize )) ) { LOGFAIL; rc = RC_MALLOC; goto FAIL; }
	memset( (*srecord_out), 0, structsize );

	(*srecord_out)->bufmax = bufsize;

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
			if( pss->data_len <= 7 ) { LOGFAIL; goto FAIL; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = txt_to_vl( (*srecord_out)->topic, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'si=0': // srecord info - shares
			if( rc = ar_util_6Bto12B( &(*srecord_out)->shares, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'si=1': // srecord info - threshold
			if( rc = ar_util_6Bto12B( &(*srecord_out)->threshold, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'si=2': // srecord info - shareid
			if( rc = ar_util_6Bto12B( &(*srecord_out)->shareid, pss->data_first ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'sh=0': // share
			if( rc = txt_to_vl( (*srecord_out)->share, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ss=0': // <asharesig> - r
			if( rc = txt_to_vl( (*srecord_out)->sharesig.r, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'ss=1': // <sharesig> - s
			if( rc = txt_to_vl( (*srecord_out)->sharesig.s, pss->data_first, pss->data_len ) ) { LOGFAIL; goto FAIL; }
			break;
		case 'sc=0': // <shareclue>
			pClue = pss->data_first;
			uClue = pss->data_len;
			break;
		}
	}

	// assemble order-dependent text body

	if( uLocation + uClue > (*srecord_out)->bufmax ) { LOGFAIL; goto FAIL; }

	byteptr bufloc = (*srecord_out)->buf;
	size_t buflen = 0;

	if( uLocation == 0 ) { LOGFAIL; goto FAIL; }
	memcpy_s( bufloc, (*srecord_out)->bufmax, pLocation, uLocation );
	(*srecord_out)->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_6BAto8BA( &deltalen, bufloc, (*srecord_out)->bufmax - buflen, pClue, uClue ) ) { LOGFAIL; goto FAIL; }
		(*srecord_out)->cluelen = (word16)(deltalen);
	}

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

	arAuthptr		arecord;
	arSharetbl		srecordtbl;

	arAuthptr		arecord_;
	arSharetbl		srecordtbl_;

	char* clues_r[3] = {"topiclue", "clue1", "clue2"};
	char* location = "foo.bar";

	char cleartextin[20];
	byteptr cleartext_out;

	typedef byte byte2048[2048];

	byte2048 bufa, bufs0, bufs1;
	word16 stype, shares, threshold;

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
		memset( &bufa, 0, sizeof(byte2048) );
		memset( &bufs0, 0, sizeof(byte2048) );
		memset( &bufs1, 0, sizeof(byte2048) );

		rc = ar_core_create( &arecord, &srecordtbl, 2, 2, cleartextin, (word16)(strlen(cleartextin) + 1), (byteptr*)clues_rw, location ); // +1 to include \0
		TESTASSERT( rc == 0 );

		bufa[0] = 0;
		rc = ar_uri_create_a( bufa, 2048, arecord );
		TESTASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufa );
			TESTASSERT( rc == 0 );
			TESTASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufa );
			TESTASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				TESTASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			TESTASSERT( strcmp( clue, clues_rw[0] ) == 0 );
		}

		bufs0[0] = 0;
		rc = ar_uri_create_s( bufs0, 2048, srecordtbl[0] );
		TESTASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufs0 );
			TESTASSERT( rc == 0 );
			TESTASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufs0 );
			TESTASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				TESTASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			TESTASSERT( strcmp( clue, clues_rw[1] ) == 0 );
		}

		bufs1[0] = 0;
		rc = ar_uri_create_s( bufs1, 2048, srecordtbl[1] );
		TESTASSERT( rc == 0 );

		{
			byteptr s, e;
			rc = ar_uri_locate_location( &s, &e, bufs1 );
			TESTASSERT( rc == 0 );
			TESTASSERT( strncmp( s, location, e-s+1 ) == 0 );
		}

		{
			byteptr s, e;
			rc = ar_uri_locate_clue( &s, &e, bufs1 );
			TESTASSERT( rc == 0 );

			char clue[80];
			size_t cluelen = 0;
			if( s != e )
			{
				rc = ar_util_6BAto8BA( &cluelen, clue, 80, s, e-s+1 );
				TESTASSERT( rc == 0 );
			}
			clue[ cluelen ] = 0;
			TESTASSERT( strcmp( clue, clues_rw[2] ) == 0 );
		}

		if(0)
		{
			printf("%s\n",bufa);
			printf("%s\n",bufs0);
			printf("%s\n",bufs1);
		}

		rc = ar_uri_parse_a( &arecord_, bufa );
		TESTASSERT( rc == 0 );

		rc = ar_uri_parse_info( &stype, &shares, &threshold, bufa );
		TESTASSERT( rc == 0 );
		TESTASSERT( stype == 1 );
		TESTASSERT( shares == 2 );
		TESTASSERT( threshold == 2 );

		srecordtbl_ = malloc( sizeof(arShare) * shares );

		rc = ar_uri_parse_s( &srecordtbl_[0], bufs0 );
		TESTASSERT( rc == 0 );

		rc = ar_uri_parse_info( &stype, &shares, &threshold, bufs0 );
		TESTASSERT( rc == 0 );
		TESTASSERT( stype == 2 );
		TESTASSERT( shares == 2 );
		TESTASSERT( threshold == 2 );

		rc = ar_uri_parse_s( &srecordtbl_[1], bufs1 );
		TESTASSERT( rc == 0 );

		rc = ar_uri_parse_info( &stype, &shares, &threshold, bufs1 );
		TESTASSERT( rc == 0 );
		TESTASSERT( stype == 2 );
		TESTASSERT( shares == 2 );
		TESTASSERT( threshold == 2 );

		{
			rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares -1 ); // fail
			TESTASSERT( rc != 0 );
			if( cleartext_out ) free( cleartext_out );
		}

		rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares );
		TESTASSERT( rc == 0 );

		TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );

		if( cleartext_out ) free( cleartext_out );

		free( arecord );
		for( size_t i = 0; i < 2; i++ ) { free( srecordtbl[i] ); }
		free( srecordtbl );

		free( arecord_ );
		for( size_t i = 0; i < 2; i++ ) { free( srecordtbl_[i] ); }
		free( srecordtbl_ );
	
		if(i > 0 &&  i % 10 == 0 ) { printf("%d",9 - i / (numtests / 9)); }
	}
	putchar('\n');

#endif

}
