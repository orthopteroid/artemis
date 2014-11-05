/*
 * Multiple-precision ("very long") integer arithmetic
 *
 * This public domain software was written by Paulo S.L.M. Barreto
 * <pbarreto@uninet.com.br> based on original C++ software written by
 * George Barwood <george.barwood@dial.pipex.com>
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ''AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * References:
 *
 * 1.	Knuth, D. E.: "The Art of Computer Programming",
 *		2nd ed. (1981), vol. II (Seminumerical Algorithms), p. 257-258.
 *		Addison Wesley Publishing Company.
 *
 * 2.	Hansen, P. B.: "Multiple-length Division Revisited: a Tour of the Minefield".
 *		Software - Practice and Experience 24:6 (1994), 579-601.
 *
 * 3.	Menezes, A. J., van Oorschot, P. C., Vanstone, S. A.:
 *		"Handbook of Applied Cryptography", CRC Press (1997), section 14.2.5.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "platform.h"
#include "ar_util.h"
#include "ar_codes.h"

#include "ec_param.h"
#include "ec_vlong.h"

int vlIsZero(const vlPoint p)
{
	if( p[0] == 0 ) { return 1; }
	for( size_t i = 0; i < p[0]; i++ ) { if( p[ 1 + i ] ) { return 0; } }
	return 1;
}

int vlEqual (const vlPoint p, const vlPoint q)
{
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return 1; }
	if( !vlIsValid( q ) ) { LOGFAIL( RC_INTERNAL ); return 1; }

	if( p[0] != q[0] )
		return 0;
	else
		return memcmp( p, q, (p[0] + 1) * sizeof(word16) ) ? 0 : 1;
} /* vlEqual */


void vlClear (vlPoint p)
{
	memset (p, 0, sizeof (vlPoint));
} /* vlClear */


void vlCopy (vlPoint p, const vlPoint q)
	/* sets p := q */
{
	if( !vlIsValid( q ) ) { LOGFAIL( RC_INTERNAL ); vlClear( p ); return; }
	memcpy( p, q, (q[0] + 1) * sizeof(word16) );
} /* vlCopy */


word16 vlGetWord16(vlPoint p, word16 i)
{
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	return ( i <= p[0] ) ? p[ p[0] - i ] : 0; // orthopteroid
}

void vlSetWord16(vlPoint p, word16 u)
{
	if( u == 0 )
		p[0] = 0;
	else {
		p[0] = 1;
		p[1] = u;
		if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return; }
	}
}

