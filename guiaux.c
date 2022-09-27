#include "ecf.h"
#include "resource.h"
#include <windowsx.h>

HMENU LoadPopupMenu(UINT id)
{
	HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(id));
	HMENU hPopup = NULL;

	if (hMenu)
	{
		hPopup = GetSubMenu(hMenu, 0);
		RemoveMenu(hMenu, 0, MF_BYPOSITION);
		DestroyMenu(hMenu);
	}

	return hPopup;
}

LPTSTR GetWndText(HWND hwnd)
{
	int iLen;
	LPTSTR lpText;

	iLen = GetWindowTextLength(hwnd) + 1;
	lpText = LocalAlloc(LPTR, iLen * sizeof (TCHAR));

	if (lpText)
	{
		GetWindowText(hwnd, lpText, iLen);
	}

	return lpText;
}

BOOL CopyText(HWND hwnd, LPCTSTR lpString)
{
	BOOL result = FALSE;
	SIZE_T dwLen = _tcslen(lpString);
	HGLOBAL hMem;

	if (dwLen)
	{
		if (OpenClipboard(hwnd))
		{
			EmptyClipboard();

			hMem = GlobalAlloc(GMEM_MOVEABLE, (dwLen + 1) * sizeof (*lpString));
			if (hMem)
			{
				LPTSTR lpText = GlobalLock(hMem);
				if (lpText)
				{
					memmove(lpText, lpString, (dwLen + 1) * sizeof (*lpString));
					GlobalUnlock(hMem);

					result = NULL != SetClipboardData(
#ifdef UNICODE
						CF_UNICODETEXT,
#else
						CF_TEXT,
#endif
						hMem
						);
				}
				/* Note: NO NOT free hMem */
			}

			CloseClipboard();
		}
	}

	return result;
}

void UrlOpen(LPTSTR lpUrl)
{
	ShellExecute(
		NULL,
		TEXT("open"),
		lpUrl,
		NULL,
		NULL,
		SW_SHOWDEFAULT
		);
}

void SearchInMSDN(LPCTSTR what)
{
	TCHAR fmt[256];
	LPTSTR lpCommand;
	DWORD dwLen = 256 + _tcslen(what);

	LoadString(hInst, IDS_MSDN_URL_FMT, fmt, 256);
	lpCommand = LocalAlloc(LPTR, dwLen * sizeof (TCHAR));

	if (lpCommand)
	{
		_sntprintf(lpCommand, dwLen, fmt, what);
		UrlOpen(lpCommand);

		LocalFree(lpCommand);
	}
}

void GetWndSize(HWND hwnd, SIZE *size)
{
	RECT r;
	GetWindowRect(hwnd, &r);
	size->cx = r.right - r.left;
	size->cy = r.bottom - r.top;
}

void GetDlgUnit(HWND hdlg, SIZE *size)
{
	RECT r;
	r.left = r.top = 0, r.right = 4, r.bottom = 8;
	MapDialogRect(hdlg, &r);
	size->cx = r.right;
	size->cy = r.bottom;
}

void GetMsgPosClient(HWND hwnd, LPPOINT ppt)
{
	LPARAM lParam;
	lParam = GetMessagePos();
	ppt->x = GET_X_LPARAM(lParam);
	ppt->y = GET_Y_LPARAM(lParam);
	ScreenToClient(hwnd, ppt);
}

BOOL FileOpenOrSave(
	HWND hwndParent,
	BOOL bOpen,
	LPCTSTR lpDefExt,
	LPCTSTR lpFilter,
	LPTSTR lpBuffer
	)
{
	OPENFILENAME ofn = {sizeof (ofn)};

	ofn.hwndOwner = hwndParent;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = lpFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	lpBuffer[0] = TEXT('\0');
	ofn.lpstrFile = lpBuffer;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY;
	ofn.Flags |= bOpen ? OFN_FILEMUSTEXIST : OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = lpDefExt;

	return (bOpen ? GetOpenFileName : GetSaveFileName)(&ofn);
}

