// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CODES_H_
#define _AR_CODES_H_

// return codes are signed
// -ve codes are errors, +ve codes can be useful values.

#define ISOK(c)			((c)>=0)
#define RC_OK			0
#define RC_INTERNAL		-1
#define RC_NULL			-2
#define RC_MALLOC		-3
#define RC_BUFOVERFLOW	-4
#define RC_ARG			-5
//
#define RC_INSUFFICIENT	-10
#define RC_TOPIC		-11
#define RC_SIGATURE		-12
#define RC_LOCATION		-13
#define RC_VERIFY		-14

#endif // _AR_CODES_H_