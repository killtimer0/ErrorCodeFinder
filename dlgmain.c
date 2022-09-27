#include "ecf.h"
#include "resource.h"
#include <commctrl.h>
#include <uxtheme.h>

HINSTANCE hInst;
ErrorCodeSet *pecs;
DWORD necs;

static HICON hIconApp = NULL;
static HMENU hMainMenu = NULL;

/* menu items */
BOOL bAlwaysTop = FALSE, bAltColor = TRUE;

static void LayoutAdjustMain(HWND hdlg)
{
	HWND hctl;
	RECT rcClient;
	SIZE size, unit;
	HDWP hdwp;
	LONG lBorderWidth, lBorderHeight;
	LONG lEditSpace;

	GetClientRect(hdlg, &rcClient);
	GetDlgUnit(hdlg, &unit);
	lBorderWidth = MulDiv(DLG_BORDER, unit.cx, 4);
	lBorderHeight = MulDiv(DLG_BORDER, unit.cy, 8);

	hctl = GetDlgItem(hdlg, IDC_MAIN_S_BAR);
	SendMessage(hctl, WM_SIZE, 0, 0);
	GetWndSize(hctl, &size);

	hdwp = BeginDeferWindowPos(2);

	hctl = GetDlgItem(hdlg, IDC_MAIN_E_SEARCH);
	DeferWindowPos(
		hdwp,
		hctl,
		NULL,
		lBorderWidth,
		lBorderHeight,
		rcClient.right - 2 * lBorderWidth,
		MulDiv(14, unit.cy, 8),
		SWP_NOACTIVATE | SWP_NOZORDER
		);

	hctl = GetDlgItem(hdlg, IDC_MAIN_L_DATA);
	lEditSpace = MulDiv(14 + 2 * DLG_BORDER, unit.cy, 8);
	DeferWindowPos(
		hdwp,
		hctl,
		NULL,
		0,
		lBorderHeight + lEditSpace,
		rcClient.right,
		rcClient.bottom - lBorderHeight - lEditSpace - size.cy,
		SWP_NOACTIVATE | SWP_NOZORDER
		);

	EndDeferWindowPos(hdwp);
}

#define IDT_STATUS			1
#define TEMPINFO_TIMEOUT	1000
static TCHAR szStatus[STATUS_MAXLEN];

void SetStatusText(HWND hdlg, LPCTSTR lpText)
{
	TCHAR buf[MAX_STRING_LENGTH];
	HWND hctl = GetDlgItem(hdlg, IDC_MAIN_S_BAR);

	KillTimer(hdlg, IDT_STATUS);

	if ((SIZE_T)lpText < 0x10000)
	{
		LoadString(hInst, LOWORD(lpText), buf, MAX_STRING_LENGTH);
		lpText = buf;
	}

	SendMessage(hctl, WM_SETTEXT, 0, (LPARAM)lpText);
}

void SetStatusTextN(HWND hdlg, LPCTSTR lpText, DWORD dwNum)
{
	TCHAR buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
	HWND hctl = GetDlgItem(hdlg, IDC_MAIN_S_BAR);

	if ((SIZE_T)lpText < 0x10000)
	{
		LoadString(hInst, LOWORD(lpText), buf, MAX_STRING_LENGTH);
		lpText = buf;
	}

	_sntprintf(buf2, MAX_STRING_LENGTH, lpText, dwNum);
	SetStatusText(hdlg, buf2);
}

void SetStatusTextTemp(HWND hdlg, LPCTSTR lpText)
{
	TCHAR buf[MAX_STRING_LENGTH];
	HWND hctl = GetDlgItem(hdlg, IDC_MAIN_S_BAR);

	if (!KillTimer(hdlg, IDT_STATUS))
		SendMessage(hctl, WM_GETTEXT, STATUS_MAXLEN, (LPARAM)szStatus);
	if ((SIZE_T)lpText < 0x10000)
	{
		LoadString(hInst, LOWORD(lpText), buf, MAX_STRING_LENGTH);
		lpText = buf;
	}
	SendMessage(hctl, WM_SETTEXT, 0, (LPARAM)lpText);

	SetTimer(hdlg, IDT_STATUS, TEMPINFO_TIMEOUT, NULL);
}

