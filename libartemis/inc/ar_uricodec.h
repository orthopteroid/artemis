// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _ar_uricodec_H_
#define _ar_uricodec_H_

#include "ec_types.h"
#include "ec_field.h"
#include "ec_crypt.h"
#include "ar_types.h"

// The arg 'si' is a triplet made from shares, threshold and fieldsize, which only have their lower 12 bits encoded in the uri scheme.

void ar_uri_bufsize_a( size_t* uribufsize, arAuth* pARecord );
	/* returns number of bytes needed for uri compose buffer */

void ar_uri_bufsize_s( size_t* uribufsize, arShare* pASecord );
	/* returns number of bytes needed for uri compose buffer */

void ar_uri_parse_messlen( size_t* len, byteptr buf );
	/* returns num of bytes in message arg, if buf holds an arecord uri */

void ar_uri_parse_cluelen( size_t* len, byteptr buf );
	/* returns num of bytes in clue arg, if buf holds an arecord uri */

void ar_uri_parse_sharecount( word16* shares, byteptr buf );
	/* returns number of shares, if buf holds an arecord uri */

int ar_uri_locate_field( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord, byteptr szField, word16 uFieldNum );

int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord );

int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord );

int ar_uri_parse_type( byteptr szRecord );
	/* -1 for error, 1 for ARecord, 2 for SRecord */

int ar_uri_parse_a( arAuth* pARecord, byteptr szRecord, byteptr location );
	/* -1 for error, -2 for bad record */

int ar_uri_parse_s( arShare* pSRecord, byteptr szRecord, byteptr location );
	/* -1 for error, -2 for bad record */

void ar_uri_test();

#endif // _ar_uricodec_H_
