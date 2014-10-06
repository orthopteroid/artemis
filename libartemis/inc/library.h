// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _library_H_
#define _library_H_

#include "platform.h"
#include "ec_types.h"

// nonzero function return values indicate an error
// functions that return values through _out args those args to be passed to library_free(...) to prevent a memory leak
// Arr args are '\n' delimited strings

DLLDECL word32 library_init();
DLLDECL void library_cleanup();

DLLDECL int library_uri_info( word16* pType, word16* pShares, word16* pThreshold, byteptr szShare );

DLLDECL int library_uri_validate( byteptr* invalidBoolArr_out, byteptr szLocation, byteptr szRecordArr );

DLLDECL int library_uri_topic( byteptr* topic_out, byteptr szShare );
DLLDECL int library_uri_clue( byteptr* clue_out, byteptr szShare );
DLLDECL int library_uri_location( byteptr* location_out, byteptr szShare );

DLLDECL int library_uri_encoder( byteptr* recordArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, byteptr message );
DLLDECL int library_uri_decoder( byteptr* message_out, byteptr szLocation, byteptr recordArr );

DLLDECL void library_free( byteptr* object );

// status functions

DLLDECL int library_isdebug();
DLLDECL int library_isdemo();
DLLDECL int library_istest();
DLLDECL void library_settest();
DLLDECL word32 library_vmajor();
DLLDECL word32 library_vminor();
DLLDECL word32 library_keylength();

DLLDECL const char* library_rclookup( int rc );

void library_test();

#endif // _library_H_