static BOOL InitListView(HWND hctl)
{
	TCHAR buf[MAX_STRING_LENGTH];
	LVCOLUMN lvc;
	UINT idColName[] = {
		IDS_MAIN_COLUMN_NAME,
		IDS_MAIN_COLUMN_VAL,
		IDS_MAIN_COLUMN_SCOPE,
		IDS_MAIN_COLUMN_MSG,
	};
	int iColFmt[] = {
		LVCFMT_LEFT,
		LVCFMT_RIGHT,
		LVCFMT_LEFT,
		LVCFMT_LEFT,
	};
	int iColWidth[] = {
		120, 60, 50, MAIN_NORMAL_X - 230
	};
	int iCol;
	SIZE size;
	DWORD dwStyle;

	SetWindowTheme(hctl, L"Explorer", NULL);

	dwStyle = ListView_GetExtendedListViewStyle(hctl);
	dwStyle |= LVS_EX_FULLROWSELECT
			| LVS_EX_HEADERDRAGDROP
			| LVS_EX_INFOTIP
			| LVS_EX_DOUBLEBUFFER;

	ListView_SetExtendedListViewStyle(hctl, dwStyle);

	GetDlgUnit(GetParent(hctl), &size);
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	for (iCol = 0; iCol < ARRAYSIZE(idColName); ++iCol)
	{
		LoadString(hInst, idColName[iCol], buf, MAX_STRING_LENGTH);

		lvc.fmt = iColFmt[iCol];

		lvc.cx = MulDiv(iColWidth[iCol], size.cx, 4);
		if (iCol == ARRAYSIZE(idColName) - 1)
			lvc.cx -= GetSystemMetrics(SM_CXVSCROLL);

		lvc.pszText = buf;
		lvc.iSubItem = iCol;
		if (-1 == ListView_InsertColumn(hctl, iCol, &lvc))
			return FALSE;
	}

	return TRUE;
}

static HMENU hMenuView;

static void DlgCheckItem(UINT id, BOOL bChecked)
{
	UINT uFlags = MF_BYCOMMAND;

	if (bChecked)
		uFlags |= MF_CHECKED;
	else
		uFlags |= MF_UNCHECKED;

	CheckMenuItem(hMenuView, id, uFlags);
}

static void SetAlwaysTop(HWND hdlg, BOOL bTop)
{
	DWORD dwExStyle;
	DWORD dwFlags;
	HWND hwndIns, hwndTT;

	dwFlags = SWP_NOMOVE | SWP_NOSIZE;
	hwndIns = bTop ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(hdlg, hwndIns, 0, 0, 0, 0, dwFlags);

	hwndTT = ListView_GetToolTips(GetDlgItem(hdlg, IDC_MAIN_L_DATA));
	dwFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE;
	hwndIns = bTop ? HWND_TOPMOST : HWND_NOTOPMOST;
	SetWindowPos(hwndTT, hwndIns, 0, 0, 0, 0, dwFlags);

	dwExStyle = (DWORD)GetWindowLongPtr(hdlg, GWL_EXSTYLE);
	bAlwaysTop = dwExStyle & WS_EX_TOPMOST;
	DlgCheckItem(ID_MAIN_VIEW_TOP, bAlwaysTop);

}

