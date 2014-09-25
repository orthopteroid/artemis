// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _library_H_
#define _library_H_

#include "platform.h"
#include "ec_types.h"

// nonzero function return values indicate an error

DLLDECL word32 library_init();
DLLDECL void library_cleanup();

DLLDECL int library_uri_shareinfo( word16* pShares, word16* pThreshold, byteptr szShare );
DLLDECL int library_uri_sharetype( word16* pType, byteptr szShare );

// functions that return values through _out args those args to be passed to library_free(...) to prevent a memory leak
// Arr args are '\n' delimited strings

DLLDECL int library_uri_encoder( byteptr* sharesArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, size_t clueArrLen, byteptr message );
DLLDECL int library_uri_decoder( byteptr* message_out, byteptr szLocation, byteptr shareArr, size_t shareArrLen );
DLLDECL int library_uri_field( byteptr* field_out, byteptr szShare, byteptr szField, word16 uFieldNum );
DLLDECL int library_uri_clue( byteptr* clue_out, byteptr szShare );
DLLDECL int library_uri_location( byteptr* location_out, byteptr szShare );

DLLDECL void library_free( byteptr* object );

// status functions

DLLDECL int library_isdebug();
DLLDECL int library_isdemo();
DLLDECL word32 library_vmajor();
DLLDECL word32 library_vminor();
DLLDECL word32 library_keylength();

void library_test();

#endif // _library_H_
