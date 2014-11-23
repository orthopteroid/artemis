// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _ar_util_H_
#define _ar_util_H_

#include "ec_types.h"
#include "ec_vlong.h"

int ar_util_buildByteTbl( bytetbl* table_out, byteptr arr, size_t len );
int ar_util_checkTbl( void** tbl, size_t len );

// text conversion/copy routines that write to memy in big endian for printing or hashing
// cant use these for the sha1 digest as it is already big endian

int ar_util_30Bto6BA( byteptr out, word32 in );
int ar_util_6BAto30B( word32ptr out, byteptr in );

int ar_util_u8_hexencode( size_t* deltalen, byteptr buf, byteptr bufend, byteptr in, size_t insize );
int ar_util_u8_hexdecode( size_t* deltalen, byteptr buf, byteptr bufend, byteptr in, size_t insize );

int ar_util_u8_b64encode( size_t* deltalen, byteptr buf, byteptr bufend, byteptr in, size_t insize );
int ar_util_u8_b64decode( size_t* deltalen, byteptr buf, byteptr bufend, byteptr in, size_t insize );

int ar_util_u16_host2packet( size_t* deltalen, byteptr   buf, byteptr   bufend, word16ptr in, size_t insize );
int ar_util_u16_packet2host( size_t* deltalen, word16ptr buf, word16ptr bufend, byteptr   in, size_t insize );

int ar_util_u16_hexencode( size_t* deltalen, byteptr   buf, byteptr bufend, word16ptr   in, size_t insize );
int ar_util_u16_hexdecode( size_t* deltalen, word16ptr buf, word16ptr bufend, byteptr   in, size_t insize );

////////

int ar_util_memcpy( byteptr buf, byteptr bufend, byteptr src, size_t len );
int ar_util_strcpy( byteptr buf, byteptr bufend, byteptr src );
int ar_util_strncpy( byteptr buf, byteptr bufend, byteptr src, size_t len );
int ar_util_strcat( byteptr buf, byteptr bufend, byteptr src );
int ar_util_strncat( byteptr buf, byteptr bufend, byteptr src, size_t len );

////////

const char* ar_util_rclookup( int rc );

////////

word32 ar_util_rnd32();
word16 ar_util_rnd16();

void ar_util_rnd32_reorder( word32ptr buf, size_t len );

////////

int ar_util_txt2vl( vlPoint v, byteptr buf, size_t bufsize );
int ar_util_vl2txt( byteptr buf, byteptr bufend, vlPoint v );

int ar_util_txttow16( word16ptr pw, byteptr buf, size_t bufsize );
int ar_util_w16totxt( byteptr buf, byteptr bufend, word16ptr pw );

////////

int ar_util_istest();
void ar_util_settest();

int ar_util_isvalid7bit( byteptr szRecord );

void ar_util_test();


#endif // _ar_util_H_
