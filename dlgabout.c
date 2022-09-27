#include "ecf.h"
#include "resource.h"
#include <commctrl.h>

INT_PTR CALLBACK DlgProcAbout(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

		case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR*)lParam;

			switch (hdr->idFrom)
			{
				case IDC_ABOUT_MSG:
				{
					switch (hdr->code)
					{
						case NM_CLICK:
						{
							/* Note: we don't handle href mark */
							LITEM *item = &((NMLINK*)hdr)->item;

							if (!wcsicmp(item->szID, L"Home"))
							{
								UrlOpen(TEXT("https://killtimer0.github.io"));
							}
							else if (!wcsicmp(item->szID, L"Feedback"))
							{
								UrlOpen(TEXT("mailto:1837009039@qq.com"));
							}
						}
						break;
					}
				}
				break;
			}
		}
		break;

		case WM_INITDIALOG:
		SetFocus(GetDlgItem(hdlg, IDOK));
		break;
	}

	return FALSE;
}

