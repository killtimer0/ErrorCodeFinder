#include "../ecf.h"
#include <wininet.h>

static ErrorCodeItem ec_http_items[] = {
    REGCODE(HTTP_STATUS_CONTINUE)
    REGCODE(HTTP_STATUS_SWITCH_PROTOCOLS)

    REGCODE(HTTP_STATUS_OK)
    REGCODE(HTTP_STATUS_CREATED)
    REGCODE(HTTP_STATUS_ACCEPTED)
    REGCODE(HTTP_STATUS_PARTIAL)
    REGCODE(HTTP_STATUS_NO_CONTENT)
    REGCODE(HTTP_STATUS_RESET_CONTENT)
    REGCODE(HTTP_STATUS_PARTIAL_CONTENT)

    REGCODE(HTTP_STATUS_AMBIGUOUS)
    REGCODE(HTTP_STATUS_MOVED)
    REGCODE(HTTP_STATUS_REDIRECT)
    REGCODE(HTTP_STATUS_REDIRECT_METHOD)
    REGCODE(HTTP_STATUS_NOT_MODIFIED)
    REGCODE(HTTP_STATUS_USE_PROXY)
    REGCODE(HTTP_STATUS_REDIRECT_KEEP_VERB)

    REGCODE(HTTP_STATUS_BAD_REQUEST)
    REGCODE(HTTP_STATUS_DENIED)
    REGCODE(HTTP_STATUS_PAYMENT_REQ)
    REGCODE(HTTP_STATUS_FORBIDDEN)
    REGCODE(HTTP_STATUS_NOT_FOUND)
    REGCODE(HTTP_STATUS_BAD_METHOD)
    REGCODE(HTTP_STATUS_NONE_ACCEPTABLE)
    REGCODE(HTTP_STATUS_PROXY_AUTH_REQ)
    REGCODE(HTTP_STATUS_REQUEST_TIMEOUT)
    REGCODE(HTTP_STATUS_CONFLICT)
    REGCODE(HTTP_STATUS_GONE)
    REGCODE(HTTP_STATUS_LENGTH_REQUIRED)
    REGCODE(HTTP_STATUS_PRECOND_FAILED)
    REGCODE(HTTP_STATUS_REQUEST_TOO_LARGE)
    REGCODE(HTTP_STATUS_URI_TOO_LONG)
    REGCODE(HTTP_STATUS_UNSUPPORTED_MEDIA)
    REGCODE(HTTP_STATUS_RETRY_WITH)

    REGCODE(HTTP_STATUS_SERVER_ERROR)
    REGCODE(HTTP_STATUS_NOT_SUPPORTED)
    REGCODE(HTTP_STATUS_BAD_GATEWAY)
    REGCODE(HTTP_STATUS_SERVICE_UNAVAIL)
    REGCODE(HTTP_STATUS_GATEWAY_TIMEOUT)
    REGCODE(HTTP_STATUS_VERSION_NOT_SUP)
};

static LPTSTR http_code2msg(DWORD dwCode)
{
    LPTSTR result = NULL;

    if (hModCertCli)
    {
        UINT uID = 0;
        SIZE_T i;

        for (i = 0; i < ARRAYSIZE(ec_http_items); ++i)
        {
            if (dwCode == ec_http_items[i].dwCode)
            {
                uID = i + 248;
                break;
            }
        }

        if (0 != uID)
        {
            result = (LPTSTR)LocalAlloc(LPTR, MAX_MSG_LEN * sizeof (TCHAR));
            if (result)
            {
                LoadString(hModCertCli, uID, result, MAX_MSG_LEN);
            }
        }
    }

    return result;
}

static void http_freemsg(LPTSTR lpMessage)
{
    LocalFree(lpMessage);
}

void EcRegHttp(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("HTTP");
    pecs->pfnMessageFromCode = http_code2msg;
    pecs->pfnFreeMessage = http_freemsg;
    pecs->items = ec_http_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_http_items);
}

void EcUnregHttp()
{
    /* Ignore */
}

