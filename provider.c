#include "ecf.h"

extern void EcRegHttp(ErrorCodeSet *pecs);
extern void EcUnregHttp();
extern void EcRegSetup(ErrorCodeSet *pecs);
extern void EcUnregSetup();
extern void EcRegWinhttp(ErrorCodeSet *pecs);
extern void EcUnregWinhttp();
extern void EcRegWin32(ErrorCodeSet *pecs);
extern void EcUnregWin32();
extern void EcRegDsreg(ErrorCodeSet *pecs);
extern void EcUnregDsreg();
extern void EcRegDme(ErrorCodeSet *pecs);
extern void EcUnregDme();
extern void EcRegCcm(ErrorCodeSet *pecs);
extern void EcUnregCcm();
extern void EcRegMdm(ErrorCodeSet *pecs);
extern void EcUnregMdm();
extern void EcRegNt(ErrorCodeSet *pecs);
extern void EcUnregNt();
extern void EcRegCom(ErrorCodeSet *pecs);
extern void EcUnregCom();
extern void EcRegD3d(ErrorCodeSet *pecs);
extern void EcUnregD3d();
extern void EcRegGdip(ErrorCodeSet *pecs);
extern void EcUnregGdip();
extern void EcRegErrno(ErrorCodeSet *pecs);
extern void EcUnregErrno();
extern void EcRegSig(ErrorCodeSet *pecs);
extern void EcUnregSig();
extern void EcRegBug(ErrorCodeSet *pecs);
extern void EcUnregBug();

static struct _ErrorCodeType {
    void (*pfnreg)(ErrorCodeSet *pecs);
    void (*pfnunreg)();
} types[] = {
    {EcRegHttp, EcUnregHttp},
    {EcRegSetup, EcUnregSetup},
    {EcRegWinhttp, EcUnregWinhttp},
    {EcRegWin32, EcUnregWin32},
    {EcRegDsreg, EcUnregDsreg},
    {EcRegDme, EcUnregDme},
    {EcRegCcm, EcUnregCcm},
    {EcRegMdm, EcUnregMdm},
    {EcRegNt, EcUnregNt},
    {EcRegCom, EcUnregCom},
    {EcRegD3d, EcUnregD3d},
    {EcRegGdip, EcUnregGdip},
    {EcRegErrno, EcUnregErrno},
    {EcRegSig, EcUnregSig},
    {EcRegBug, EcUnregBug},
};

static ErrorCodeSet ec_set[ARRAYSIZE(types)];

HMODULE hModCertCli = NULL, hModCertUtil = NULL, hModHttp = NULL, hModNt = NULL, hModKrnl = NULL;

static void ResLoadAll()
{
    LOAD_RES(hModCertCli, TEXT("certca"));
    LOAD_RES(hModCertCli, TEXT("certcli"));
    LOAD_RES(hModCertUtil, TEXT("certutil.exe"));
    LOAD_RES(hModHttp, TEXT("winhttp"));
    hModNt = GetModuleHandle(TEXT("ntdll"));
    LOAD_RES(hModKrnl, TEXT("ntoskrnl.exe"));
}

static void ResUnloadAll()
{
    FREE_RES(hModCertCli);
    FREE_RES(hModCertUtil);
    FREE_RES(hModHttp);
    hModNt = NULL;
    FREE_RES(hModKrnl);
}

DWORD EcInit(ErrorCodeSet **ppecs)
{
    SIZE_T i;

    ResLoadAll();

    for (i = 0; i < ARRAYSIZE(types); ++i)
    {
        types[i].pfnreg(&ec_set[i]);
    }

    *ppecs = ec_set;
    return ARRAYSIZE(types);
}

void EcUninit()
{
    SIZE_T i;

    ResUnloadAll();

    for (i = 0; i < ARRAYSIZE(types); ++i)
    {
        types[i].pfnunreg();
    }
}

LPTSTR MsgFromSys(DWORD dwFlags, LPCVOID lpSource, DWORD dwCode)
{
    LPTSTR result = NULL;

    dwFlags |= FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;
    if (!FormatMessage(dwFlags, lpSource, dwCode, 0, (LPTSTR)&result, 0, NULL))
    {
        result = (LPTSTR)LocalAlloc(LPTR, sizeof(*result));
    }

    return result;
}

