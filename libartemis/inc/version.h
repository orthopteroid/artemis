// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "ec_param.h"

// units are 16-bit words
#define AR_UNITS ((GF_K*GF_L + 15)/16 + 1) // 9 units = 144 bits @ 136 gf_m

// storage-bits (fixed length)
#define AR_VERIFYUNITS		2 // 32 bits
#define AR_TOPICUNITS		4 // 64 bits
#define AR_MACUNITS			4 // 64 bits

// entropy-bits (length adjusted to prevent saturation/overflow)
#define AR_SESSIONUNITS		AR_UNITS-2 // 112 bits @ 136 gf_m
#define AR_SHARECOEFUNITS	AR_UNITS-4 // 80 bits @ 136 gf_m
#define AR_CRYPTKEYUNITS	AR_UNITS-2 // 112 bits @ 136 gf_m
#define AR_SIGNKEYUNITS		AR_UNITS-4 // 80 bits @ 136 gf_m

#define AR_LOCATION "tereslogica.com"
#define AR_LOCHASH 0x0D

#define AR_VERSION 1
#define AR_DEMO
