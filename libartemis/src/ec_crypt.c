/*
 * Elliptic curve cryptographic primitives
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
 */

#include <stdio.h>

#include "platform.h"
#include "ar_codes.h"
#include "ar_util.h"

#include "ec_curve.h"
#include "ec_vlong.h"
#include "ec_crypt.h"

int cpMakePublicKey( vlPoint vlPublicKey, const vlPoint vlPrivateKey)
{
	int rc = 0;
	ecPoint ecPublicKey;

	ecCopy( &ecPublicKey, &curve_point );
	if( rc = ecMultiply( &ecPublicKey, vlPrivateKey ) ) { LOGFAIL( rc ); goto EXIT; }
	ecPack( vlPublicKey, &ecPublicKey );
EXIT:
	return rc;
} /* cpMakePublicKey */


int cpEncodeSecret(vlPoint vlSecret, vlPoint vlMessage, const vlPoint vlPublicKey)
{
	int rc = 0;
	ecPoint q;

	ecCopy( &q, &curve_point );
	if( rc = ecMultiply( &q, vlSecret ) ) { LOGFAIL( rc ); goto EXIT; }
	ecPack( vlMessage, &q );
	ecUnpack( &q, vlPublicKey );
	if( rc = ecMultiply( &q, vlSecret ) ) { LOGFAIL( rc ); goto EXIT; }
	gfPack( vlSecret, q.x );
EXIT:
	return rc;
} /* cpMakeSecret */


int cpDecodeSecret(vlPoint d, vlPoint vlMessage, const vlPoint vlPrivateKey)
{
	int rc = 0;
	ecPoint q;

	ecUnpack( &q, vlMessage );
	if( rc = ecMultiply( &q, vlPrivateKey ) ) { LOGFAIL( rc ); goto EXIT; }
	gfPack( d, q.x );
EXIT:
	return rc;
} /* ecDecodeSecret */

int cpSign(cpPair * sig, const vlPoint vlPrivateKey, const vlPoint k, const vlPoint vlMac)
{
	int rc = 0;
	ecPoint q;
	vlPoint tmp;
	
	vlClear( tmp );
	ecCopy( &q, &curve_point );
	if( rc = ecMultiply( &q, k ) ) { LOGFAIL( rc ); cpClear( sig ); goto EXIT; }
	gfPack( sig->r, q.x );
	vlAdd( sig->r, vlMac );
	vlRemainder( sig->r, prime_order );
	if( sig->r[0] == 0 ) { rc = RC_INTERNAL; LOGFAIL( rc ); cpClear( sig ); goto EXIT; }
	vlMulMod( tmp, vlPrivateKey, sig->r, prime_order );
	vlCopy( sig->s, k );
	if( vlGreater( tmp, sig->s ) ) { vlAdd( sig->s, prime_order ); }
	vlSubtract( sig->s, tmp );
EXIT:
	return rc;
} /* cpSign */

int cpVerify(int* pEqual, cpPair * sig, const vlPoint vlPublicKey, const vlPoint vlMac)
{
	int rc = 0;

	if( !pEqual ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }
	if( !sig ) { rc = RC_NULL; LOGFAIL( rc ); goto EXIT; }

	*pEqual = 0; // assume !equal

	if( !vlIsValid( vlPublicKey ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( vlMac) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( sig->r ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }
	if( !vlIsValid( sig->s ) ) { rc = RC_INTERNAL; LOGFAIL( rc ); goto EXIT; }

	ecPoint t1,t2;
	vlPoint t3,t4;
	
	ecCopy( &t1, &curve_point );
	if( rc = ecMultiply( &t1, sig->s ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ecUnpack( &t2, vlPublicKey ) ) { LOGFAIL( rc ); goto EXIT; }
	if( rc = ecMultiply( &t2, sig->r ) ) { LOGFAIL( rc ); goto EXIT; }
	ecAdd( &t1, &t2 );
	gfPack( t4, t1.x );
	vlRemainder( t4, prime_order );
	vlCopy( t3, sig->r );
	if( vlGreater( t4, t3 ) ) { vlAdd( t3, prime_order ); }
	vlSubtract( t3, t4 );
	*pEqual = vlEqual( t3, vlMac );
EXIT:
	return rc;
} /* cpVerify */

void cpCopy( cpPair* pp, const cpPair* pq )
{
	vlCopy( pp->r, pq->r );
	vlCopy( pp->s, pq->s );
}

void cpClear( cpPair* p )
{
	vlClear( p->r );
	vlClear( p->s );
}
