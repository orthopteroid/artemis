#ifndef __EC_CURVE_H
#define __EC_CURVE_H

#include <stddef.h>

#include "ec_types.h"
#include "ec_field.h"
#include "ec_vlong.h"

typedef struct {
	gfPoint x, y;
} ecPoint;

extern const vlPoint prime_order;
extern const ecPoint curve_point;

int ecCheck (const ecPoint *p);
	/* confirm that y^2 + x*y = x^3 + EC_B for point p */

int ecEqual (const ecPoint *p, const ecPoint *q);
	/* evaluates to 1 if p == q, otherwise 0 (or an error code) */

void ecClear (ecPoint *p);
	/* sets p to the point at infinity O, clearing entirely the content of p */

void ecCopy (ecPoint *p, const ecPoint *q);
	/* sets p := q */

int ecCalcY (ecPoint *p, int ybit);
	/* given the x coordinate of p, evaluate y such that y^2 + x*y = x^3 + EC_B */

void ecAdd (ecPoint *p, const ecPoint *r);
	/* sets p := p + r */

void ecSub (ecPoint *p, const ecPoint *r);
	/* sets p := p - r */

void ecNegate (ecPoint *p);
	/* sets p := -p */

void ecDouble (ecPoint *p);
	/* sets p := 2*p */

void ecMultiply (ecPoint *p, const vlPoint k);
	/* sets p := k*p */

int ecYbit (const ecPoint *p);
	/* evaluates to 0 if p->x == 0, otherwise to gfYbit (p->y / p->x) */

void ecPack (const ecPoint *p, vlPoint k);
	/* packs a curve point into a vlPoint */

int ecUnpack (ecPoint *p, const vlPoint k);
	/* unpacks a vlPoint into a curve point, returns 0 if error */

#endif /* __EC_CURVE_H */
