#ifndef PW_TYPES_H
#define PW_TYPES_H

typedef unsigned char	byte;	/*  8 bit */
typedef unsigned short	word16;	/* 16 bit */

#if defined(_WIN32)

	typedef unsigned long	word32;	/* 32 bit */

#else

	typedef unsigned int	word32;	/* 32 bit */

#endif

typedef byte*	byteptr;
typedef word16*	word16ptr;

#endif // PW_TYPES_H
