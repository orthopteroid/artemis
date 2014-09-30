// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_TYPES_H_
#define _AR_TYPES_H_

#include "ec_types.h"
#include "ec_vlong.h"
#include "ec_field.h"
#include "ec_crypt.h"

typedef struct arAuth_
{
	vlPoint		topic;			// digest of cryptext
	word16		shares;			// 12 bits
	word16		threshold;		// 12 bits
	word16		fieldsize;		// 12 bits
	//
	vlPoint		pubkey;
	cpPair		authsig;
	vlPoint		verify;			// digest of message cleartext
	//
	word16		bufmax;
	word16		loclen;			// location part of http url: domain/path/object
	word16		cluelen;		// optional plaintext clue
	word16		msglen;			// size of message part
	byte		buf[1];			// location [ clue ] message
} arAuth;

typedef arAuth*		arAuthptr;

typedef struct arShare_
{
	vlPoint		topic;			// digest of cryptext
	word16		shares;			// 12 bits
	word16		threshold;		// 12 bits
	word16		shareid;		// 12 bits
	//
	vlPoint		share;
	cpPair		sharesig;
	//
	word16		bufmax;
	word16		loclen;			// location part of http url: domain/path/object
	word16		cluelen;		// optional plaintext clue
	byte		buf[1];			// location [ clue ]
} arShare;

typedef arShare*	arShareptr;
typedef arShareptr*	arSharetbl;

#endif // _AR_TYPES_H_
