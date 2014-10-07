/*
 * Algebraic operations on the finite field GF(2^m)
 *
 * This public domain software was written by Paulo S.L.M. Barreto
 * <pbarreto@uninet.com.br> based on original C++ software written by
 * George Barwood <george.barwood@dial.pipex.com>
 *
 * References:
 *
 * 1.	Erik De Win <erik.dewin@esat.kuleuven.ac.be> et alii:
 *		"A Fast Software Implementation for Arithmetic Operations in GF(2^n)",
 *		presented at Asiacrypt96 (preprint).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform.h"
#include "ar_util.h"
#include "ar_codes.h"

#include "ec_field.h"
#include "ec_param.h"
#include "ec_vlong.h"

#define BASE	(1U << GF_L)
#define TOGGLE	(BASE-1)

static lunit *expt = NULL; /* index range is [0..(BASE-2)] */
static lunit *logt = NULL; /* index range is [1..(BASE-1)], but logt[0] is set to (BASE-1) */


int gfInit (void)
	/* initialize the library ---> MUST be called before any other gf-function */
{
	ltemp root, i, j;

	if (logt != NULL && expt != NULL) {
		/* already initialized */
		return 0;
	}
	if (logt != NULL && expt == NULL ||
		logt == NULL && expt != NULL) {
		return 2; /* logic error: half initialized (?!) */
	}
	if ((logt = (lunit *) malloc (BASE * sizeof (lunit))) == NULL) {
		return 1; /* not enough memory */
	}
	if ((expt = (lunit *) malloc (BASE * sizeof (lunit))) == NULL) {
		free (logt); logt = NULL;
		return 1; /* not enough memory */
	}
	root = BASE | GF_RP;
	expt[0] = 1;
	for (i = 1; i < BASE; i++) { 
		j = (ltemp)expt[i-1] << 1;
		if (j & BASE) {
			j ^= root;
		}
		expt[i] = (lunit)j;
	}
	for (i = 0; i < TOGGLE; i++) {
		logt[expt[i]] = (lunit)i;
	}
	logt[0] = TOGGLE; /* a trick used by gfMultiply, gfSquare, gfAddMul */

	return 0;
} /* gfInit */


void gfQuit (void)
	/* perform housekeeping for library termination */
{
	if (expt) {
		free (expt); expt = NULL;
	}
	if (logt) {
		free (logt); logt = NULL;
	}
} /* gfQuit */

int gfIsValid(const gfPoint p)
{
	return p[0] < GF_POINT_UNITS;
}

int gfEqual (const gfPoint p, const gfPoint q)
	/* evaluates to 1 if p == q, otherwise 0 (or an error code) */
{
	ASSERT (p != NULL);
	ASSERT (q != NULL);
	return memcmp (p, q, sizeof(lunit) * (p[0] + 1)) ? 0 : 1;
} /* gfEqual */


void gfClear (gfPoint p)
	/* sets p := 0, clearing entirely the content of p */
{
	ASSERT (p != NULL);
	memset (p, 0, sizeof(gfPoint));
} /* gfClear */


void gfSetLUnit (gfPoint p, lunit u)
	/* sets p := u */
{
	ASSERT (p != NULL);
	p[0] = 1; p[1] = u;
	if( !u ) p[0] = 0; // orthopteroid
} /* gfSetLUnit */

void gfCopy (gfPoint p, const gfPoint q)
	/* sets p := q */
{
	ASSERT (p != NULL);
	ASSERT (q != NULL);
	size_t b = (q[0] + 1) * sizeof(lunit);
	if( b > sizeof(gfPoint) ) { LOGFAIL( RC_INTERNAL ); b = sizeof(gfPoint); }
	memcpy( p, q, b );
} /* gfCopy */


