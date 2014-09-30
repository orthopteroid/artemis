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

DLLDECL int library_uri_shareinfo( word16* pShares, word16* pThreshold, byteptr szShare );
DLLDECL int library_uri_sharetype( word16* pType, byteptr szShare );

DLLDECL int library_uri_validate( byteptr* invalidBoolArr_out_opt, byteptr szARrecord, byteptr szSRecordArr_opt );
	/* -5 srecord sig fail, -4 srecord topic fail, -3 arecord sig fail, -2 arecord topic fail, -1 api, 0 ok */

DLLDECL int library_uri_topic( byteptr* topic_out, byteptr szShare );
DLLDECL int library_uri_clue( byteptr* clue_out, byteptr szShare );
DLLDECL int library_uri_location( byteptr* location_out, byteptr szShare );

DLLDECL int library_uri_encoder( byteptr* arecord_out, byteptr* srecordArr_out, int shares, int threshold, byteptr szLocation, byteptr clueArr, byteptr message );
DLLDECL int library_uri_decoder( byteptr* message_out, byteptr arecord, byteptr srecordArr );

DLLDECL void library_free( byteptr* object );

// status functions

DLLDECL int library_isdebug();
DLLDECL int library_isdemo();
DLLDECL int library_istest();
DLLDECL void library_settest();
DLLDECL word32 library_vmajor();
DLLDECL word32 library_vminor();
DLLDECL word32 library_keylength();

void library_test();

#endif // _library_H_
