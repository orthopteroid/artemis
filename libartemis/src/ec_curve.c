/*
 * Elliptic curves over GF(2^m)
 *
 * This public domain software was written by Paulo S.L.M. Barreto
 * <pbarreto@uninet.com.br> based on original C++ software written by
 * George Barwood <george.barwood@dial.pipex.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "platform.h"
#include "ar_codes.h"
#include "ar_util.h"

#include "ec_curve.h"
#include "ec_field.h"
#include "ec_param.h"
#include "ec_vlong.h"

extern const vlPoint prime_order;
extern const ecPoint curve_point;

PRAGMA_PUSH
PRAGMA_OPTIMIZE

int ecCheck (const ecPoint *p)
	/* confirm that y^2 + x*y = x^3 + EC_B for point p */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( p->y ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	gfPoint t1, t2, t3, b;

	b[0] = 1; b[1] = EC_B;
	gfSquare (t1, p->y);
	gfMultiply (t2, p->x, p->y);
	gfAdd (t1, t1, t2);	/* t1 := y^2 + x*y */
	gfSquare (t2, p->x);
	gfMultiply (t3, t2, p->x);
	gfAdd (t2, t3, b);	/*/ t2 := x^3 + EC_B */
	return gfEqual (t1, t2);
} /* ecCheck */


int ecEqual (const ecPoint *p, const ecPoint *q)
	/* evaluates to 1 if p == q, otherwise 0 (or an error code) */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !q ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( q->y ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	return gfEqual (p->x, q->x) && gfEqual (p->y, q->y);
} /* ecEqual */


void ecClear (ecPoint *p)
	/* sets p to the point at infinity O, clearing entirely the content of p */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return; }

	gfClear (p->x);
	gfClear (p->y);
} /* ecClear */


void ecCopy (ecPoint *p, const ecPoint *q)
	/* sets p := q */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return; }
	if( !q ) { LOGFAIL( RC_INTERNAL ); return; }

	gfCopy (p->x, q->x);
	gfCopy (p->y, q->y);
} /* ecCopy */


int ecCalcY (ecPoint *p, int ybit)
	/* given the x coordinate of p, evaluate y such that y^2 + x*y = x^3 + EC_B */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 1; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); return 1; }

	int rc = 0;
	gfPoint a, b, t;

	b[0] = 1; b[1] = EC_B;

	/* simple sqrt if elliptic equation reduces to y^2 = EC_B: */
	if( p->x[0] == 0 ) { gfSquareRoot (p->y, EC_B); return 0; }

	/* evaluate alpha = x^3 + b = (x^2)*x + EC_B: */
	gfSquare (t, p->x); /* keep t = x^2 for beta evaluation */
	gfMultiply (a, t, p->x);
	gfAdd (a, a, b); /* now a == alpha */
	if( a[0] == 0 ) { p->y[0] = 0; rc=1; goto EXIT; }

	/* evaluate beta = alpha/x^2 = x + EC_B/x^2 */
	gfSmallDiv(t, EC_B);
	if( 1 == gfInvert(a, t) ) { LOGFAIL( RC_INTERNAL ); rc=1; goto EXIT; }
	gfAdd(a, p->x, a); /* now a == beta */

	/* check if a solution exists: */
	if( gfTrace(a) != 0 ) { LOGFAIL( RC_INTERNAL ); rc=1; goto EXIT; }

	/* solve equation t^2 + t + beta = 0 so that gfYbit(t) == ybit: */
	if( gfSolveQuad(t, a) == 1 ) { LOGFAIL( RC_INTERNAL ); rc=0; goto EXIT; }

	if( gfYbit (t) != ybit ) { t[1] ^= 1; }

	/* compute y = x*t: */
	gfMultiply(p->y, p->x, t);

EXIT:
	gfClear(a);
	gfClear(t);
	return rc;
} /* ecCalcY */


