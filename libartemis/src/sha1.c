/*
SHA-1 in C
By Steve Reid <steve@edmweb.com>
100% Public Domain

Test Vectors (from FIPS PUB 180-1)
"abc"
  A9993E36 4706816A BA3E2571 7850C26C 9CD0D89D
"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
  84983E44 1C3BD26E BAAE4AA1 F95129E5 E54670F1
A million repetitions of "a"
  34AA973C D4C4DAA4 F61EEB2B DBAD2731 6534016F
*/

#include <string.h>
#include <stdio.h>

#include "platform.h"
#include "sha1.h"

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#ifdef LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

//#define SHA1HANDSOFF // Copies data before messing with it.

/* Hash a single 512-bit block. This is the core of the algorithm. */

PRAGMA_PUSH
PRAGMA_O3

void sha1_transform( word32 state[5], byte buffer[64])
{
	word32 a, b, c, d, e;
	typedef union
	{
		byte c[64];
		word32 l[16];
	} CHAR64LONG16;
	CHAR64LONG16* block;

#ifdef SHA1HANDSOFF

	static byte workspace[64];
    block = (CHAR64LONG16*)workspace;
    memcpy(block, buffer, 64);

#else

    block = (CHAR64LONG16*)buffer;

#endif

    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
}

PRAGMA_POP

/* Initialize new context */

void sha1_initial(sha1_context* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
    memset(context->buffer, 0, 64);
}


/* Run your data through this. */
void sha1_process( sha1_context * context, unsigned char * data, unsigned len )
{
	unsigned int i, j;
	unsigned long blen = ((unsigned long)len)<<3;

    j = (context->count[0] >> 3) & 63;
    if ((context->count[0] += blen) < blen ) context->count[1]++;
    context->count[1] += (len >> 29);
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
        sha1_transform(context->state, context->buffer);
        for ( ; i + 63 < len; i += 64) {
            sha1_transform(context->state, &data[i]);
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}


/* Add padding and return the message digest. */

void sha1_final( sha1_context* context, sha1Digest digest )
{
	unsigned long i, j;
	unsigned char finalcount[8];

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    sha1_process(context, (unsigned char *)"\200", 1);
    while ((context->count[0] & 504) != 448) {
        sha1_process(context, (unsigned char *)"\0", 1);
    }
    sha1_process(context, finalcount, 8);  /* Should cause a sha1_transform() */
    for (i = 0; i < 5; i++) {
        digest[i] = context->state[i];
    }
    /* Wipe variables */
    i = j = 0;
    memset(context->buffer, 0, 64);
    memset(context->state, 0, 20);
    memset(context->count, 0, 8);
    memset(&finalcount, 0, 8);
#ifdef SHA1HANDSOFF  /* make sha1_transform overwrite it's own static vars */
    sha1_transform(context->state, context->buffer);
#endif
}

void sha1_digest( sha1Digest digest, byteptr bytes, size_t len )
{
	sha1_context c[1];
	sha1_initial( c );
	sha1_process( c, bytes, (unsigned)len );
	sha1_final( c, digest );
}

void sha1_test()
{

#if defined(_DEBUG)

	#define bufSize 2000
	char buf[ bufSize ];

	printf("# test sha1 test vectors\n");

	char *x, *y;
	sha1Digest digest;

	x = "abc";
	y = "A9993E364706816ABA3E25717850C26C9CD0D89D";
	sha1_digest( digest, x, strlen(x) );
#if defined(_WINDOWS)
	sprintf_s( buf, bufSize,
#else
	sprintf( buf, 
#endif
		"%X%X%X%X%X", (unsigned int)digest[0], (unsigned int)digest[1], (unsigned int)digest[2], (unsigned int)digest[3], (unsigned int)digest[4] );
	if( strcmp( y, buf ) != 0 )
	{
		printf( "%s -> %s (should be %s)\n", x, buf, y );
		ASSERT( 0 );
	}
		
	x = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	y = "84983E441C3BD26EBAAE4AA1F95129E5E54670F1";
	sha1_digest( digest, x, strlen(x) );
#if defined(_WINDOWS)
	sprintf_s( buf, bufSize,
#else
	sprintf( buf, 
#endif
		"%X%X%X%X%X", (unsigned int)digest[0], (unsigned int)digest[1], (unsigned int)digest[2], (unsigned int)digest[3], (unsigned int)digest[4] );
	if( strcmp( y, buf ) != 0 )
	{
		printf( "%s -> %s (should be %s)\n", x, buf, y );
		ASSERT( 0 );
	}

#endif //_DEBUG

}