static void SetAltColor(HWND hctl, BOOL bColor)
{
	bAltColor = bColor;
	RedrawWindow(hctl, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	DlgCheckItem(ID_MAIN_VIEW_ALTCOLOR, bAltColor);
}

static BOOL ExportIteratorCurrent(DWORD *cur, DWORD *scope, DWORD *idx)
{
	if (cur[0] >= ctxList.dwCount)
		return FALSE;

	*scope = ctxList.item[*cur].dwScope;
	*idx = ctxList.item[*cur].dwItem;
	++*cur;

	return TRUE;
}

static BOOL ExportIteratorAll(DWORD *cur, DWORD *scope, DWORD *idx)
{
	while (cur[0] < necs && cur[1] >= pecs[cur[0]].dwCodeCount)
	{
		cur[1] = 0;
		++cur[0];
	}

	if (cur[0] >= necs)
		return FALSE;

	*scope = cur[0];
	*idx = cur[1];

	++cur[1];

	return TRUE;
}

static BOOL ExportItems(HWND hdlg, UINT id)
{
	DWORD dwPos = 0;
	TCHAR filter[MAX_STRING_LENGTH * 2 + ARRAYSIZE(PATTXT) + ARRAYSIZE(PATALL) + 1];
	TCHAR szPath[MAX_PATH];
	BOOL (*pfnExport)(DWORD *, DWORD *, DWORD *) = NULL;
	DWORD cur[2];
	BOOL bSucceeded = FALSE;

	dwPos += LoadString(hInst, IDS_FILTER_TEXT, filter + dwPos, MAX_STRING_LENGTH);
	++dwPos;
	memmove(filter + dwPos, PATTXT, sizeof (PATTXT));
	dwPos += ARRAYSIZE(PATTXT);

	dwPos += LoadString(hInst, IDS_FILTER_ALL, filter + dwPos, MAX_STRING_LENGTH);
	++dwPos;
	memmove(filter + dwPos, PATALL, sizeof (PATALL));
	dwPos += ARRAYSIZE(PATALL);

	filter[dwPos++] = TEXT('\0');

	switch (id)
	{
		case ID_MAIN_FILE_EXPCUR:
		cur[0] = 0;
		pfnExport = ExportIteratorCurrent;
		break;

		case ID_MAIN_FILE_EXPALL:
		cur[0] = cur[1] = 0;
		pfnExport = ExportIteratorAll;
		break;

	}

	if (pfnExport)
	{
		if (FileOpenOrSave(hdlg, FALSE, TEXT("txt"), filter, szPath))
		{
			FILE *f = _tfopen(szPath, TEXT("w"));
			if (f)
			{
				DWORD scope, idx;
				while (pfnExport(cur, &scope, &idx))
				{
					LPTSTR lpDescExp = TEXT("");
					LPTSTR lpDesc = pecs[scope].pfnMessageFromCode(
						pecs[scope].items[idx].dwCode
						);

					if (lpDesc)
						lpDescExp = lpDesc;

					_ftprintf(
						f,
						TEXT("%s 0x%X %s %s\n"),
						pecs[scope].items[idx].lpName,
						pecs[scope].items[idx].dwCode,
						pecs[scope].lpName,
						lpDescExp
						);

					if (lpDesc)
						pecs[scope].pfnFreeMessage(lpDesc);
				}

				fclose(f);
				bSucceeded = TRUE;
			}
		}
	}

	return bSucceeded;
}

static void EditOnChange(HWND hdlg, HWND hctl)
{
	LPTSTR lpText;
	SearchFilter filter;

	lpText = GetWndText(hctl);
	if (lpText)
	{
		if (FilterFromString(lpText, &filter))
		{
			LPTSTR lpMsg = MAKEINTRESOURCE(filter.dwMask ? IDS_S_SEARCHDONE : IDS_S_DONE);

			LvLoadFilter(GetDlgItem(hdlg, IDC_MAIN_L_DATA), &filter);
			SetStatusTextN(hdlg, lpMsg, ctxList.dwCount);

			FilterClear(&filter);
		}
		LocalFree(lpText);
	}
}

static void DlgMainOnCmd(HWND hdlg, UINT id, UINT code, HWND hctl)
{
	switch (id)
	{
		case IDCANCEL:
		EndDialog(hdlg, id);
		break;

		case IDC_MAIN_E_SEARCH:
		switch (code)
		{
			case EN_CHANGE:
			EditOnChange(hdlg, hctl);
			break;
		}
		break;

		case ID_MAIN_FILE_READ:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_READ), hdlg, DlgProcRead);
		break;

		case ID_MAIN_FILE_EXPCUR:
		case ID_MAIN_FILE_EXPALL:
		if (ExportItems(hdlg, id))
			SetStatusTextTemp(hdlg, MAKEINTRESOURCE(IDS_S_EXPORTED));
		break;

		case ID_MAIN_VIEW_TOP:
		SetAlwaysTop(hdlg, !bAlwaysTop);
		break;

		case ID_MAIN_VIEW_ALTCOLOR:
		SetAltColor(GetDlgItem(hdlg, IDC_MAIN_L_DATA), !bAltColor);
		break;

		case ID_MAIN_HELP_HELP:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_HELP), hdlg, DlgProcHelp);
		break;

		case ID_MAIN_HELP_ABOUT:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hdlg, DlgProcAbout);
		break;
	}
}

WNDPROC pfnLVProc = NULL;
extern LRESULT CALLBACK LvxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

