#ifndef PW_TYPES_H
#define PW_TYPES_H

typedef unsigned char		byte;	/*  8 bit */
typedef unsigned short		word16;	/* 16 bit */
typedef unsigned int		word32;	/* 32 bit */

typedef byte*		byteptr;
typedef const byte*	constbyteptr;

typedef word32*		word32ptr;
typedef word16*		word16ptr;

// an array is a sequence of a type
// a table is an array of pointers to a type
typedef byteptr*	bytetbl;
typedef word32ptr*	word32tbl;
typedef word16ptr*	word16tbl;

#endif // PW_TYPES_H