void gfAdd (gfPoint p, const gfPoint q, const gfPoint r)
	/* sets p := q + r */
{
	ltemp i;

	ASSERT (logt != NULL && expt != NULL);
	ASSERT (p != NULL);
	ASSERT (q != NULL);
	ASSERT (r != NULL);
	if (q[0] > r[0]) {
		/* xor the the common-degree coefficients: */
		for (i = 1; i <= r[0]; i++) {
			p[i] = q[i] ^ r[i];
		}
		/* invariant: i == r[0] + 1 */
		memcpy (&p[i], &q[i], (q[0] - r[0]) * sizeof (lunit));
		/* deg(p) inherits the value of deg(q): */
		p[0] = q[0];
	} else if (q[0] < r[0]) {
		/* xor the the common-degree coefficients: */
		for (i = 1; i <= q[0]; i++) {
			p[i] = q[i] ^ r[i];
		}
		/* invariant: i == q[0] + 1 */
		memcpy (&p[i], &r[i], (r[0] - q[0]) * sizeof (lunit));
		/* deg(p) inherits the value of deg(r): */
		p[0] = r[0];
	} else { /* deg(q) == deg(r) */
		/* scan to determine deg(p): */
		for (i = q[0]; i > 0; i--) {
			if (q[i] ^ r[i]) {
				break;
			}
		}
		/* xor the the common-degree coefficients, if any is left: */
		for (p[0] = (lunit)i; i > 0; i--) {
			p[i] = q[i] ^ r[i];
		}
	}
} /* gfAdd */


void gfReduce (gfPoint p)
	/* reduces p mod the irreducible trinomial x^GF_K + x^GF_T + 1 */
{
	int i;

	for (i = p[0]; i > GF_K; i--) {
		p[i - GF_K] ^= p[i];
		p[i + GF_T - GF_K] ^= p[i];
		p[i] = 0;
	}
	if (p[0] > GF_K) {
		/* scan to update deg(p): */
		p[0] = GF_K;
		while (p[0] && p[p[0]]==0) {
			p[0]--;
		}
	}
} /* gfReduce */


void gfMultiply (gfPoint r, const gfPoint p, const gfPoint q)
	/* sets r := p * q mod (x^GF_K + x^GF_T + 1) */
{
	int i, j;
	ltemp x, log_pi, log_qj;
	lunit lg[GF_K + 2]; /* this table should be cleared after use */

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( p );
	ASSERT( q );
	ASSERT( r );

	if( p == r ) { LOGFAIL( RC_INTERNAL ); return; }
	if( q == r ) { LOGFAIL( RC_INTERNAL ); return; }
	if( p[0] + q[0] > GF_POINT_UNITS ) { LOGFAIL( RC_INTERNAL ); return; }

	if (p[0] && q[0]) {
		/* precompute logt[q[j]] to reduce table lookups: */
		for (j = q[0]; j; j--) {
			lg[j] = logt[q[j]];
		}
		/* perform multiplication: */
		gfClear (r);
		for (i = p[0]; i; i--) {
			if ((log_pi = logt[p[i]]) != TOGGLE) { /* p[i] != 0 */
				for (j = q[0]; j; j--) {
					if ((log_qj = lg[j]) != TOGGLE) { /* q[j] != 0 */
					/*	r[i+j-1] ^= expt[(logt[p[i]] + logt[q[j]]) % TOGGLE]; */
						r[i+j-1] ^= expt[(x = log_pi + log_qj) >= TOGGLE ? x - TOGGLE : x];
					}
				}
			}
		}
		r[0] = p[0] + q[0] - 1;
		/* reduce r mod (x^GF_K + x^GF_T + 1): */
		gfReduce (r);
	} else {
		/* set r to the null polynomial: */
		r[0] = 0;
	}
	/* destroy potentially sensitive data: */
	x = log_pi = log_qj = 0;
	memset (lg, 0, sizeof (lg));
} /* gfMultiply */


void gfSquare (gfPoint r, const gfPoint p)
	/* sets r := p^2 mod (x^GF_K + x^GF_T + 1) */
{
	int i;
	ltemp x;

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( r );
	ASSERT( p );

	if (p[0]) {
		/* in what follows, note that (x != 0) => (x^2 = exp((2 * log(x)) % TOGGLE)): */
		i = p[0];
		if ((x = logt[p[i]]) != TOGGLE) { /* p[i] != 0 */
			r[2*i - 1] = expt[(x += x) >= TOGGLE ? x - TOGGLE : x];
		} else {
			r[2*i - 1] = 0;
		}
		for (i = p[0] - 1; i; i--) {
			r[2*i] = 0;
			if ((x = logt[p[i]]) != TOGGLE) { /* p[i] != 0 */
				r[2*i - 1] = expt[(x += x) >= TOGGLE ? x - TOGGLE : x];
			} else {
				r[2*i - 1] = 0;
			}
		}
		r[0] = 2*p[0] - 1;
		/* reduce r mod (x^GF_K + x^GF_T + 1): */
		gfReduce (r);
	} else {
		r[0] = 0;
	}
} /* gfSquare */


