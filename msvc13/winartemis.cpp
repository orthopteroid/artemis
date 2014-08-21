// winartemis.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern "C" int ar_main(int argc, char **argv);

int _tmain(int argc, _TCHAR* argv[])
{
	return ar_main( argc, argv );
}

