// winartemis.cpp : Defines the entry point for the console application.
//

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "stdafx.h"

extern "C" int ar_main(int argc, char **argv);

int _tmain(int argc, _TCHAR* argv[])
{

#if defined(_DEBUG)
	// dump leaks on exit
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	return ar_main( argc, argv );
}

