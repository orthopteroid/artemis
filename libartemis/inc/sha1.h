#ifndef _SHA1_H_
#define _SHA1_H_

#include "ec_types.h"

#define HW 5

typedef union _sha1Digest {
	word32	w32[5];
	word16	w16[10];
	byte	b8[20];
} sha1Digest;

typedef struct {
	byte	buffer[64]; // requires proper alignment 
	sha1Digest digest;
	word32	count[2];
} sha1_context;

// digest stored in big endian, i think
//typedef word32 sha1Digest [5];

void sha1_initial( sha1_context * c );
void sha1_process( sha1_context * c, unsigned char * data, unsigned len );
void sha1_final( sha1_context * c, sha1Digest* pDigest );
void sha1_clear( sha1_context * c );

void sha1_digest( sha1Digest* pDigest, byteptr bytes, size_t len );

void sha1_test();

#endif // _SHA1_H_