void ecAdd (ecPoint *p, const ecPoint *q)
	/* sets p := p + q */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return; }
	if( !q ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( p->y ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( q->x ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( q->y ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }

	gfPoint lambda, t, tx, ty, x3;

	/* first check if there is indeed work to do (q != 0): */
	if (q->x[0] != 0 || q->y[0] != 0) {
		if (p->x[0] != 0 || p->y[0] != 0) {
			/* p != 0 and q != 0 */
			if (gfEqual (p->x, q->x)) {
				/* either p == q or p == -q: */
				if (gfEqual (p->y, q->y)) {
					/* points are equal; double p: */
					if( 1 == ecDouble( p ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
				} else {
					/* must be inverse: result is zero */
					/* (should ASSERT that q->y = p->x + p->y) */
					p->x[0] = p->y[0] = 0;
				}
			} else {
				/* p != 0, q != 0, p != q, p != -q */
				/* evaluate lambda = (y1 + y2)/(x1 + x2): */
				gfAdd (ty, p->y, q->y);
				gfAdd (tx, p->x, q->x);
				if( 1 == gfInvert (t, tx) ) { LOGFAIL( RC_INTERNAL ); return; }
				gfMultiply (lambda, ty, t);
				/* evaluate x3 = lambda^2 + lambda + x1 + x2: */
				gfSquare (x3, lambda);
				gfAdd (x3, x3, lambda);
				gfAdd (x3, x3, tx);
				/* evaluate y3 = lambda*(x1 + x3) + x3 + y1: */
				gfAdd (tx, p->x, x3);
				gfMultiply (t, lambda, tx);
				gfAdd (t, t, x3);
				gfAdd (p->y, t, p->y);
				/* deposit the value of x3: */
				gfCopy (p->x, x3);
			}
		} else {
			/* just copy q into p: */
			gfCopy (p->x, q->x);
			gfCopy (p->y, q->y);
		}
	}
} /* ecAdd */


void ecSub (ecPoint *p, const ecPoint *r)
	/* sets p := p - r */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return; }
	if( !r ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( r->x ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }
	if( !gfIsValid( r->y ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return; }

	ecPoint t;

	gfCopy (t.x, r->x);
	gfAdd  (t.y, r->x, r->y);
	ecAdd (p, &t);
} /* ecSub */

#ifdef SELF_TESTING

void ecNegate (ecPoint *p)
	/* sets p := -p */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return; }

	gfAdd (p->y, p->x, p->y);
} /* ecNegate */

#endif /* SELF_TESTING */

int ecDouble (ecPoint *p)
	/* sets p := 2*p */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 1; }
	if( !gfIsValid( p->y ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return 1; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return 1; }

	gfPoint lambda, t1, t2;

	/* evaluate lambda = x + y/x: */
	if( 1 == gfInvert (t1, p->x) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return 1; }
	gfMultiply (lambda, p->y, t1);
	gfAdd (lambda, lambda, p->x);
	/* evaluate x3 = lambda^2 + lambda: */
	gfSquare (t1, lambda);
	gfAdd (t1, t1, lambda); /* now t1 = x3 */
	/* evaluate y3 = x^2 + lambda*x3 + x3: */
	gfSquare (p->y, p->x);
	gfMultiply (t2, lambda, t1);
	gfAdd (p->y, p->y, t2);
	gfAdd (p->y, p->y, t1);
	/* deposit the value of x3: */
	gfCopy (p->x, t1);
	return 0;
} /* ecDouble */

int ecMultiply (ecPoint *p, const vlPoint k)
	/* sets p := k*p */
{
	int rc = 0;

	if( !p ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( k ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }

	vlPoint h;
	int z, hi, ki;
	word16 i;
	ecPoint r;

	gfCopy(r.x, p->x); p->x[0] = 0;
	gfCopy(r.y, p->y); p->y[0] = 0;
	if( vlShortMultiply(h, k, 3) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	z = vlNumBits(h) - 1; /* so vlTakeBit (h, z) == 1 */
	i = 1;
	for (;;) {
		hi = vlTakeBit(h, i);
		ki = vlTakeBit(k, i);
		if( hi == 1 && ki == 0 ) { ecAdd (p, &r); }
		if( hi == 0 && ki == 1 ) { ecSub (p, &r); }
		if( i >= z ) { break; }
		i++;
		if( 1 == ecDouble(&r) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	}

EXIT:

	if( p && rc ) { ecClear( p ); }

	return rc;
} /* ecMultiply */


int ecYbit (const ecPoint *p)
	/* evaluates to 0 if p->x == 0, otherwise to gfYbit (p->y / p->x) */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( p->x ) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !gfIsValid( p->y ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	gfPoint t1, t2;

	if( p->x[0] == 0 ) { return 0; }

	if( 1 == gfInvert(t1, p->x) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	gfMultiply(t2, p->y, t1);
	return gfYbit(t2);
} /* ecYbit */


void ecPack( vlPoint k, const ecPoint *p )
	/* packs a curve point into a vlPoint */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); k[0] = 0; return; }

	vlPoint a;

	if( p->x[0] )
	{
		gfPack( k, p->x );
		vlShortLshift (k, 1);
		vlSetUnit(a, (word16) ecYbit (p));
		vlAdd(k, a);
	} else if( p->y[0] ) {
		vlSetUnit(k, 1);
	} else {
		k[0] = 0;
	}
} /* ecPack */


int ecUnpack( ecPoint *p, const vlPoint k )
	/* unpacks a vlPoint into a curve point */
{
	if( !p ) { LOGFAIL( RC_INTERNAL ); return 1; }
	if( !vlIsValid( k ) ) { LOGFAIL( RC_INTERNAL ); return 1; }

	int yb;
	vlPoint a;

	vlCopy(a, k);
	yb = a[0] ? a[1] & 1 : 0;
	vlShortRshift(a, 1);
	if( 1 == gfUnpack( p->x, a ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return 1; }

	if( p->x[0] || yb )
	{
		if( 1 == ecCalcY( p, yb ) ) { LOGFAIL( RC_INTERNAL ); ecClear( p ); return 1; }
		return 0;
	}

	p->y[0] = 0;
	return 0;
} /* ecUnpack */

int ecSelfTest (int test_count)
	/* perform test_count self tests */
{

#if defined(_DEBUGX)

	int i, yb, nfail = 0, afail = 0, sfail = 0, cfail = 0, qfail = 0, pfail = 0, yfail = 0;
	ecPoint f, g, x, y;
	vlPoint m, n, p;
	clock_t elapsed = 0L;

	srand ((unsigned)(time(NULL) % 65521U));
	printf ("Executing %d curve self tests...", test_count);
	for (i = 0; i < test_count; i++) {
		ecRandom (&f);
		ecRandom (&g);
		vlSetRandom (m);
		vlSetRandom (n);

		/* negation test: -(-f) = f */
		ecCopy (&x, &f);
		ecNegate (&x);
		ecNegate (&x);
		if (!ecEqual (&x, &f)) {
			nfail++;
			/* printf ("Addition test #%d failed!\n", i); */
		}
		/* addition test: f+g = g+f */
		ecCopy (&x, &f); ecAdd (&x, &g);
		ecCopy (&y, &g); ecAdd (&y, &f);
		if (!ecEqual (&x, &y)) {
			afail++;
			/* printf ("Addition test #%d failed!\n", i); */
		}
		/* subtraction test: f-g = f+(-g) */
		ecCopy (&x, &f); ecSub (&x, &g);
		ecCopy (&y, &g); ecNegate (&y); ecAdd (&y, &f);
		if (!ecEqual (&x, &y)) {
			sfail++;
			/* printf ("Subtraction test #%d failed!\n", i); */
		}
		/* quadruplication test: 2*(2*f) = f + f + f + f */
		ecCopy (&x, &f); ecDouble (&x); ecDouble (&x);
		ecClear (&y); ecAdd (&y, &f); ecAdd (&y, &f); ecAdd (&y, &f); ecAdd (&y, &f);
		if (!ecEqual (&x, &y)) {
			qfail++;
			/* printf ("Quadruplication test #%d failed!\n", i); */
		}
		/* scalar multiplication commutativity test: m*(n*f) = n*(m*f) */
		ecCopy (&x, &f);
		ecCopy (&y, &f);
		elapsed -= clock ();
		ecMultiply (&x, n); ecMultiply (&x, m);
		ecMultiply (&y, m); ecMultiply (&y, n);
		elapsed += clock ();
		if (!ecEqual (&x, &y)) {
			cfail++;
			/* printf ("Commutativity test #%d failed!\n", i); */
		}
		/* y calculation test: */
		yb = ecYbit (&f);
		ecClear (&x);
		gfCopy (x.x, f.x);
		ecCalcY (&x, yb);
		if (!ecEqual (&f, &x)) {
			yfail++;
			/* printf ("Y calculation test #%d failed!\n", i); */
		}
		/* packing test: unpack (pack (f)) = f */
		ecPack( p, &f );
		ecUnpack( &x, p );
		if (!ecEqual( &f, &x )) {
			pfail++;
			/* printf ("Packing test #%d failed!\n", i); */
		}
	}
	printf (" done, scalar multiplication time: %.3f s/op.\n",
		(float)elapsed/CLOCKS_PER_SEC/(test_count?4*test_count:4));
	if (nfail) printf ("---> %d negations failed <---\n", nfail);
	if (afail) printf ("---> %d additions failed <---\n", afail);
	if (sfail) printf ("---> %d subtractions failed <---\n", sfail);
	if (qfail) printf ("---> %d quadruplications failed <---\n", qfail);
	if (cfail) printf ("---> %d commutativities failed <---\n", cfail);
	if (yfail) printf ("---> %d y calculations failed <---\n", yfail);
	if (pfail) printf ("---> %d packings failed <---\n", pfail);
	return nfail || afail || sfail || qfail || cfail || yfail || pfail;

#else

	return 0;

#endif

} /* ecSelfTest */

PRAGMA_POP
