// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CORE_H
#define _AR_CORE_H

#include "ar_types.h"
//#include "ec_types.h"
//#include "ec_vlong.h"
//#include "ec_field.h"
//#include "ec_crypt.h"

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, byte numThres, byteptr bytes, word16 bytelen, bytetbl clueTbl, byteptr location );

int ar_core_decrypt( byteptr* buf_out, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords );

int ar_core_check_topic( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords );
	/* buf is atleast numSRecords big to hold the topic crosschecks. 0 == ok, 0xFF fail */

int ar_core_check_signature( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords );
	/* buf is atleast numSRecords big to hold the signature crosschecks. 0 == ok, 0xFF fail */

void ar_core_test();

#endif // _AR_CORE_H
