// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _ar_uricodec_H_
#define _ar_uricodec_H_

#include "ec_types.h"
#include "ec_field.h"
#include "ec_crypt.h"
#include "ar_types.h"

// The arg 'si' is a triplet made from version (6bits), shares (16bits) and threshold(8bits)

void ar_uri_bufsize_a( size_t* pUribufsize, arAuth* pARecord );
	/* returns number of bytes needed for uri compose buffer */

void ar_uri_bufsize_s( size_t* pUribufsize, arShare* pASecord );
	/* returns number of bytes needed for uri compose buffer */

void ar_uri_parse_vardatalen( size_t* pLen, byteptr buf );
	/* returns num of bytes needed to hold all variable length data in buf record */

int ar_uri_parse_info( word16* pType, word16* pShares, byte* pThreshold, byteptr szRecord );

int ar_uri_locate_topic( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord );

int ar_uri_locate_clue( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord );

int ar_uri_locate_location( byteptr* ppFirst, byteptr* ppLast, byteptr szRecord );

int ar_uri_create_a( byteptr buf, byteptr bufend, arAuth* pARecord );

int ar_uri_create_s( byteptr buf, byteptr bufend, arShare* pSRecord );

int ar_uri_parse_a( arAuthptr* arecord_out, byteptr szRecord );

int ar_uri_parse_s( arShareptr* srecord_out, byteptr szRecord );

void ar_uri_test();

#endif // _ar_uricodec_H_
