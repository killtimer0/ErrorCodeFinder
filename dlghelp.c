#include "ecf.h"
#include "resource.h"

INT_PTR CALLBACK DlgProcHelp(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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
				case IDCANCEL:
				EndDialog(hdlg, id);
				return 0;
			}
		}
		break;

		case WM_INITDIALOG:
		SetFocus(GetDlgItem(hdlg, IDOK));
		break;
	}

	return FALSE;
}

