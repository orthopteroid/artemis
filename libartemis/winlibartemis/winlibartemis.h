// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WINLIBARTEMIS_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WINLIBARTEMIS_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINLIBARTEMIS_EXPORTS
#define WINLIBARTEMIS_API __declspec(dllexport)
#else
#define WINLIBARTEMIS_API __declspec(dllimport)
#endif

// This class is exported from the winlibartemis.dll
class WINLIBARTEMIS_API Cwinlibartemis {
public:
	Cwinlibartemis(void);
	// TODO: add your methods here.
};

extern WINLIBARTEMIS_API int nwinlibartemis;

WINLIBARTEMIS_API int fnwinlibartemis(void);
