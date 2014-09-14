// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _library_H_
#define _library_H_

#include "platform.h"
#include "ec_types.h"

DLLDECL word32 library_init();

DLLDECL int library_uri_encoder( byteptr* sharesBarArr_out, int shares, int threshold, byteptr location, byteptr clueBarArr, byteptr message );
DLLDECL int library_uri_decoder( byteptr* message_out, byteptr location, byteptr sharesNLArr );
DLLDECL int library_uri_field( byteptr* field_out, byteptr szShare, byteptr szField, word16 uFieldNum );

DLLDECL void library_free( byteptr* object );

DLLDECL void library_cleanup();

DLLDECL int library_isdebug();

DLLDECL int library_isdemo();

DLLDECL word32 library_vmajor();
DLLDECL word32 library_vminor();
DLLDECL word32 library_keylength();

void library_test();

#endif // _library_H_
