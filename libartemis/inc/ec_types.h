#ifndef PW_TYPES_H
#define PW_TYPES_H

typedef unsigned char		byte;	/*  8 bit */
typedef unsigned short		word16;	/* 16 bit */
typedef unsigned int		word32;	/* 32 bit */

typedef byte*		byteptr;
typedef const byte*	constbyteptr;

typedef word32*		word32ptr;
typedef word16*		word16ptr;

// helper types for pasing around pointers to arrays
typedef byteptr*	byteptrarr;
typedef word32ptr*	word32ptrarr;
typedef word16ptr*	word16ptrarr;

#endif // PW_TYPES_H
