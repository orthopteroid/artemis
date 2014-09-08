// Copyright 2014 John Howard (orthopteroid@gmail.com)

#ifndef _library_H_
#define _library_H_

#include "ec_types.h"

DLLDECL word32 library_init();

DLLDECL void library_cleanup();

DLLDECL int library_isdebug();

DLLDECL int library_isdemo();

DLLDECL word32 library_vmajor();
DLLDECL word32 library_vminor();
DLLDECL word32 library_keylength();

#endif // _library_H_