void gfSmallDiv (gfPoint p, lunit b)
	/* sets p := (b^(-1))*p mod (x^GF_K + x^GF_T + 1) */
{
	int i;
	ltemp x, lb = logt[b];

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( p );
	ASSERT( b );

	for (i = p[0]; i; i--) {
		if ((x = logt[p[i]]) != TOGGLE) { /* p[i] != 0 */
			p[i] = expt[(x += TOGGLE - lb) >= TOGGLE ? x - TOGGLE : x];
		}
	}
} /* gfSmallDiv */


static void gfAddMul (gfPoint a, ltemp alpha, ltemp j, gfPoint b)
{
	ltemp i, x, la = logt[alpha];
	lunit *aj = &a[j];

	ASSERT( logt );
	ASSERT( expt );

	while (a[0] < j + b[0]) {
		a[0]++; a[a[0]] = 0;
	}
	for (i = b[0]; i; i--) {
		if ((x = logt[b[i]]) != TOGGLE) { /* b[i] != 0 */
			aj[i] ^= expt[(x += la) >= TOGGLE ? x - TOGGLE : x];
		}
	}
	while (a[0] && a[a[0]]==0) {
		a[0]--;
	}
} /* gfAddMul */


int gfInvert (gfPoint b, const gfPoint a)
	/* sets b := a^(-1) mod (x^GF_K + x^GF_T + 1) */
	/* warning: a and b must not overlap! */
{
	gfPoint c, f, g;
	ltemp x, j, alpha;

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( a );
	ASSERT( b );
	ASSERT( a != b ); /* note that this test is not complete */

	if (a[0] == 0) {
		/* a is not invertible */
		return 1;
	}

	/* initialize b := 1; c := 0; f := p; g := x^GF_K + x^GF_T + 1: */
	b[0] = 1; b[1] = 1;
	c[0] = 0;
	gfCopy (f, a);
	gfClear (g);
	g[0] = GF_K + 1; g[1] = 1; g[GF_T + 1] = 1; g[GF_K + 1] = 1;

	for (;;) {
		if (f[0] == 1) {
			if( f[1] == 0 ) { LOGFAIL( RC_INTERNAL ); }
			gfSmallDiv (b, f[1]);
			/* destroy potentially sensitive data: */
			gfClear (c); gfClear (f); gfClear (g); x = j = alpha = 0;
			return 0;
		}
		if (f[0] < g[0]) {
			goto SWAP_FG;
		}
SWAP_GF:
		j = f[0] - g[0];
		x = logt[f[f[0]]] - logt[g[g[0]]] + TOGGLE;
		alpha = expt[x >= TOGGLE ? x - TOGGLE : x];
		gfAddMul (f, alpha, j, g);
		gfAddMul (b, alpha, j, c);
	}

	/* basically same code with b,c,f,g swapped */
	for (;;) {
		if (g[0] == 1) {
			if( g[1] == 0 ) { LOGFAIL( RC_INTERNAL ); }
			gfSmallDiv (c, g[1]);
			gfCopy (b, c);
			/* destroy potentially sensitive data: */
			gfClear (c); gfClear (f); gfClear (g); x = j = alpha = 0;
			return 0;
		}
		if (g[0] < f[0]) {
			goto SWAP_GF;
		}
SWAP_FG:
		j = g[0] - f[0];
		x = logt[g[g[0]]] - logt[f[f[0]]] + TOGGLE;
		alpha = expt[x >= TOGGLE ? x - TOGGLE : x];
		gfAddMul (g, alpha, j, f);
		gfAddMul (c, alpha, j, b);
	}
} /* gfInvert */


void gfSquareRoot (gfPoint p, lunit b)
	/* sets p := sqrt(b) = b^(2^(GF_M-1)) */
{
	int i;
	gfPoint q;

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( p );

	q[0] = 1; q[1] = b;
	if ((GF_M - 1) & 1) {
		/* GF_M - 1 is odd */
		gfSquare (p, q);
		i = GF_M - 2;
	} else {
		/* GF_M - 1 is even */
		gfCopy (p, q);
		i = GF_M - 1;
	}
	while (i) {
		gfSquare (p, p);
		gfSquare (p, p);
		i -= 2;
	}
} /* gfSquareRoot */


