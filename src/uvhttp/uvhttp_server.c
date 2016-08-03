#include "uvhttp_server.h"
#include <stdarg.h>

int uvhttp_server_set_option( 
    uvhttp_server server,
    uvhttp_server_option option,
    ...
    )
{
    int ret = 0;
    va_list ap;
    va_start(ap, option);
    switch( option)
    {
    case UVHTTP_SRV_OPT_USER_DATA:
        break;
    case UVHTTP_SRV_OPT_SESSION_BEGIN_CB:
        break;
    case UVHTTP_SRV_OPT_END_CB:
        break;
    }
    va_end(ap);
    return ret;
}