#ifndef _RC4_H_
#define _RC4_H_

#include "ec_types.h"

void rc4( byteptr key, int keylen, int skip, byteptr txt, int txtlen );

#endif // _RC4_H_
