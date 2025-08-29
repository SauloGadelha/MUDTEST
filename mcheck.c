#include "mcheck.h"

int mcheck (void (*abortfunc)(enum mcheck_status mstatus))
{
    return 0;
}

int mcheck_pedantic (void (*abortfunc)(enum mcheck_status mstatus))
{
    return 0;
}

void mcheck_check_all ()
{
}

enum mcheck_status mprobe (void *ptr)
{
    return MCHECK_OK;
}