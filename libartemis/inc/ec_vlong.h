#ifndef __EC_VLONG_H
#define __EC_VLONG_H

#include "ec_types.h"
#include "ec_param.h"

// vlAdd seems to propagate carry from start to end of array, so vlPoint is big endian format, i think
typedef word16 vlunit;
typedef vlunit vlPoint [VL_UNITS +1 ]; // +1 for length

#define vlIsValid(p) (p[0] <= VL_UNITS)
#define vlIsZero(p) (p[0] == 0)

#define DEBUGPRINT_V(s,v) \
	do { DEBUGPRINT(s); for( int j=0; j< (v)[0]; j++ ) { DEBUGPRINT("%04X", (v)[j+1]); } DEBUGPRINT("\n"); } while(0)

void vlClear (vlPoint p);

vlunit vlGetUnit(vlPoint p, word16 i);
void vlSetUnit(vlPoint p, vlunit u);

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
