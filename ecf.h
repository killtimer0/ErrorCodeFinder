#pragma once

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#ifdef _DEBUG
#define dbg(fmt, ...) _tprintf(fmt##_T("\n"), __VA_ARGS__)
#define pause()       system("pause")
#define dbgstart()                   \
    do                               \
    {                                \
        AllocConsole();              \
        freopen("CON", "r", stdin);  \
        freopen("CON", "w", stdout); \
        freopen("CON", "w", stderr); \
    } while (0)
#define dbgend() FreeConsole()
#else
#define dbg(fmt, ...)
#define pause()
#define dbgstart()
#define dbgend()
#endif

/* res macro */

#define LOAD_RES(mod, f)                                              \
    do                                                                \
    {                                                                 \
        if (!(mod))                                                   \
        {                                                             \
            (mod) = LoadLibraryEx(f, NULL, LOAD_LIBRARY_AS_DATAFILE); \
        }                                                             \
    } while (0)

#define FREE_RES(mod)           \
    do                          \
    {                           \
        if ((mod))              \
        {                       \
            FreeLibrary((mod)); \
            (mod) = NULL;       \
        }                       \
    } while (0)

/* max size for error description */
#define MAX_MSG_LEN     256

/* alt color alpha blend */
#define ALTALPHABK      (15.0F/127)

#pragma pack(4)
typedef struct _ErrorCodeItem {
    DWORD dwCode;
    LPCTSTR lpName;
} ErrorCodeItem;
#pragma pack()

#ifdef UNICODE
#define REGCODE(c) c, L#c,
#else
#define REGCODE(c) c, #c,
#endif

typedef struct _ErrorCodeSet {
    LPCTSTR lpName;
    /* return NULL or a pointer that can be free by pfnFreeMessage */
    LPTSTR (*pfnMessageFromCode)(
        DWORD dwCode
        );
    /* the free function */
    void (*pfnFreeMessage)(
        LPTSTR lpMessage
        );
    DWORD dwCodeCount;
    ErrorCodeItem *items;
} ErrorCodeSet;

/* highlight related */

#define SFM_CODE		1
#define SFM_NAME		2
#define SFM_SCOPE		4

typedef struct _HighlightRange {
	DWORD dwStart, dwEnd;
} HighlightRange;

typedef struct _HighlightHint {
	DWORD dwHintMask;
	DWORD dwRanges;
	HighlightRange *ranges;
} HighlightHint;

/* item in listview */
typedef struct _ListItem {
	DWORD dwScope;
	DWORD dwItem;
	HighlightHint;
} ListItem;

/* all items */
typedef struct _ListContext {
	DWORD dwCount;
	ListItem *item;
} ListContext;

typedef struct _TArray {
	DWORD dwCount, dwMaxCount, dwCell;
	LPVOID lpData;
} TArray;

#define SF_MAX_NAMES	16

typedef struct _SearchFilter {
	DWORD dwMask;
	DWORD dwCode;
	DWORD dwNameCount;
	DWORD dwScopeCount;
	DWORD dwScopes[SF_MAX_NAMES];
	LPTSTR lpNames[SF_MAX_NAMES];
} SearchFilter;

extern HMODULE hModCertCli, hModCertUtil, hModHttp, hModNt, hModKrnl;

/* main */

#define STATUS_MAXLEN		1024

extern HINSTANCE hInst;
extern ErrorCodeSet *pecs;
extern DWORD necs;
extern ListContext ctxList;
extern BOOL bAlwaysTop, bAltColor;

extern int dlgmain(HINSTANCE hInstance);
extern void SetStatusText(HWND hdlg, LPCTSTR lpText);
extern void SetStatusTextN(HWND hdlg, LPCTSTR lpText, DWORD dwNum);
extern void SetStatusTextTemp(HWND hdlg, LPCTSTR lpText);

/* error handler */

extern LONG WINAPI BugHandler(EXCEPTION_POINTERS *e);

/* provider */

extern DWORD EcInit(ErrorCodeSet **ppecs);
extern void EcUninit();
extern LPTSTR MsgFromSys(DWORD dwFlags, LPCVOID lpSource, DWORD dwCode);

/* utils */

extern BOOL ArrayInit(TArray *ary, DWORD dwCell, DWORD dwSizeHint);
extern void ArrayClear(TArray *ary);
extern LPVOID ArrayFinish(TArray *ary, DWORD *pdwCount);
extern BOOL ArrayAppend(TArray *ary, LPVOID data);
extern BOOL MatchWildCard(
	LPCTSTR lpText,
	LPCTSTR lpPattern,
	DWORD *pdwRanges,
	HighlightRange **ppRange
	);
extern BOOL TcsStartsWith(LPCTSTR p, LPCTSTR q);
extern BOOL FmtParseDword(LPCTSTR fmt, DWORD l, DWORD r, DWORD *dwResult);
extern BOOL FilterFromString(LPCTSTR fmt, SearchFilter *filter);
extern void FilterClear(SearchFilter *filter);
extern COLORREF BlendColor(COLORREF color);

/* dlgproc */

extern INT_PTR CALLBACK DlgProcRead(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcHelp(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcAbout(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/* guiaux */

#define PATALL			TEXT("*.*")
#define PATTXT			TEXT("*.txt")
#define PATRES			TEXT("*.exe;*.dll;*.ocx;*.cpl;*.scr;*.mui;*.msstyles")

extern HMENU LoadPopupMenu(UINT id);
extern LPTSTR GetWndText(HWND hwnd);
extern BOOL CopyText(HWND hwnd, LPCTSTR lpString);
extern void UrlOpen(LPTSTR lpUrl);
extern void SearchInMSDN(LPCTSTR what);
extern void GetWndSize(HWND hwnd, SIZE *size);
extern void GetDlgUnit(HWND hdlg, SIZE *size);
extern void GetMsgPosClient(HWND hwnd, LPPOINT ppt);
extern BOOL FileOpenOrSave(
	HWND hwndParent,
	BOOL bOpen,
	LPCTSTR lpDefExt,
	LPCTSTR lpFilter,
	LPTSTR lpBuffer
	);

/* ldmgr */

extern BOOL LvLoadFilter(HWND hctl, SearchFilter *filter);
extern BOOL LvOnNotify(HWND hdlg, NMHDR *hdr);
extern BOOL ItemLoadAll(HWND hctl);
extern void LvInitFont(HWND hwnd);

