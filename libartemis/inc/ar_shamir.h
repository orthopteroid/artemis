// Implementation, Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_SHAMIR_H_
#define _AR_SHAMIR_H_

#include "sha1.h"
#include "ec_field.h"
#include "ec_vlong.h"
#include "ec_crypt.h"

void ar_shamir_splitsecret( gfPoint* shareArr, word16* shareIDArr, word16 numShares, gfPoint* gfCryptCoefArr, word16 numThres );
	/* Create numbered shares from a key */

void ar_shamir_recoversecret( gfPoint key, word16* shareIDArr, gfPoint* shareArr, word16 numShares );
	/* Calc key from numbered shares */

void ar_shamir_test();

#endif // _AR_SHAMIR_H_