static INT_PTR CALLBACK DlgProcMain(
	HWND hdlg,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
	)
{
	switch (uMsg)
	{
		case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR*)lParam;
			switch(hdr->idFrom)
			{
				case IDC_MAIN_L_DATA:
				return LvOnNotify(hdlg, hdr);
			}
		}
		break;

		case WM_TIMER:
		{
			switch (wParam)
			{
				case IDT_STATUS:
				{
					HWND hctl = GetDlgItem(hdlg, IDC_MAIN_S_BAR);
					SendMessage(hctl, WM_SETTEXT, 0, (LPARAM)szStatus);
					KillTimer(hdlg, IDT_STATUS);
				}
				break;
			}
		}
		break;

		case WM_SIZE:
		LayoutAdjustMain(hdlg);
		break;

		case WM_GETMINMAXINFO:
		{
			MINMAXINFO *mmi = (MINMAXINFO*)lParam;
			RECT rcMinSize;

			rcMinSize.left = rcMinSize.top = 0;
			rcMinSize.right = MAIN_MIN_X;
			rcMinSize.bottom = MAIN_MIN_Y;
			MapDialogRect(hdlg, &rcMinSize);
			AdjustWindowRectEx(
				&rcMinSize,
				(DWORD)GetWindowLongPtr(hdlg, GWL_STYLE),
				GetMenu(hdlg) != NULL,
				(DWORD)GetWindowLongPtr(hdlg, GWL_EXSTYLE)
				);

			mmi->ptMinTrackSize.x = rcMinSize.right - rcMinSize.left;
			mmi->ptMinTrackSize.y = rcMinSize.bottom - rcMinSize.top;
		}
		break;

		case WM_COMMAND:
		{
			UINT id = LOWORD(wParam);
			UINT code = HIWORD(wParam);
			HWND hctl = (HWND)lParam;

			DlgMainOnCmd(hdlg, id, code, hctl);
			return 0;
		}

		case WM_INITDIALOG:
		{
			WCHAR buf[MAX_STRING_LENGTH];
			HWND hctl;

			SendMessage(hdlg, WM_SETICON, ICON_BIG, (LPARAM)hIconApp);
			SetMenu(hdlg, hMainMenu);

			hctl = GetDlgItem(hdlg, IDC_MAIN_E_SEARCH);
			LoadStringW(hInst, IDS_HINT_SEARCH, buf, MAX_STRING_LENGTH);
			SendMessage(hctl, EM_SETCUEBANNER, TRUE, (LPARAM)buf);
			SetFocus(hctl);

			SetStatusText(hdlg, MAKEINTRESOURCE(IDS_S_PREPARING));

			hctl = GetDlgItem(hdlg, IDC_MAIN_L_DATA);

			/* we must modify this control, as we have our own way
			 * to calculate header width, etc. */
			pfnLVProc = (WNDPROC)GetWindowLongPtr(hctl, GWLP_WNDPROC);
			SetWindowLongPtr(hctl, GWLP_WNDPROC, (LONG_PTR)LvxProc);
			LvInitFont(hctl);

			if (!InitListView(hctl))
			{
				SetWindowLongPtr(hctl, GWLP_WNDPROC, (LONG_PTR)pfnLVProc);
				return FALSE;
			}

			ItemLoadAll(hctl);
			SetStatusTextN(hdlg, MAKEINTRESOURCE(IDS_S_DONE), ctxList.dwCount);

			hMenuView = GetSubMenu(GetMenu(hdlg), 1);
			SetAlwaysTop(hdlg, bAlwaysTop);
			DlgCheckItem(ID_MAIN_VIEW_ALTCOLOR, bAltColor);

		}
		break;

		default:
		return FALSE;

	}
	return TRUE;
}

static BOOL LoadClasses()
{
	BOOL result = FALSE;
	/* WNDCLASSEX wc; */

	InitCommonControls();

	/*
	if (GetClassInfoEx(hInst, TEXT("SysListView32"), &wc))
	{
		pfnLVProc = wc.lpfnWndProc;
		wc.cbSize = sizeof (wc);
		wc.lpfnWndProc = listviewx_proc;
		wc.lpszClassName = TEXT(CLS_LISTVIEWX);

		if (RegisterClassEx(&wc))
			result = TRUE;
	}
	*/
	result = TRUE;

	return result;
}

static void UnloadClasses()
{
	/* UnregisterClass(TEXT(CLS_LISTVIEWX), hInst); */
}

int dlgmain(HINSTANCE hInstance){
	hInst = hInstance;
	SetUnhandledExceptionFilter(BugHandler);

	dbgstart();

	necs = EcInit(&pecs);

	hIconApp = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONAPP));

	if (LoadClasses(hInst))
	{
		hMainMenu = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINMENU));
		DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, DlgProcMain);

		DestroyMenu(hMainMenu);
		UnloadClasses(hInst);
	}

	EcUninit();

	dbgend();

	return 0;
}
