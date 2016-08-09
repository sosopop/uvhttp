#include "unittest.h"
#ifdef WIN32
#include <Windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi")
#endif

void run_shell( const char* cmd)
{
#ifdef WIN32
    WinExec( cmd, SW_SHOW);
#endif
}

void app_path( char* path, unsigned int path_size, const char* path_append)
{
#ifdef WIN32
    GetModuleFileName( 0, path, path_size);
    PathRemoveFileSpec( path);
    PathAppend( path, path_append);
#endif
}
