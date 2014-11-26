// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "ec_param.h"

// units are 16-bit words
#define AR_UNITS ((GF_K*GF_L + 15)/16 + 1)

// unit length is 16 bits
// 9 units (144 bits) ie. 136 gfm
// 7 units (112 bits) ie. 91 gfm
// 6 units (92 bits)

#define AR_MACUNITS			4 // 64 bits
#define AR_CRYPTKEYUNITS	AR_UNITS-1
#define AR_SESSKEYUNITS		AR_UNITS-1
#define AR_PRIVKEYUNITS		AR_UNITS-1

#define AR_LOCSTR	"arcanacard.com"
#define AR_LOCHASH	0x1E

#define AR_VERSION 1
#define AR_DEMO
