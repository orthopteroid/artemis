#ifndef __EC_VLONG_H
#define __EC_VLONG_H

#include <stdio.h>

#include "ec_types.h"
#include "ec_param.h"

#define VL_UNITS ((GF_K*GF_L + 15)/16 + 1) /* must be large enough to hold a (packed) curve point (plus one element: the length) */

// little endian format, i think
typedef word16 vlPoint [VL_UNITS + 2];

#define vlIsValid(p) (p[0] < VL_UNITS)

void vlClear (vlPoint p);

int vlIsZero(const vlPoint p);

word16 vlGetWord16(vlPoint p, word16 i);

void vlSetWord16(vlPoint p, word16 u);
void vlSetWord32(vlPoint p, word32 u);
void vlSetWord64(vlPoint p, word32 h, word32 l);
void vlSetWord128(vlPoint p, word32 hh, word32 hl, word32 lh, word32 ll);

void vlSetWord32Ptr( vlPoint p, word16 maxWord16s, word32* q );

int  vlEqual (const vlPoint p, const vlPoint q);

int  vlGreater (const vlPoint p, const vlPoint q);

int  vlNumBits (const vlPoint k);
	/* evaluates to the number of bits of k (index of most significant bit, plus one) */

int  vlTakeBit (const vlPoint k, word16 i);
	/* evaluates to the i-th bit of k */

void vlCopy (vlPoint p, const vlPoint q);
	/* sets p := q */

void vlAdd (vlPoint u, const vlPoint v);

void vlSubtract (vlPoint u, const vlPoint v);

void vlRemainder (vlPoint u, const vlPoint v);

void vlMulMod (vlPoint u, const vlPoint v, const vlPoint w, const vlPoint m);

void vlShortLshift (vlPoint u, int n);

void vlShortRshift (vlPoint u, int n);

int  vlShortMultiply (vlPoint p, const vlPoint q, word16 d);
	/* sets p = q * d, where d is a single digit */

typedef word16 (*rnd16gen)();
void vlSetRandom( vlPoint p, word16 maxWord16s, rnd16gen fn );

int  vlSelfTest (int test_count);

#endif /* __EC_VLONG_H */
