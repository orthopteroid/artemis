// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _ar_util_H_
#define _ar_util_H_

#include "ec_types.h"

// text conversion/copy routines that write to memy in big endian for printing or hashing
// cant use these for the sha1 digest as it is already big endian

int ar_util_12Bto6B( byteptr out, word16 in );
int ar_util_6Bto12B( word16ptr out, byteptr in );

// BA to BA

int ar_util_8BAto4BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize );
int ar_util_4BAto8BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize );

int ar_util_8BAto6BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize );
int ar_util_6BAto8BA( size_t* deltalen, byteptr buf, size_t bufsize, byteptr in, size_t insize );

int ar_util_16BAto4BA( size_t* deltalen, byteptr   buf, size_t bufsize, word16ptr in, size_t insize );
int ar_util_4BAto16BA( size_t* deltalen, word16ptr buf, size_t bufsize, byteptr   in, size_t insize );

int ar_util_16BAto8BA( size_t* deltalen, byteptr   buf, size_t bufsize, word16ptr in, size_t insize );
int ar_util_8BAto16BA( size_t* deltalen, word16ptr buf, size_t bufsize, byteptr   in, size_t insize );

// mixed BA and BZ

int ar_util_8BAto4BZ( byteptr buf, size_t bufsize, byteptr in, size_t insize );

int ar_util_16BAto4BZ( byteptr buf, size_t bufsize, word16ptr in, size_t insize );

// BZ to BZ

int ar_util_8BZto4BZ( byteptr buf, size_t bufsize, byteptr in );
int ar_util_4BZto8BZ( byteptr buf, size_t bufsize, byteptr in );

int ar_util_8BZto6BZ( byteptr buf, size_t bufsize, byteptr in );
int ar_util_6BZto8BZ( byteptr buf, size_t bufsize, byteptr in );

////////

int ar_util_strcat( byteptr dst, size_t dstsize, byteptr src );
int ar_util_strncat( byteptr dst, size_t dstsize, byteptr src, size_t srcsize );

////////

word32 ar_util_rnd32();

////////

void ar_util_test();

#endif // _ar_util_H_
