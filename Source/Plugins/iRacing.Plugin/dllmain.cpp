// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <string>
#include <iostream>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    printf("DllMain() called\n");
    return TRUE;
}

extern "C"
{
    int __declspec(dllexport) __stdcall getPluginVersion()
    {
        printf("getPluginVersion() called\n");
        return 1;
    }
}