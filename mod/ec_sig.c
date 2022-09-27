#include "../ecf.h"
#include <signal.h>

static ErrorCodeItem ec_sig_items[] = {
    REGCODE(SIGINT)
    REGCODE(SIGILL)
    REGCODE(SIGFPE)
    REGCODE(SIGSEGV)
    REGCODE(SIGTERM)
    REGCODE(SIGBREAK)
    REGCODE(SIGABRT)
    REGCODE(SIGABRT_COMPAT)
};

static LPTSTR sig_code2msg(DWORD dwCode)
{
    switch (dwCode)
    {
        case SIGINT:
        return TEXT("interrupt");

        case SIGILL:
        return TEXT("illegal instruction - invalid function image");

        case SIGFPE:
        return TEXT("floating point exception");

        case SIGSEGV:
        return TEXT("segment violation");

        case SIGTERM:
        return TEXT("Software termination signal from kill");

        case SIGBREAK:
        return TEXT("Ctrl-Break sequence");

        case SIGABRT:
        return TEXT("abnormal termination triggered by abort call");

        case SIGABRT_COMPAT:
        return TEXT("SIGABRT compatible with other platforms, same as SIGABRT");

        default:
        return TEXT("Unknown signal");
    }
}

static void sig_freemsg(LPTSTR lpMessage)
{
    /* Ignore */
}

void EcRegSig(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("signal");
    pecs->pfnMessageFromCode = sig_code2msg;
    pecs->pfnFreeMessage = sig_freemsg;
    pecs->items = ec_sig_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_sig_items);
}

void EcUnregSig()
{
    /* Ignore */
}

