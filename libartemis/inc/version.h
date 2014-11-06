// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "ec_param.h"

// units are 16-bit words
#define AR_UNITS ((GF_K*GF_L + 15)/16 + 1)

// fixed length
#define AR_VERIFYUNITS		2
#define AR_TOPICUNITS		4
#define AR_MACUNITS			4

// length adjusted to prevent saturation/overflow
#define AR_SESSIONUNITS		AR_UNITS-2
#define AR_SHARECOEFUNITS	AR_UNITS-2
#define AR_CRYPTKEYUNITS	AR_UNITS-2
#define AR_SIGNKEYUNITS		AR_UNITS-4

// some sanity checks
STATICASSERT( AR_VERIFYUNITS < AR_UNITS );
STATICASSERT( AR_TOPICUNITS < AR_UNITS );
STATICASSERT( AR_MACUNITS < AR_UNITS );
STATICASSERT( AR_SHARECOEFUNITS < AR_UNITS );
STATICASSERT( AR_SESSIONUNITS < AR_UNITS );
STATICASSERT( AR_CRYPTKEYUNITS < AR_UNITS );
STATICASSERT( AR_SIGNKEYUNITS < AR_UNITS );

#define AR_VERSION 1
#define AR_DEMO 1
