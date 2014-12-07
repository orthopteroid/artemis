#ifndef __EC_CRYPT_H
#define __EC_CRYPT_H

#include "ec_curve.h"
#include "ec_vlong.h"

typedef struct {
	vlPoint r, s;
} cpPair;

int cpMakePublicKey(vlPoint vlPublicKey, const vlPoint vlPrivateKey);
int cpEncodeSecret(vlPoint vlSecret, vlPoint vlMessage, const vlPoint vlPublicKey);
int cpDecodeSecret(vlPoint d, vlPoint vlMessage, const vlPoint vlPrivateKey);
int cpSign(cpPair * cpSig, const vlPoint vlPrivateKey, const vlPoint secret, const vlPoint mac);
int cpVerify(int* pEqual, cpPair * cpSig, const vlPoint vlPublicKey, const vlPoint vlMac);

void cpCopy( cpPair* pp, const cpPair* pq );
void cpClear( cpPair* p );

#endif /* __EC_CRYPT_H */
