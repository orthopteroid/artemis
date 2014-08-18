// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _ar_uricodec_H_
#define _ar_uricodec_H_

#include "ec_types.h"
#include "ec_field.h"
#include "ec_crypt.h"
#include "ar_types.h"

// The arg 'si' is a triplet made from shares, threshold and fieldsize, which only have their lower 12 bits encoded in the uri scheme.

DLLDECL 
void ar_uri_estimatebufsize( size_t* uribufsize, arAuth* pARecord );
	/* returns number of bytes needed for uri compose buffer */

DLLDECL 
void ar_uri_estimatemessagesize( size_t* uribufsize, byteptr buf );
	/* returns number of bytes needed for ARecord, if buf holds an arecord uri */

DLLDECL 
void ar_uri_estimateshares( word16* shares, byteptr buf );
	/* returns number of shares, if buf holds an arecord uri */

DLLDECL 
int ar_uri_create_a( byteptr buf, size_t bufsize, arAuth* pARecord );

DLLDECL 
int ar_uri_create_s( byteptr buf, size_t bufsize, arShare* pSRecord );

DLLDECL 
int ar_uri_parse( arAuth* pARecord, arShare* pSRecord, byteptr szRecord );
	/* -1 for error, 1 for ARecord, 2 for SRecord */

void ar_uri_test();

#endif // _ar_uricodec_H_
