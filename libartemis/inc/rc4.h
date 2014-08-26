#ifndef _RC4_H_
#define _RC4_H_

#include "ec_types.h"

void rc4( byteptr key, word32 keylen, word32 skip, byteptr txt, word32 txtlen );

#endif // _RC4_H_
