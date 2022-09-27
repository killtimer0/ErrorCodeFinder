#include "ecf.h"
#include "resource.h"
#include <commctrl.h>

static void ReadDlgOnOk(HWND hdlg)
{
	HMODULE hMod = NULL;
	LPTSTR lpText, lpMsg;
	TCHAR buf[1024], fmt[MAX_STRING_LENGTH];
	int iLen;
	DWORD dwCode;
	LPTSTR lpFailText = NULL;

	iLen = GetWindowText(GetDlgItem(hdlg, IDC_READ_E_ID), buf, 1024);

	if (FmtParseDword(buf, 0, iLen, &dwCode))
	{
		lpText = GetWndText(GetDlgItem(hdlg, IDC_READ_COMBO));

		if (lpText && _tcslen(lpText) > 0)
		{
			BOOL bFailed = TRUE;

			LOAD_RES(hMod, lpText);
			if (hMod)
			{
				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_IGNORE_INSERTS
					| FORMAT_MESSAGE_FROM_HMODULE,
					hMod,
					dwCode,
					0,
					(LPTSTR)&lpMsg,
					0,
					NULL
					);

				if (lpMsg)
				{
					SetDlgItemText(hdlg, IDC_READ_E_MSG, lpMsg);
					bFailed = FALSE;
					LocalFree(lpMsg);
				}
				else
				{
					SetDlgItemText(hdlg, IDC_READ_E_MSG, NULL);
				}

				iLen = LoadString(hMod, dwCode, buf, 1024);

				if (iLen)
				{
					SetDlgItemText(hdlg, IDC_READ_E_STR, buf);
					bFailed = FALSE;
				}
				else
				{
					SetDlgItemText(hdlg, IDC_READ_E_STR, NULL);
				}

				FREE_RES(hMod);

				if (bFailed)
				{
					LoadString(hInst, IDS_ERR_READ_RES, fmt, MAX_STRING_LENGTH);
					_sntprintf(buf, 1024, fmt, lpText, dwCode);
					lpFailText = buf;
				}
			}
			else
			{
				LPTSTR lpReason = TEXT("");
				LoadString(hInst, IDS_ERR_READ_MOD, fmt, MAX_STRING_LENGTH);

				FormatMessage(
					FORMAT_MESSAGE_ALLOCATE_BUFFER
					| FORMAT_MESSAGE_IGNORE_INSERTS
					| FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					0,
					(LPTSTR)&lpMsg,
					0,
					NULL
					);

				if (lpMsg)
					lpReason = lpMsg;

				_sntprintf(buf, 1024, fmt, lpText, lpReason);
				lpFailText = buf;
			}

			LocalFree(lpText);
		}
		else
		{
			LoadString(hInst, IDS_ERR_READ_NAME, fmt, MAX_STRING_LENGTH);
			lpFailText = fmt;
		}
	}
	else
	{
		LoadString(hInst, IDS_ERR_READ_FMT, fmt, MAX_STRING_LENGTH);
		lpFailText = fmt;
	}

	if (lpFailText)
	{
		MessageBox(hdlg, lpFailText, NULL, MB_OK | MB_ICONERROR);
	}

}

#define INITCOMBOEXT		TEXT("\\*.dll")

INT_PTR CALLBACK DlgProcRead(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_COMMAND:
		{
			UINT id = LOWORD(wParam);
			UINT code = HIWORD(wParam);

			switch (id)
			{
				case IDOK:
				ReadDlgOnOk(hdlg);
				break;

				case IDCANCEL:
				EndDialog(hdlg, id);
				return 0;

				case IDC_READ_BROWSE:
				{
					TCHAR szPath[MAX_PATH];
					TCHAR filter[
						MAX_STRING_LENGTH * 2
						+ ARRAYSIZE(PATRES)
						+ ARRAYSIZE(PATALL) + 1];
					DWORD dwPos = 0;

					dwPos += LoadString(
						hInst,
						IDS_FILTER_RES,
						filter + dwPos,
						MAX_STRING_LENGTH
						);
					++dwPos;
					memmove(filter + dwPos, PATRES, sizeof (PATRES));
					dwPos += ARRAYSIZE(PATRES);

					dwPos += LoadString(
						hInst,
						IDS_FILTER_ALL,
						filter + dwPos,
						MAX_STRING_LENGTH
						);
					++dwPos;
					memmove(filter + dwPos, PATALL, sizeof (PATALL));
					dwPos += ARRAYSIZE(PATALL);

					filter[dwPos++] = TEXT('\0');

					if (FileOpenOrSave(hdlg, TRUE, TEXT("dll"), filter, szPath))
					{
						SetDlgItemText(hdlg, IDC_READ_COMBO, szPath);
					}
				}
				break;
			}
		}
		break;

		case WM_INITDIALOG:
		{
			TCHAR szSystemPath[MAX_PATH + ARRAYSIZE(INITCOMBOEXT)];
			WCHAR szHint[MAX_STRING_LENGTH];
			DWORD dwPos = 0;
			HWND hctl;

			dwPos += GetSystemDirectory(szSystemPath, MAX_PATH);
			memmove(szSystemPath + dwPos, INITCOMBOEXT, sizeof (INITCOMBOEXT));
			dwPos += ARRAYSIZE(INITCOMBOEXT);

			DlgDirListComboBox(
				hdlg,
				szSystemPath,
				IDC_READ_COMBO,
				0,
				DDL_SYSTEM | DDL_READONLY
				);

			hctl = GetDlgItem(hdlg, IDC_READ_COMBO);
			LoadStringW(hInst, IDS_HINT_PATH, szHint, MAX_STRING_LENGTH);
			SendMessage(hctl, CB_SETCUEBANNER, 0, (LPARAM)szHint);
			SetFocus(hctl);

			hctl = GetDlgItem(hdlg, IDC_READ_E_ID);
			LoadStringW(hInst, IDS_HINT_ID, szHint, MAX_STRING_LENGTH);
			SendMessage(hctl, EM_SETCUEBANNER, TRUE, (LPARAM)szHint);
		}
		break;
	}

	return FALSE;
}

