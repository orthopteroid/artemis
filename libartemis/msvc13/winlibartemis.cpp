// winlibartemis.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "winlibartemis.h"


// This is an example of an exported variable
WINLIBARTEMIS_API int nwinlibartemis=0;

// This is an example of an exported function.
WINLIBARTEMIS_API int fnwinlibartemis(void)
{
	return 42;
}

// This is the constructor of a class that has been exported.
// see winlibartemis.h for the class definition
Cwinlibartemis::Cwinlibartemis()
{
	return;
}