int vlNumBits (const vlPoint k)
	/* evaluates to the number of bits of k (index of most significant bit, plus one) */
{
	if( !vlIsValid( k ) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( k[0] == 0 ) { return 0; }

	word16 m = 0;
	word16 w = k[k[0]]; /* last unit of k */
	for( int i = (int)(k[0] << 4), m = 0x8000U; m; i--, m >>= 1 )
	{
		if( w & m ) { return i; }
	}
	return 0;
} /* vlNumBits */


int vlTakeBit (const vlPoint k, word16 i)
	/* evaluates to the i-th bit of k */
{
	if( !vlIsValid( k ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	if (i >= (k[0] << 4)) {
		return 0;
	}
	return (int)((k[(i >> 4) + 1] >> (i & 15)) & 1);
} /* vlTakeBit */


void vlAdd (vlPoint u, const vlPoint v)
{
	if( !vlIsValid( v ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( !vlIsValid( u ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }

	word16 i;
	word32 t;

	/* clear high words of u if necessary: */
	for (i = u[0] + 1; i <= v[0]; i++) {
		u[i] = 0;
	}
    if (u[0] < v[0])
      u[0] = v[0];
	t = 0L;
	for (i = 1; i <= v[0]; i++) {
		t = t + (word32)u[i] + (word32)v[i];
		u[i] = (word16) (t & 0xFFFFUL);
		t >>= 16;
	}
    i = v[0]+1;
	while (t) {
        if ( i > u[0] )
        {
          u[i] = 0;
          u[0] += 1;
        }
        t = (word32)u[i] + 1;
		u[i] = (word16) (t & 0xFFFFUL);
        t >>= 16;
        i += 1;
	}
} /* vlAdd */


void vlSubtract (vlPoint u, const vlPoint v)
{
	if( !vlIsValid( v ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( !vlIsValid( u ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }

	/* Assume u >= v */

	word32 carry = 0, tmp;
	int i = 0;
	for( i = 1; i <= v[0]; i++ )
	{
		tmp = 0x10000UL + (word32)u[i] - (word32)v[i] - carry;
		carry = 1;
		if (tmp >= 0x10000UL) {
			tmp -= 0x10000UL;
			carry = 0;
		}
		u[i] = (word16) tmp;
	}
	if( carry )
	{
		while( u[i] == 0 ) { i++; }
		u[i]--;
	}
	while( u[u[0]] == 0 && u[0] )
	{
		u[0]--;
	}
} /* vlSubtract */


void vlShortLshift (vlPoint p, int n)
{
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); vlClear( p ); return; }
	if( p[0] == 0 ) { return; }

	/* this will only work if 0 <= n <= 16 */
	if (p[p[0]] >> (16 - n)) {
		/* check if there is enough space for an extra unit: */
		if (p[0] <= VL_UNITS + 1) {
			++p[0];
			p[p[0]] = 0; /* just make room for one more unit */
		}
	}

	for( word16 i = p[0]; i > 1; i-- )
	{
		p[i] = (p[i] << n) | (p[i - 1] >> (16 - n));
	}
	p[1] <<= n;
} /* vlShortLshift */


void vlShortRshift (vlPoint p, int n)
{
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); vlClear( p ); return; }
	if( p[0] == 0 ) { return; }

	/* this will only work if 0 <= n <= 16 */
	for( word16 i = 1; i < p[0]; i++ )
	{
		p[i] = (p[i + 1] << (16 - n)) | (p[i] >> n);
	}

	p[p[0]] >>= n;
	if (p[p[0]] == 0) {
		--p[0];
	}
} /* vlShortRshift */


int vlShortMultiply (vlPoint p, const vlPoint q, word16 d)
	/* sets p = q * d, where d is a single digit */
{
	if( !vlIsValid( q ) ) { LOGFAIL( RC_INTERNAL ); vlClear( p ); return -1; }

	int i;
	word32 t;

	if (d > 1) {
		t = 0L;
		for (i = 1; i <= q[0]; i++) {
			t += (word32)q[i] * (word32)d;
			p[i] = (word16) (t & 0xFFFFUL);
			t >>= 16;
		}
		if (t) {
			p[0] = q[0] + 1;
			p[p[0]] = (word16) (t & 0xFFFFUL);
		} else {
			p[0] = q[0];
		}
	} else if (d) { /* d == 1 */
		vlCopy (p, q);
	} else { /* d == 0 */
		p[0] = 0;
	}

	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); vlClear( p ); return -1; }

	return 0;
} /* vlShortMultiply */


int vlGreater(const vlPoint p, const vlPoint q)
{
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return 0; }
	if( !vlIsValid( q ) ) { LOGFAIL( RC_INTERNAL ); return 0; }

	if (p[0] > q[0]) return 1;
	if (p[0] < q[0]) return 0;
	for( int i = p[0]; i > 0; i-- )
	{
		if (p[i] > q[i]) return 1;
		if (p[i] < q[i]) return 0;
	}
	return 0;
} /* vlGreater */


void vlRemainder(vlPoint u, const vlPoint v)
{
	if( !vlIsValid( u ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( !vlIsValid( v ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( v[0] == 0 ) { LOGFAIL( RC_INTERNAL ); return; }

	vlPoint t;
	int shift = 0;

	vlCopy( t, v );
	while ( vlGreater( u, t ) )
	{
		vlShortLshift( t, 1 );
		shift += 1;
	}
	while ( 1 )
	{
		if ( vlGreater( t, u ) )
		{
			if (shift)
			{
				vlShortRshift( t, 1 );
				shift -= 1;
			}
			else
				break;
		}
		else
			vlSubtract( u, t );
	}
} /* vlRemainder */



void vlMulMod (vlPoint u, const vlPoint v, const vlPoint w, const vlPoint m)
{
	if( !vlIsValid( v ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( !vlIsValid( w ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( !vlIsValid( m ) ) { LOGFAIL( RC_INTERNAL ); vlClear( u ); return; }
	if( m[0] == 0 ) { LOGFAIL( RC_INTERNAL ); return; }

	vlPoint t;	
	vlCopy( t, w );

	for( int i=1; i<=v[0]; i+=1 )
	{
		for( int j=0; j<16; j+=1 )
		{
			if( v[i] & (1u<<j) )
			{
				vlAdd( u, t );
				vlRemainder( u, m );
			}
			vlShortLshift( t, 1 );
			vlRemainder( t, m );
		}
	}
} /* vlMulMod */


void vlSetRandom( vlPoint p, word16 maxWord16s, rnd16gen fn )
{
	p[0] = ( VL_UNITS-2 < maxWord16s ) ? VL_UNITS-2 : maxWord16s;
	for( size_t i = 0; i < p[0]; i++ ) { p[i+1] = fn(); } // +1 to skip length indicator
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return; }
}

void vlSetWord32Ptr( vlPoint p, word16 maxWord16s, word32* q )
{
	p[0] = ( VL_UNITS-2 < maxWord16s ) ? VL_UNITS-2 : maxWord16s;
	for( size_t j = 0, i = 0; i < p[0]; i++ )
	{
		p[i+1] = 0xFFFF & (q[j] >> ( (i & 0x01) ? 0 : 16 ) ); // hi on even and low on odd, +1 to skip length indicator
		j += (i & 0x01 ); // inc j on odd i
	}
	if( !vlIsValid( p ) ) { LOGFAIL( RC_INTERNAL ); return; }
}
