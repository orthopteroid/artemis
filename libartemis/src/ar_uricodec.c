// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include <string.h>
#include <stdlib.h>

#include "platform.h"
#include "ar_codes.h"

#include "ar_uricodec.h"
#include "ec_vlong.h"
#include "ar_util.h"
#include "version.h"

#if defined(ENABLE_TESTS)
	#include "ar_core.h" // for testing
#endif

#define DELIM_INIT( f, t ) { (f) ? '?' : '&', 0xff & ((t) >> 24), 0xff & ((t) >> 16), 0xff & ((t) >> 8), 0 }

//////////////////////

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
	int rc = 0;

	if( !pps ) { rc = RC_INTERNAL; LOGFAIL( rc ); return; }
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
	int rc = 0;

	int bPsuedoTag = ( pps->curr == pps->buf_first );

	if( pps->curr > pps->buf_last ) { return pps->tagID = 0; } // no more

	///////////////////////
	// find next tag

	ps2_scan_private( pps );
	if( pps->curr[0] == '\n' ) { pps->curr++; return pps->tagID = '\n\0\0\0'; } // end of line
	if( pps->curr > pps->buf_last ) { rc = RC_ARG; LOGFAIL( rc ); goto FINI; } // found buffer end without finding a tag
	if( bPsuedoTag && pps->curr[0] != '?' ) { rc = RC_ARG; LOGFAIL( rc ); goto FINI; } // first delim not '?'

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
	else { rc = RC_ARG; LOGFAIL( rc ); goto FINI; } // check for invalid scan termination

	///////////////////////
	// scan for data

	if( bPsuedoTag )
	{
		if( pps->curr[0] != '?' ) { rc = RC_ARG; LOGFAIL( rc ); goto FINI; }
		pps->data_item = 0;
		pps->data_first = pps->buf_first; // data for tag starts at start of buf
		pps->data_last = pps->curr - 1;
	}
	else
	{
		if( pps->data_last == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto FINI; }

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

static int ar_uri_info_parse( word16ptr pshares, byteptr pthresh, byteptr pvers, byteptr pdata )
{
	int rc = 0;

	if( !pshares || !pthresh || !pvers ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	word32 blob = 0;
	if( rc = ar_util_6BAto30B( &blob, pdata ) ) { LOGFAIL( rc ); goto EXIT; }
	*pvers = ( blob >> 24 ) & 63;
	*pshares = ( blob >> 8 ) & 0xFFFF;
	*pthresh = ( blob ) & 0x00FF;

EXIT:

	return rc;
}

static int ar_uri_info_cat( byteptr buf, size_t bufsize, word16 shares, byte threshold, byte vers )
{
	int rc = 0;

	if( !buf ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	char sz[5+1] = {0,0,0,0,0,0}; // 5b64chars = 30 bits, +1 for \0 term

	word32 blob = 0;
	blob |= ( vers & 63 ) << 24;
	blob |= ( shares & 0xFFFF ) << 8;
	blob |= ( threshold & 0x00FF );
	if( rc = ar_util_30Bto6BA( sz, blob ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_strcat( buf, bufsize, sz ) ) { LOGFAIL( rc ); goto EXIT; }

EXIT:

	return rc;
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
	
	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

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

EXIT:

	return rc;	
}

int ar_uri_locate_topic( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord )
{
	int rc = 0;
	
	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

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

EXIT:

	return rc;	
}

int ar_uri_locate_location( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord )
{
	int rc = 0;
	
	if( !ppFirst ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !ppLast ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( strlen( szRecord ) < 10 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	*ppFirst = *ppLast = 0;

	byteptr s, e;
	s = e = 0;
	s = strstr( szRecord, "http://" );
	if( s == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	s += 7;
	e = strchr( s, '?' );
	if( e == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	e--;

	if( s > e ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	*ppFirst = s;
	*ppLast = e;

EXIT:

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

int ar_uri_parse_info( word16ptr pType, word16ptr pShares, byteptr pThreshold, byteptr szRecord )
{
	int rc = 0;

	if( !pType ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !pShares ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !pThreshold ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	*pType = *pShares = *pThreshold = 0;

	parsestate2 ss;
	parsestate2* pss = &ss;
	ps2_init( pss, szRecord, strlen( szRecord ) );

	word32 token;
	while( token = ps2_token( pss ) )
	{
		if( token == 'ai=0' )		{ *pType = 1; break; }
		else if( token == 'si=0' )	{ *pType = 2; break; }
	}
	if( !(*pType) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	byte vers = 0;
	if( rc = ar_uri_info_parse( pShares, pThreshold, &vers, pss->data_first ) ) { LOGFAIL( rc ); goto EXIT; }
	if( vers != AR_VERSION ) { rc = RC_VERSION; LOGFAIL( rc ); goto EXIT; }

EXIT:

	return rc;
}

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord )
{
	int rc = 0;

	if( !buf ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !pARecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	size_t msgoffset = pARecord->loclen + pARecord->cluelen;

	size_t buflen = 0;
	char sz[3] = {0,0,0};

	size_t tokenlen = 0;
	word32 stateArr[] = {'tp=\0', 'ai=\0', 'vf=\0', 'pk=\0', 'as=\0', 'mt=\0', 'mc=\0', 0 };

#if defined(ENABLE_TESTS)

	// impose some robustness on the order of the tags....
	int k = sizeof(stateArr) / sizeof(word32) -1 -1; // -1 conv from size to count, -1 to omit EOA marker
	for( int i = 0; i < k; i++ )
	{
		int j = ar_util_rnd32() % k;
		word32 t = stateArr[j];
		stateArr[j] = stateArr[i]; stateArr[i] = t;
	}

#endif

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_strncat( buf, bufsize, pARecord->buf, pARecord->loclen ) ) { LOGFAIL( rc ); goto EXIT; }

	word32 state = 0;
	while( stateArr[ state ] )
	{
		// no clue, skip
		if( stateArr[ state ] == 'mc=\0' && pARecord->cluelen == 0 ) { state++; continue; }

		char delim[5] = DELIM_INIT( state == 0, stateArr[ state ] );

		if( rc = ar_util_strcat( buf, bufsize, delim ) ) { LOGFAIL( rc ); goto EXIT; }

		switch( stateArr[ state ] )
		{
		default: rc = RC_ARG; LOGFAIL( rc ); break;
		case 'tp=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pARecord->topic ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'ai=\0':
			if( rc = ar_uri_info_cat( buf, bufsize, pARecord->shares, pARecord->threshold, AR_VERSION ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'vf=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pARecord->verify ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'pk=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pARecord->pubkey ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'as=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pARecord->authsig.r ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_vl2txt( buf, bufsize, pARecord->authsig.s ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'mc=\0':
			if( pARecord->cluelen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
			buflen = strlen(buf);
			rc = ar_util_u8_b64encode( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + pARecord->loclen, pARecord->cluelen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'mt=\0':
			buflen = strlen(buf);
			rc = ar_util_u8_b64encode( &tokenlen, buf + buflen, bufsize - buflen, pARecord->buf + msgoffset, pARecord->msglen );
			if( rc == 0 ) { buf[ tokenlen + buflen ] = 0; } else { LOGFAIL( rc ); goto EXIT; }
			break;
		}
		state++;
	}

EXIT:

	if( rc && buf ) { memset( buf, 0, bufsize ); }

	return rc;
}

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord )
{
	int rc = 0;

	size_t buflen = 0;
	size_t tokenlen = 0;
	word32 stateArr[] = {'tp=\0', 'si=\0', 'sh=\0', 'ss=\0', 'sc=\0', 0 };

#if defined(ENABLE_TESTS)

	// impose some robustness on the order of the tags....
	int k = sizeof(stateArr) / sizeof(word32) -1 -1; // -1 conv from size to count, -1 to omit EOA marker
	for( int i = 0; i < k; i++ )
	{
		int j = ar_util_rnd32() % k;
		word32 t = stateArr[j];
		stateArr[j] = stateArr[i]; stateArr[i] = t;
	}

#endif

	if( rc = ar_util_strcat( buf, bufsize, "http://" ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ar_util_strncat( buf, bufsize, pSRecord->buf, pSRecord->loclen ) ) { LOGFAIL( rc ); goto EXIT; }

	word32 state = 0;
	while( stateArr[ state ] )
	{
		// no clue, skip
		if( stateArr[ state ] == 'sc=\0' && pSRecord->cluelen == 0 ) { state++; continue; }

		char delim[5] = DELIM_INIT( state == 0, stateArr[ state ] );

		if( rc = ar_util_strcat( buf, bufsize, delim ) ) { LOGFAIL( rc ); goto EXIT; }

		switch( stateArr[ state ] )
		{
		default: rc = RC_ARG; LOGFAIL( rc ); break;
		case 'tp=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pSRecord->topic ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'si=\0':
			if( rc = ar_uri_info_cat( buf, bufsize, pSRecord->shares, pSRecord->threshold, AR_VERSION ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'sh=\0':
			if( rc = ar_util_w16totxt( buf, bufsize, &(pSRecord->shareid) ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_vl2txt( buf, bufsize, pSRecord->share ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'ss=\0':
			if( rc = ar_util_vl2txt( buf, bufsize, pSRecord->sharesig.r ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_strcat( buf, bufsize, "!" ) ) { LOGFAIL( rc ); goto EXIT; }
			if( rc = ar_util_vl2txt( buf, bufsize, pSRecord->sharesig.s ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'sc=\0':
			if( pSRecord->cluelen == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
			buflen = strlen(buf);
			if( rc = ar_util_u8_b64encode( &tokenlen, buf + buflen, bufsize - buflen, pSRecord->buf + pSRecord->loclen, pSRecord->cluelen ) ) { LOGFAIL( rc ); goto EXIT; }
			buf[ tokenlen + buflen ] = 0;
			break;
		}
		state++;
	}

EXIT:

	if( rc && buf ) { memset( buf, 0, bufsize ); }

	return rc;
}


int ar_uri_parse_a( arAuthptr* arecord_out, byteptr szRecord )
{
	int rc = 0;

	if( !arecord_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*arecord_out = 0;

	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( strlen( szRecord ) < 10 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	size_t bufsize = 0;
	ar_uri_parse_vardatalen( &bufsize, szRecord );
	size_t structsize = sizeof(arAuth) + bufsize;
	if( !((*arecord_out) = malloc( structsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( (*arecord_out), 0, structsize );

	(*arecord_out)->bufmax = bufsize;

	// cached, as order is important when bundling
	size_t uLocation = 0, uClue = 0, uMessage = 0;
	byteptr pLocation = 0, pClue = 0, pMessage = 0;

	// vl conversion vars
	size_t deltalen = 0;
	char tmp[ sizeof(vlPoint) + 2 ] = {0};
	const word16 VL_WORD_COUNT = (word16)(sizeof(vlPoint)/sizeof(word16) - 1);

	byte vers = 0;

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
			if( pss->data_len <= 7 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = ar_util_txt2vl( (*arecord_out)->topic, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'ai=0': // arecord info - version | shares | threshold
			if( rc = ar_uri_info_parse( &(*arecord_out)->shares, &(*arecord_out)->threshold, &vers, pss->data_first ) ) { LOGFAIL( rc ); goto EXIT; }
			if( vers != AR_VERSION ) { rc = RC_VERSION; LOGFAIL( rc ); goto EXIT; }
			break;
		case 'vf=0': // <verify>
			if( rc = ar_util_txt2vl( (*arecord_out)->verify, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'pk=0': // <pubkey>
			if( rc = ar_util_txt2vl( (*arecord_out)->pubkey, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'as=0': // <authsig> - r
			if( rc = ar_util_txt2vl( (*arecord_out)->authsig.r, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'as=1': // <authsig> - s
			if( rc = ar_util_txt2vl( (*arecord_out)->authsig.s, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
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

	if( uLocation == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( uMessage == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( uLocation + uClue + uMessage > (*arecord_out)->bufmax ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); goto EXIT; }

	byteptr bufloc = (*arecord_out)->buf;
	size_t buflen = 0;

	memcpy_s( bufloc, (*arecord_out)->bufmax, pLocation, uLocation );
	(*arecord_out)->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_u8_b64decode( &deltalen, bufloc, (*arecord_out)->bufmax - buflen, pClue, uClue ) ) { LOGFAIL( rc ); goto EXIT; }
		(*arecord_out)->cluelen = (word16)(deltalen);
		bufloc += deltalen;
		buflen += deltalen;
	}

	if( rc = ar_util_u8_b64decode( &deltalen, bufloc, (*arecord_out)->bufmax - buflen, pMessage, uMessage ) ) { LOGFAIL( rc ); goto EXIT; }
	(*arecord_out)->msglen = (word16)(deltalen);

EXIT:

	if( rc && arecord_out && *arecord_out ) { free(*arecord_out); *arecord_out = 0; }

	return rc;
}

int ar_uri_parse_s( arShareptr* srecord_out, byteptr szRecord )
{
	int rc = 0;

	if( !srecord_out ) { rc = RC_NULL; LOGFAIL( rc ); return rc; }

	*srecord_out = 0;

	if( !szRecord ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	if( strlen( szRecord ) < 10 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( !ar_util_isvalid7bit( szRecord ) ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }

	size_t bufsize = 0;
	ar_uri_parse_vardatalen( &bufsize, szRecord );
	size_t structsize = sizeof(arAuth) + bufsize;
	if( !((*srecord_out) = malloc( structsize )) ) { rc = RC_MALLOC; LOGFAIL( rc ); goto EXIT; }
	memset( (*srecord_out), 0, structsize );

	(*srecord_out)->bufmax = bufsize;

	// cached, as order is important when bundling
	size_t uLocation = 0, uClue = 0;
	byteptr pLocation = 0, pClue = 0;

	byte vers = 0;

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
			if( pss->data_len <= 7 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
			pLocation = pss->data_first + 7; // 7 to remove 'http://'
			uLocation = pss->data_len - 7; // 7 to remove 'http://'
			break;
		case 'tp=0': // topic
			if( rc = ar_util_txt2vl( (*srecord_out)->topic, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'si=0': // srecord info - version | shares | threshold
			if( rc = ar_uri_info_parse( &(*srecord_out)->shares, &(*srecord_out)->threshold, &vers, pss->data_first ) ) { LOGFAIL( rc ); goto EXIT; }
			if( vers != AR_VERSION ) { rc = RC_VERSION; LOGFAIL( rc ); goto EXIT; }
			break;
		case 'sh=0': // share - shareid
			if( rc = ar_util_txttow16( &(*srecord_out)->shareid, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
		case 'sh=1': // share
			if( rc = ar_util_txt2vl( (*srecord_out)->share, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'ss=0': // <asharesig> - r
			if( rc = ar_util_txt2vl( (*srecord_out)->sharesig.r, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'ss=1': // <sharesig> - s
			if( rc = ar_util_txt2vl( (*srecord_out)->sharesig.s, pss->data_first, pss->data_len ) ) { LOGFAIL( rc ); goto EXIT; }
			break;
		case 'sc=0': // <shareclue>
			pClue = pss->data_first;
			uClue = pss->data_len;
			break;
		}
	}

	// assemble order-dependent text body

	if( uLocation == 0 ) { rc = RC_ARG; LOGFAIL( rc ); goto EXIT; }
	if( uLocation + uClue > (*srecord_out)->bufmax ) { rc = RC_BUFOVERFLOW; LOGFAIL( rc ); goto EXIT; }

	byteptr bufloc = (*srecord_out)->buf;
	size_t buflen = 0;

	memcpy_s( bufloc, (*srecord_out)->bufmax, pLocation, uLocation );
	(*srecord_out)->loclen = (word16)(uLocation);
	bufloc += uLocation;
	buflen += uLocation;

	if( uClue > 0 ) // optional
	{
		if( rc = ar_util_u8_b64decode( &deltalen, bufloc, (*srecord_out)->bufmax - buflen, pClue, uClue ) ) { LOGFAIL( rc ); goto EXIT; }
		(*srecord_out)->cluelen = (word16)(deltalen);
	}

EXIT:

	if( rc && srecord_out && *srecord_out ) { free(*srecord_out); *srecord_out = 0; }

	return rc;
}

void ar_uri_test()
{

#if defined(ENABLE_TESTS)

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

	char* clues_r[4] = {"topiclue", "clue1", "clue2", 0}; // is a table, ends with 0
	char* location = AR_LOCATION;

	char cleartextin[20];
	byteptr cleartext_out;

	typedef byte byte2048[2048];

	byte2048 bufa, bufs0, bufs1;
	word16 stype, shares;
	byte threshold;

#if defined(ENABLE_FUZZING)

	int numi = 500;
	for( int i=0; i<numi; i++ )

#else

	int numi = 100;
	for( int i=0; i<numi; i++ )

#endif

	{
		int rc = 0;

		for( int j=0; j<20; j++ ) { cleartextin[j] = (char)(ar_util_rnd32() % (122 - 32) + 32); }
		cleartextin[ ar_util_rnd32() % 20 ] = 0;

		// make clue optional
		char* clues_rw[4] = {clues_r[0], clues_r[1], clues_r[2], 0}; // table ends in 0
		if( ar_util_rnd32() % 9 > 5 ) {	clues_rw[ ar_util_rnd32() % 3 ] = ""; }
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
				rc = ar_util_u8_b64decode( &cluelen, clue, 80, s, e-s+1 );
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
				rc = ar_util_u8_b64decode( &cluelen, clue, 80, s, e-s+1 );
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
				rc = ar_util_u8_b64decode( &cluelen, clue, 80, s, e-s+1 );
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

		rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares );
		TESTASSERT( rc == 0 );

		TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );

		if( cleartext_out ) free( cleartext_out );

		free( arecord );
		for( size_t i = 0; i < 2; i++ ) { free( srecordtbl[i] ); }
		free( srecordtbl );

#if defined(ENABLE_FUZZING)

		{
			// test failure with too few shares
			rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares -1 );
			TESTASSERT( rc != 0 );
			if( cleartext_out ) free( cleartext_out );
		}

		{
			// test failure from uri corruption
			byteptr buf[2];
			buf[0]=(byteptr)strdup( bufa );
			buf[1]=(byteptr)strdup( bufs0 );

			for( int j=0; j<400; j++)
			{
				free( arecord_ );
				for( size_t i = 0; i < 2; i++ ) { free( srecordtbl_[i] ); srecordtbl_[i] = 0; }

				byteptr p = buf[ ar_util_rnd32() & 0x01 ];
				size_t ii = ar_util_rnd32() % strlen( p );
				{
					byte bb = p[ii];
					p[ii] = 32 + (ar_util_rnd32() % (128 - 32)); // dirty without non-printables

					rc = ar_uri_parse_a( &arecord_, buf[0] );
					if( !rc ) { rc = ar_uri_parse_s( &srecordtbl_[0], buf[1] ); }
					if( !rc ) { rc = ar_uri_parse_s( &srecordtbl_[1], bufs1 ); }
					if( !rc ) { rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares ); }
					if( !rc )
					{
						// sometimes there won't be failures, because a delim may be replaced with another delim for instance
						TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );
					}
					if( cleartext_out ) { free( cleartext_out ); cleartext_out = 0; }

					p[ii] = bb; // clean
				}
			}
			free( buf[0] );
			free( buf[1] );
		}

		{
			// test failure from uri streching
			byteptr bufa_stretch=(byteptr)malloc( strlen( bufa ) +1 +1 ); // +1 for extra char, +1 for /0

			for( int j=0; j<400; j++)
			{
				free( arecord_ );
				for( size_t i = 0; i < 2; i++ ) { free( srecordtbl_[i] ); srecordtbl_[i] = 0; }

				size_t ii = ar_util_rnd32() % strlen( bufa );
				{
					strncpy_s( bufa_stretch, strlen( bufa ) + 1, bufa, ii );
					bufa_stretch[ii] = 32 + (ar_util_rnd32() % (128 - 32)); // dirty without non-printables
					strncpy_s( bufa_stretch +ii +1, strlen( bufa ) -ii, bufa +ii, strlen( bufa ) -1 -ii );

					rc = ar_uri_parse_a( &arecord_, bufa_stretch );
					if( !rc ) { rc = ar_uri_parse_s( &srecordtbl_[0], bufs0 ); }
					if( !rc ) { rc = ar_uri_parse_s( &srecordtbl_[1], bufs1 ); }
					if( !rc ) { rc = ar_core_decrypt( &cleartext_out, arecord_, srecordtbl_, shares ); }
					if( !rc )
					{
						// sometimes there won't be failures, because a delim may be replaced with another delim for instance
						TESTASSERT( strcmp( cleartextin, cleartext_out ) == 0 );
					}
					if( cleartext_out ) { free( cleartext_out ); cleartext_out = 0; }
				}
			}
			free( bufa_stretch );
		}

#endif // ENABLE_FUZZING

		free( arecord_ );
		for( size_t i = 0; i < 2; i++ ) { free( srecordtbl_[i] ); }
		free( srecordtbl_ );

		if( i%20==0) { DEBUGPRINT( "uri%d ", numi / 20 - i / 20 ); }
	}
	DEBUGPRINT("\n");

#endif

}
