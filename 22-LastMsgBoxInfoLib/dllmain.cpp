#include <Windows.h>
#include <stdio.h>
#include "MinHook.h"

#if defined _M_X64
#pragma comment(lib, "libMinHook.x64.lib")
#elif defined _M_IX86
#pragma comment(lib, "libMinHook.x86.lib")
#endif

typedef int (WINAPI *CREATEPROCESSA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
                                     BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);

typedef int (WINAPI *CREATEPROCESSW)(LPCWSTR, LPWSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES,
                                     BOOL, DWORD, LPVOID, LPCWSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);

// Pointer for calling original MessageBoxW.
CREATEPROCESSA fpCreateProcessA = NULL;
CREATEPROCESSW fpCreateProcessW = NULL;

char buffer[1024];
wchar_t w_buffer[1024];
int WINAPI DetourCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine,
                                LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles,
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment,
                                LPCSTR lpCurrentDirectory,
                                LPSTARTUPINFOA lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation) {
  memset(buffer, 0, 1024);
  sprintf(buffer, "MinHookDemo:%s", lpApplicationName);
  OutputDebugStringA(buffer);
  if (MessageBoxA(NULL, buffer, "Allowed Process To Be Launched?", MB_YESNO) == IDYES)
  {
  return fpCreateProcessA(lpApplicationName, lpCommandLine,
                        lpProcessAttributes, lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags,
                        lpEnvironment,
                        lpCurrentDirectory,
                        lpStartupInfo,
                        lpProcessInformation);
  }
  return 0;
}
int WINAPI DetourCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine,
                                LPSECURITY_ATTRIBUTES lpProcessAttributes,
                                LPSECURITY_ATTRIBUTES lpThreadAttributes,
                                BOOL bInheritHandles,
                                DWORD dwCreationFlags,
                                LPVOID lpEnvironment,
                                LPCWSTR lpCurrentDirectory,
                                LPSTARTUPINFOA lpStartupInfo,
                                LPPROCESS_INFORMATION lpProcessInformation) {
  memset(w_buffer, 0, 1024*2);
  swprintf(w_buffer, L"MinHookDemo:%s", lpApplicationName);
  OutputDebugStringW(w_buffer);
  if (MessageBoxW(NULL, w_buffer, L"Allowed Process To Be Launched?", MB_YESNO) == IDYES)
  {
  return fpCreateProcessW(lpApplicationName, lpCommandLine,
                        lpProcessAttributes, lpThreadAttributes,
                        bInheritHandles,
                        dwCreationFlags,
                        lpEnvironment,
                        lpCurrentDirectory,
                        lpStartupInfo,
                        lpProcessInformation);
  }
  return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{

	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		// Initialize MinHook.
		if (MH_Initialize() != MH_OK)
		{
			return FALSE;
		}

		if (MH_CreateHook(&CreateProcessA, &DetourCreateProcessA, 
			reinterpret_cast<void**>(&fpCreateProcessA)) != MH_OK)
		{
			return FALSE;
		}

		if (MH_EnableHook(&CreateProcessA) != MH_OK)
		{
			return FALSE;
		}
        if (MH_CreateHook(&CreateProcessW, &DetourCreateProcessW, 
			reinterpret_cast<void**>(&fpCreateProcessW)) != MH_OK)
		{
			return FALSE;
		}

		if (MH_EnableHook(&CreateProcessW) != MH_OK)
		{
			return FALSE;
		}
		return TRUE;
	case DLL_PROCESS_DETACH:
		OutputDebugStringA("Detaching dll ...");
		if (MH_DisableHook(&CreateProcessA) != MH_OK)
		{
			return FALSE;
		}
        if (MH_DisableHook(&CreateProcessW) != MH_OK)
		{
			return FALSE;
		}
		// Uninitialize MinHook.
		if (MH_Uninitialize() != MH_OK)
		{
			return FALSE;
		}
		break;
	}
	return TRUE;
}
