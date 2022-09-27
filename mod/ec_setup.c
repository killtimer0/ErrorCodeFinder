#include "../ecf.h"
#include <setupapi.h>

static ErrorCodeItem ec_setup_items[] = {
    REGCODE(ERROR_EXPECTED_SECTION_NAME)
    REGCODE(ERROR_BAD_SECTION_NAME_LINE)
    REGCODE(ERROR_SECTION_NAME_TOO_LONG)
    REGCODE(ERROR_GENERAL_SYNTAX)

    REGCODE(ERROR_WRONG_INF_STYLE)
    REGCODE(ERROR_SECTION_NOT_FOUND)
    REGCODE(ERROR_LINE_NOT_FOUND)
};

static LPTSTR setup_code2msg(DWORD dwCode)
{
    LPTSTR result = NULL;

    if (hModCertCli)
    {
        UINT uID = 0;
        SIZE_T n, i;

        for (i = 0; i < ARRAYSIZE(ec_setup_items); ++i)
        {
            if (dwCode == ec_setup_items[i].dwCode)
            {
                uID = i + 212;
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

static void setup_freemsg(LPTSTR lpMessage)
{
    LocalFree(lpMessage);
}

void EcRegSetup(ErrorCodeSet *pecs)
{
    pecs->lpName = TEXT("Setup");
    pecs->pfnMessageFromCode = setup_code2msg;
    pecs->pfnFreeMessage = setup_freemsg;
    pecs->items = ec_setup_items;
    pecs->dwCodeCount = ARRAYSIZE(ec_setup_items);
}

void EcUnregSetup()
{
    /* Ignore */
}

