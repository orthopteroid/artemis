// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "ec_param.h"

// units are 16-bit words
#define AR_UNITS ((GF_K*GF_L + 15)/16 + 1) // 9 units = 136 gf_m

// property-length (in 16bit words)
#define AR_VERIFYUNITS		1 // 16 bits
#define AR_TOPICUNITS		4 // 64 bits
#define AR_MACUNITS			4 // 64 bits

#define AR_CRYPTKEYUNITS	AR_UNITS-1 // 8 units @ 136 gf_m
#define AR_SESSKEYUNITS		AR_UNITS-1 // 8 units @ 136 gf_m
#define AR_PRIVKEYUNITS		AR_UNITS-1 // 8 units @ 136 gf_m

#define AR_LOCSTR	"as.tereslogica.com"
#define AR_LOCHASH	0x10

#define AR_VERSION 1
#define AR_DEMO
