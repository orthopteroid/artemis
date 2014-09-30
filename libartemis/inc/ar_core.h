// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CORE_H
#define _AR_CORE_H

#include "ar_types.h"
#include "ec_types.h"
#include "ec_vlong.h"
#include "ec_field.h"
#include "ec_crypt.h"

int ar_core_create( arAuthptr* arecord_out, arSharetbl* srecordtbl_out, word16 numShares, word16 numThres, byteptr bytes, word16 bytelen, bytetbl clueTbl, byteptr location );
	/* -3 borked (try again), -2 bounds, -1 api, 0 OK */

int ar_core_decrypt( byteptr* buf_out, arAuthptr arecord, arSharetbl srecordtbl, word16 numSRecords );
	/* -9 malloc, -8 decrypt, -7 authsig, -6 sharesig, -5 sharetopic, -4 authtopic, -3 too few shares, -2 bounds, -1 api, 0 OK */

int ar_core_check_topic( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords );
	/* buf is atleast numSRecords big to hold the topic crosschecks. 0 == ok, 0xFF fail */
	/* -3 SRecord fail (check buf for individual codes), -2 ARecord fail, -1 api, 0 OK */

int ar_core_check_signature( byteptr buf_opt, arAuthptr arecord, arSharetbl srecordtbl_opt, word16 numSRecords );
	/* buf is atleast numSRecords big to hold the signature crosschecks. 0 == ok, 0xFF fail */
	/* -3 SRecord fail (check buf for individual codes), -2 ARecord fail, -1 api, 0 OK */

void ar_core_test();

#endif // _AR_CORE_H
