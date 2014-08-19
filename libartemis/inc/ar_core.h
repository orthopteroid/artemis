// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _AR_CORE_H
#define _AR_CORE_H

#include "ar_types.h"
#include "ec_types.h"
#include "ec_vlong.h"
#include "ec_field.h"
#include "ec_crypt.h"

DLLDECL 
int ar_core_create( arAuth* pARecord, arShareptr* pSRecordArr, word16 numShares, word16 numThres, byteptr bytes, word16 bytelen, byteptr* clueArr );
	/* -2 bounds, -1 api, 0 OK */

DLLDECL 
int ar_core_decrypt( byteptr buf, word16 buflen, arAuth* pARecord, arShareptr* pSRecordArr, word16 numSRecords );
	/* -5 decrypt, -4 authsig, -3 sharesig, -6 topichash, -7 too few shares, -2 bounds, -1 api, 0 OK */

void ar_core_test();

#endif // _AR_CORE_H
