/*
 * @*************************************: 
 * @FilePath: /user/C/string/dllmain.c
 * @version: 
 * @Author: dof
 * @Date: 2021-10-20 19:33:39
 * @LastEditors: dof
 * @LastEditTime: 2023-08-19 19:56:24
 * @Descripttion: 
 * @**************************************: 
 */
/* Replace "dll.h" with the name of your header */


#include "dll.h"
#include <windows.h>

DLLIMPORT void HelloWorld()
{
	MessageBox(0, "Hello World from DLL!\n", "Hi", MB_ICONINFORMATION);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}
