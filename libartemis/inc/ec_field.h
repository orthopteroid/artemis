#ifndef _EC_FIELD_H_
#define _EC_FIELD_H_

#include "platform.h"
#include "ec_types.h"
#include "sha1.h"
#include "ec_param.h"
#include "ec_vlong.h"

#define GF_UNITS	( ( GF_K +1 ) *2 +1 ) // +1 for ??, *2 for holding square result, +1 for square-overflow

#if GF_L < 8 || GF_L > 16
	#error "this implementation assumes 8 <= GF_L <= 16"
#endif

#if GF_L ==  8
	#define BITS_PER_LUNIT 8
	typedef byte	gfunit;
#else
	#define BITS_PER_LUNIT 16
	typedef word16	gfunit;
#endif

#if GF_L == 16
	typedef word32	ltemp;
#else
	typedef word16	ltemp;
#endif

// little endian format, i think
typedef gfunit gfPoint [ GF_UNITS +1 ]; // +1 for length

#define GF_BYTES (GF_UNITS * sizeof(gfunit))
#define gfIsValid(p) (p[0] <= GF_UNITS)
#define gfRoomForSquare(p) ((p[0]*2-1) <= GF_UNITS)

/* interface functions: */

int  gfInit (void);
	/* initialize the library ---> MUST be called before any other gf-function */

void gfQuit (void);
	/* perform housekeeping for library termination */

void gfSetUnit(gfPoint p, gfunit u);
	/* sets p := u */

int  gfEqual (const gfPoint p, const gfPoint q);
	/* evaluates to 1 if p == q, otherwise 0 (or an error code) */

void gfClear (gfPoint p);
	/* sets p := 0, clearing entirely the content of p */

void gfRandom (gfPoint p);
	/* sets p := <random field element> */

void gfCopy (gfPoint p, const gfPoint q);
	/* sets p := q */

void gfAdd (gfPoint p, const gfPoint q, const gfPoint r);
	/* sets p := q + r */

void gfMultiply (gfPoint r, const gfPoint p, const gfPoint q);
	/* sets r := p * q mod (x^GF_K + x^GF_T + 1) */

void gfSmallDiv (gfPoint p, gfunit b);
	/* sets p := (b^(-1))*p mod (x^GF_K + x^GF_T + 1) for b != 0 (of course...) */

void gfSquare (gfPoint p, const gfPoint q);
	/* sets p := q^2 mod (x^GF_K + x^GF_T + 1) */

int  gfInvert (gfPoint p, const gfPoint q);
	/* sets p := q^(-1) mod (x^GF_K + x^GF_T + 1) */
	/* warning: p and q must not overlap! */

void gfSquareRoot (gfPoint p, gfunit b);
	/* sets p := sqrt(b) = b^(2^(GF_M-1)) */

int  gfTrace (const gfPoint p);
	/* quickly evaluates to the trace of p (or an error code) */

int  gfSolveQuad (gfPoint p, const gfPoint q);
	/* sets p to a solution of p^2 + p = q */

int  gfYbit (const gfPoint p);
	/* evaluates to the rightmost (least significant) bit of p (or an error code) */

void gfPack (const gfPoint p, vlPoint k);
	/* packs a field point into a vlPoint */

int gfUnpack (gfPoint p, const vlPoint k);
	/* unpacks a vlPoint into a field point */

int  gfSelfTest (int test_count);
	/* perform test_count self tests */

void gfReduce (gfPoint p);
	/* previously static/private */

#endif // _EC_FIELD_H_