int gfTrace (const gfPoint p)
	/* quickly evaluates to the trace of p (or an error code) */
{
/*
	Let GF(2^m) be considered as a space vector over GF(2).
	The trace function Tr: GF(2^m) -> GF(2) is linear:
	Tr(p + q) = Tr(p) + Tr(q) and Tr(k*p) = k*Tr(p) for k in GF(2).

	Hence, the trace of any field element can be efficiently computed
	if the trace is known for a basis of GF(2^m).

	In other terms, let p(x) = SUM {p_i * x^i} for i = 0...m-1;
	then Tr(p) = SUM {p_i * Tr(x^i)} for i = 0...m-1.

	Surprisingly enough (at least for me :-), it is often the case that
	Tr(p) is simply Tr(p_0) or else Tr(p_0) + Tr(p_(m-1)).

	These properties are exploited in this fast algorithm by George Barwood.
*/

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( p );

#if (GF_TM0 == 1) && (GF_TM1 == 1)
	/* unit trace mask */
	return p[0] ? p[1] & 1 : 0;
#else
	if (p[0]) {
		int i;
		lunit w;

		w = p[1] & GF_TM1;
#if GF_TM0 != 1
		if (p[0] <= GF_TM0) {
			w ^= p[GF_TM0] & GF_TM2;
		}
#endif /* ?(GF_TM0 != 1) */
		/* compute parity of w: */
		for (i = BITS_PER_LUNIT/2; i > 0; i >>= 1) {
			w ^= w >> i;
		}
		return w & 1;
	} else {
		return 0;
	}
#endif
} /* gfTrace */


int gfSolveQuad (gfPoint p, const gfPoint beta)
	/* sets p to a solution of p^2 + p = beta */
{
	int i;
#if (GF_M & 1) == 0

	gfPoint d, t, nzt;

#endif /* ?((GF_M & 1) == 0) */

	ASSERT( logt );
	ASSERT( expt );
	ASSERT( p );
	ASSERT( beta );
	ASSERT( p != beta ); /* note that this test is not complete */

	/* check if a solution exists: */
	if( gfTrace( beta ) != 0 ) { LOGFAIL( RC_INTERNAL ); return 1; }

#if (GF_M & 1) == 0

	p[0] = 0;
	gfCopy(d, beta);
	nzt[0] = 1;
	nzt[1] = GF_NZT; /* field element with nonzero trace */
	if( gfTrace(nzt) == 0 ) { LOGFAIL( RC_INTERNAL ); return 1; }
	for (i = 1; i < GF_M; i++) {
		gfSquare (p, p);
		gfSquare (d, d);
		gfMultiply (t, d, nzt);
		gfAdd (p, p, t);
		gfAdd (d, d, beta);
	}
	/* destroy potentially sensitive information: */
	gfClear (d);
	gfClear (t);

#else /* GF_M is odd: compute half-trace */

	gfCopy (p, beta);
	for (i = 0; i < GF_M/2; i++) {
		gfSquare (p, p);
		gfSquare (p, p);
		gfAdd (p, p, beta);
	}

#endif /* ?((GF_M & 1) == 0) */

	return 0;
} /* gfSolveQuad */


int gfYbit (const gfPoint p)
	/* evaluates to the rightmost (least significant) bit of p (or an error code) */
{
	ASSERT( p );

	return p[0] ? (int) (p[1] & 1) : 0;
} /* gfYbit */


void gfPack (const gfPoint p, vlPoint k)
	/* packs a field point into a vlPoint */
{
	int i;
	vlPoint a;

	vlClear(k);
	a[0] = 1;
	for (i = p[0]; i > 0; i--) {
		vlShortLshift (k, GF_L); /* this only works if GF_L <= 16 */
		a[1] = p[i];
		vlAdd (k, a);
	}
} /* gfPack */


void gfUnpack (gfPoint p, const vlPoint k)
	/* unpacks a vlPoint into a field point */
{
	if( k[0] > VL_UNITS ) { LOGFAIL( RC_INTERNAL ); return; }

	vlPoint x;
	lunit n;

	vlCopy (x, k);
	for (n = 0; x[0]; n++) {
		p[n+1] = (lunit) (x[1] & TOGGLE);
		vlShortRshift (x, GF_L); /* this only works if GF_L <= 16 */
	}
	p[0] = n;
} /* gfUnpack */

