// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CORE_H
#define _AR_CORE_H

#include "ar_types.h"

int ar_core_check_arecord( byteptr szLocation, arAuthptr arecord );
int ar_core_check_srecord( byteptr szLocation, arShareptr srecord );
int ar_core_check_recordset( byteptr szLocation, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords );

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, byte numThres, byteptr bytes, word16 bytelen, bytetbl clueTbl, byteptr location );

int ar_core_decrypt( byteptr* buf_out, byteptr szLocation, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords );

void ar_core_test();

#endif // _AR_CORE_H
