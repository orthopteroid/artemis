// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_TYPES_H_
#define _AR_TYPES_H_

#include "ec_types.h"
#include "ec_vlong.h"
#include "ec_field.h"
#include "ec_crypt.h"

typedef struct arAuth_
{
	word16		fieldsize;		// 12 bits
	word16		shares;			// 12 bits
	word16		threshold;		// 12 bits
	vlPoint		topic;			// high 64 bits of crypt digest
	vlPoint		pubkey;
	cpPair		authsig;
	vlPoint		verify;			// high 32 bits of clear digest
	//
	word16		bufmax;
	word16		bufused;
	byte		buf[1];
} arAuth;

typedef struct arShare_
{
	vlPoint		topic;			// high 64 bits of crypt digest
	word16		shareid;		// bits 11...0
	vlPoint		share;
	cpPair		sharesig;
} arShare;

#endif // _AR_TYPES_H_
