// privilege stuff from https://support.microsoft.com/en-us/kb/131065
#include <YoloMouse/Loader/Inject/InjectEnvironment.hpp>
#include <YoloMouse/Share/Constants.hpp>

namespace Yolomouse
{
    // local
    namespace
    {
        //---------------------------------------------------------------------
        BOOL _SetPrivileges( HANDLE hToken, LPCTSTR Privilege, BOOL bEnablePrivilege )
        {
            TOKEN_PRIVILEGES tp = {0};
            LUID             luid;
            TOKEN_PRIVILEGES tpPrevious;
            DWORD            cbPrevious = sizeof(TOKEN_PRIVILEGES);

            if(!LookupPrivilegeValue( NULL, Privilege, &luid )) 
                return FALSE;

            // first pass. get current privilege setting
            tp.PrivilegeCount           = 1;
            tp.Privileges[0].Luid       = luid;
            tp.Privileges[0].Attributes = 0;

            AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tp,
                sizeof(TOKEN_PRIVILEGES),
                &tpPrevious,
                &cbPrevious);
            if (GetLastError() != ERROR_SUCCESS)
                return FALSE;

            // second pass. set privilege based on previous setting
            tpPrevious.PrivilegeCount       = 1;
            tpPrevious.Privileges[0].Luid   = luid;

            if(bEnablePrivilege)
                tpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
            else
                tpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tpPrevious.Privileges[0].Attributes);

            AdjustTokenPrivileges(
                hToken,
                FALSE,
                &tpPrevious,
                cbPrevious,
                NULL,
                NULL);
            if( GetLastError() != ERROR_SUCCESS )
                return false;

            return true;
        }
    }


    // public
    //-------------------------------------------------------------------------
    InjectEnvironment::InjectEnvironment():
        _hprivileges(NULL)
    {
    }

    InjectEnvironment::~InjectEnvironment()
    {
        ASSERT( !IsInitialized() );
    }

    //-------------------------------------------------------------------------
    void InjectEnvironment::Initialize()
    {
        ASSERT( !IsInitialized() );

        // initialize privileges, can fail
        _InitializePrivileges();
    }

    void InjectEnvironment::Shutdown()
    {
        ASSERT( IsInitialized() );

        // shutdown privileges if initialized
        _ShutdownPrivileges();

        // reset fields
        _hprivileges = NULL;
    }

    //-------------------------------------------------------------------------
    Bool InjectEnvironment::IsInitialized() const
    {
        return _hprivileges != NULL;
    }

    // private
    //-------------------------------------------------------------------------
    Bool InjectEnvironment::_InitializePrivileges()
    {
        /*
        // open thread access token
        if( !OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &_hprivileges) )
        {
            if( GetLastError() == ERROR_NO_TOKEN )
            {
                if( !ImpersonateSelf(SecurityImpersonation) )
                    return false;

                if( !OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &_hprivileges) )
                    return false;
            }
            else
                return false;
        }

        // enable SeDebugPrivilege
        if(!_SetPrivileges(_hprivileges, SE_DEBUG_NAME, TRUE))
        {
            // close token handle
            CloseHandle(_hprivileges);

            // indicate failure
            return false;
        }

        return true;
        */
        
        TOKEN_PRIVILEGES token_privileges = {0};
        Bool             status = false;

        // open token for our process
        if( OpenProcessToken( GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &_hprivileges ) != TRUE )
            return false;

        // describe requested privileges
        token_privileges.PrivilegeCount = 1;
        token_privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        // get debug privilege
        if( LookupPrivilegeValue( NULL, SE_DEBUG_NAME, &token_privileges.Privileges[0].Luid ) != TRUE )
            return false;

        // update our privilege
        return AdjustTokenPrivileges(_hprivileges, FALSE, &token_privileges, 0, NULL, NULL) == TRUE;
    }

    void InjectEnvironment::_ShutdownPrivileges()
    {
        // disable SeDebugPrivilege
        _SetPrivileges(_hprivileges, SE_DEBUG_NAME, FALSE);

        // close token handle
        CloseHandle(_hprivileges);
    }
}
