#include <Core/System/SystemTools.hpp>
#include <Core/Support/Tools.hpp>
#include <psapi.h>
#include <Shlobj.h>
#include <stdio.h>

namespace Core
{
    // public
    //-------------------------------------------------------------------------
    Bitness SystemTools::GetProcessBitness( HANDLE process )
    {
        SYSTEM_INFO system_info;

        // check if everything is 32bit
        GetNativeSystemInfo(&system_info);

        // select cpu architecture
        switch( system_info.wProcessorArchitecture )
        {
        // 64bit
        case PROCESSOR_ARCHITECTURE_AMD64:
        case PROCESSOR_ARCHITECTURE_IA64:
            {
                BOOL is_wow64;

                // get wow64 (emulated 32) status
                if( process==NULL || !IsWow64Process(process, &is_wow64) )
                    return BITNESS_UNKNOWN;

                // if emulated then 32 else 64
                return is_wow64 ? BITNESS_32 : BITNESS_64;
            }

        // 32bit
        case PROCESSOR_ARCHITECTURE_INTEL:
            return BITNESS_32;
        }

        return BITNESS_UNKNOWN;
    }

    Bool SystemTools::GetProcessDirectory( PathString& path )
    {
        if( GetModuleFileName( NULL, path.EditMemory(), path.GetLimit() ) == 0 )
            return false;

        Tools::StripFileName(path.EditMemory());
        return true;
    }

    UHuge SystemTools::GetTickTime()
    {
        UHuge counter = 0;
        QueryPerformanceCounter((LARGE_INTEGER*)&counter);
        return counter;
    }

    UHuge SystemTools::GetTickFrequency()
    {
        UHuge frequency = 0;
        QueryPerformanceFrequency((LARGE_INTEGER*)&frequency);
        return frequency;
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::AccessUserPath( PathString& path, const WCHAR* app_name )
    {
        PathString  wpath;
        Bool        status = false;

        // get appdata folder
        if( SHGetFolderPath( NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, wpath.EditMemory() ) != S_OK )
            return false;

        // build save path
        if( swprintf_s( path.EditMemory(), path.GetCount(), L"%s\\%s", wpath.GetMemory(), app_name ) <= 0 )
            return false;

        // ensure save path exists
        if( !CreateDirectory(path.GetMemory(), NULL) &&  GetLastError() != ERROR_ALREADY_EXISTS )
            return false;

        return true;
    }

    //-------------------------------------------------------------------------
    Bool SystemTools::EnableAutoStart( const WCHAR* name, const PathString& path, Bool enable )
    {
        const WCHAR* REGISTRY_PATH = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
        Bool status = false;
        HKEY hkey;

        // if enabling
        if(enable)
        {
            // open/create key
            if(RegCreateKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, NULL, 0, KEY_SET_VALUE, NULL, &hkey, NULL) == ERROR_SUCCESS)
            {
                // set value
                if(RegSetValueEx (hkey, name, 0, REG_SZ, (Byte*)path.GetMemory(), (DWORD)(wcslen(path.GetMemory()) * sizeof(WCHAR))) == ERROR_SUCCESS)
                    status = true;

                // close key
                RegCloseKey(hkey);
            }
        }
        // open key
        else if( RegOpenKey(HKEY_CURRENT_USER, REGISTRY_PATH, &hkey) == ERROR_SUCCESS )
        {
            // delete value
            if( RegDeleteValue(hkey, name) == ERROR_SUCCESS )
                status = true;

            // close key
            RegCloseKey(hkey);
        }

        return status;
    }
}
