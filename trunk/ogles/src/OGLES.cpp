// OGLES.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "OGLES.h"
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#if 0
// This is an example of an exported variable
OGLES_API int nOGLES=0;

// This is an example of an exported function.
OGLES_API int fnOGLES(void)
{
	return 42;
}

#i
// This is the constructor of a class that has been exported.
// see OGLES.h for the class definition
COGLES::COGLES()
{ 
	return; 
}
#endif
