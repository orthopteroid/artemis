#ifndef _SHA1_H_
#define _SHA1_H_

#include "ec_types.h"

#define HW 5

typedef struct {
	byte	buffer[64]; // requires proper alignment 
	word32	state[5];
	word32	count[2];
} sha1_context;

// digest stored in big endian, i think
typedef word32 sha1Digest [5];

void sha1_initial( sha1_context * c );
void sha1_process( sha1_context * c, unsigned char * data, unsigned len );
void sha1_final( sha1_context * c, sha1Digest digest );

void sha1_digest( sha1Digest digest, byteptr bytes, size_t len );

void sha1_test();

#endif // _SHA1_H_
