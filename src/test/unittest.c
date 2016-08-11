#include "unittest.h"
#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")
#include <process.h>
#endif

void run_thread( 
    void* param
    )
{
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    BOOL bRet = FALSE;
    char* path = (char*)param;
    si.cb= sizeof(si);
    si.wShowWindow = SW_NORMAL;

    bRet = CreateProcess( NULL, (char*)param , NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
    if ( bRet)
    {
        CloseHandle( pi.hThread);
        WaitForSingleObject( pi.hProcess, INFINITE);
    }
    free(param);
}

void* run_shell( const char* cmd)
{
#ifdef WIN32
   return (void*)_beginthread( run_thread, 0, strdup( cmd));
#endif
}

void wait_run(void* shellptr)
{
    if ( shellptr)
    {
        WaitForSingleObject( shellptr, INFINITE);
        CloseHandle( shellptr);
    }
}

void app_path( char* path, unsigned int path_size, const char* path_append)
{
#ifdef WIN32
    GetModuleFileName( 0, path, path_size);
    PathRemoveFileSpec( path);
    PathAppend( path, path_append);
#endif
}

void del_file( const char* path )
{
    DeleteFile( path);
}
