#include "uvhttp.h"
#include <stdarg.h>

int uvhttp_obj_setopt(
    uvhttp_obj obj, 
    uvhttp_opt_type opt_type, 
    ...
    )
{
    int ret = 0;
    va_list ap;
    va_start(ap, opt_type);
    switch( opt_type)
    {
    case UVHTTP_SRV_OPT_USER_DATA:
        break;
    case UVHTTP_SRV_OPT_REQUEST_CB:
        break;
    case UVHTTP_SRV_OPT_END_CB:
        break;
    case UVHTTP_SRV_SESSION_OPT_USER_DATA:
        break;
    case UVHTTP_SRV_SESSION_OPT_READ_CB:
        break;
    case UVHTTP_SRV_SESSION_OPT_END_CB:
        break;
    case UVHTTP_CLT_OPT_USER_DATA:
        break;
    case UVHTTP_CLT_OPT_RESPONSE_CB:
        break;
    case UVHTTP_CLT_OPT_READ_CB:
        break;
    case UVHTTP_CLT_OPT_WRITE_CB:
        break;
    case UVHTTP_CLT_OPT_END_CB:
        break;
    }

    va_end(ap);
    return ret;
}
