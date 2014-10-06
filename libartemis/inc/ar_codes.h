// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CODES_H_
#define _AR_CODES_H_

#define ISOK(c)			((c)>=0)

#undef RC_XXX
#define RC_XXX(a,b) a = b,

enum {

#include "ar_codes.x"

};

#endif // _AR_CODES_H_