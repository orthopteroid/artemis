#include "rc4.h"

// Key Scheduling Algorithm 
// Input: state - the state used to generate the keystream
//        key - Key to use to initialize the state 
//        len - length of key in bytes  
static void ksa( byteptr state, byteptr key, int len )
{
   for( int i=0; i < 256; ++i ) { state[i] = i; }
   int j=0;
   for( int i=0; i < 256; ++i )
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
static void prga( byteptr state, byteptr out, int len )
{  
   int i=0, j=0;   
   for( int x=0; x < len; ++x)  {
      i = (i + 1) % 256; 
      j = (j + state[i]) % 256; 
      int t = state[i]; 
      state[i] = state[j]; 
      state[j] = t;
	  if( out ) { out[x] ^= state[(state[i] + state[j]) % 256]; }
   }   
}  

void rc4( byteptr key, int keylen, int skip, byteptr txt, int txtlen )
{
	byte state[256];
	ksa( state, key, keylen );
	prga( state, 0, skip );
	prga( state, txt, txtlen );
}
