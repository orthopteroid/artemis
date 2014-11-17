// Implementation Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "rc4.h"

// Key Scheduling Algorithm 
// Input: state - the state used to generate the keystream
//        key - Key to use to initialize the state 
//        len - length of key in bytes  
static void ksa( byteptr state, byteptr key, word32 len )
{
   for( word32 i=0; i < 256; ++i ) { state[i] = i; }
   word32 j=0;
   for( word32 i=0; i < 256; ++i )
   {
      j = (j + state[i] + key[i % len]) % 256; 
      int t = state[i]; 
      state[i] = state[j]; 
      state[j] = t; 
   }   
}

// Pseudo-Random Generator Algorithm 
// Input: state - the state used to generate the keystream 
//        out - Must be of at least "len" length
//        len - number of bytes to generate 
static void prga( byteptr state, byteptr out, word32 len )
{  
   int i=0, j=0;   
   for( word32 x=0; x < len; ++x)  {
      i = (i + 1) % 256; 
      j = (j + state[i]) % 256; 
      int t = state[i]; 
      state[i] = state[j]; 
      state[j] = t;
	  if( out ) { out[x] ^= state[(state[i] + state[j]) % 256]; }
   }   
}  

void rc4( byteptr key, word32 keylen, word32 skip, byteptr txt, word32 txtlen )
{
	byte state[256];
	ksa( state, key, keylen );
	prga( state, 0, skip );
	prga( state, txt, txtlen );
}
