#include "ecf.h"
#include "resource.h"
#include <commctrl.h>

typedef struct _CurrentSortColumn {
	SHORT idx;
	SHORT sDirection;
} CurrentSortColumn;

ListContext ctxList = {0, NULL};

#define LVITEM_MAXLEN	1024
static HFONT hFontNormal = NULL, hFontBold = NULL;
static CurrentSortColumn ctxSort = {0xFF, 0};

static void ItemFree()
{
	if (ctxList.item)
	{
		DWORD i, dwCount = ctxList.dwCount;
		ListItem *item = ctxList.item;

		for (i = 0; i < dwCount; ++i, ++item)
		{
			if ((item->dwHintMask & SFM_NAME) && item->ranges)
				LocalFree(item->ranges);
		}

		LocalFree(ctxList.item);
		ctxList.item = NULL;
		ctxList.dwCount = 0;
	}
}

static void LvSetCount(HWND hctl, DWORD dwCount)
{
	SendMessage(hctl, WM_SETREDRAW, FALSE, 0);
	ListView_DeleteAllItems(hctl);
	ListView_SetItemCountEx(hctl, dwCount, LVSICF_NOINVALIDATEALL | LVSICF_NOSCROLL);
	SendMessage(hctl, WM_SETREDRAW, TRUE, 0);
}

static void LvSetSel(HWND hctl, int idx, BOOL bVisible)
{
	ListView_SetItemState(hctl, idx, LVIS_SELECTED, LVIS_SELECTED);
	ListView_SetSelectionMark(hctl, idx);
	if (bVisible)
	{
		ListView_EnsureVisible(hctl, idx, FALSE);
	}
}

static int __cdecl ItemSort_Name(const ListItem *p1, const ListItem *p2)
{
	int result;

	result = _tcsicmp(
		pecs[p1->dwScope].items[p1->dwItem].lpName,
		pecs[p2->dwScope].items[p2->dwItem].lpName
		);

	if (0 == result)
	{
		dbg(_T("Same string: %s in %s and %s"),
			pecs[p1->dwScope].items[p1->dwItem].lpName,
			pecs[p1->dwScope].lpName,
			pecs[p2->dwScope].lpName
			);
	}

	return result;
}

static int __cdecl ItemSort_Value(const ListItem *p1, const ListItem *p2)
{
	int result;
	DWORD c1, c2;

	c1 = pecs[p1->dwScope].items[p1->dwItem].dwCode;
	c2 = pecs[p2->dwScope].items[p2->dwItem].dwCode;

	if (c1 > c2)
	{
		result = 1;
	}
	else if (c1 < c2)
	{
		result = -1;
	}
	else
	{
		result = ItemSort_Name(p1, p2);
	}

	return result;
}

static int __cdecl ItemSort_Scope(const ListItem *p1, const ListItem *p2)
{
	int result;

	result = _tcsicmp(
		pecs[p1->dwScope].lpName,
		pecs[p2->dwScope].lpName
		);

	if (0 == result)
		result = ItemSort_Name(p1, p2);

	return result;
}

BOOL ItemLoadAll(HWND hctl)
{
	DWORD i, c = 0;
	ListItem *items;

	for (i = 0; i < necs; ++i)
	{
		c += pecs[i].dwCodeCount;
	}

	items = (ListItem*)LocalAlloc(LPTR, c * sizeof (ListItem));

	if (items)
	{
		DWORD j, idx = 0;
		for (i = 0; i < necs; ++i)
		{
			DWORD cc = pecs[i].dwCodeCount;
			for (j = 0; j < cc; ++j)
			{
				items[idx].dwScope = i;
				items[idx].dwItem = j;
				/* Other members are automatically set to zero due to LPTR */
				++idx;
			}
		}

		ItemFree();
		ctxList.dwCount = c;
		ctxList.item = items;

		LvSetCount(hctl, c);

		return TRUE;
	}

	return FALSE;
}

static void ItemInvert()
{
	DWORD i, j, dwCount, dwHalf;
	ListItem buf;

	dwCount = ctxList.dwCount;
	dwHalf = dwCount >> 1;

	for (i = 0, j = dwCount - 1; i < dwHalf; ++i, --j)
	{
		buf = ctxList.item[i];
		ctxList.item[i] = ctxList.item[j];
		ctxList.item[j] = buf;
	}
}

static void ItemSort(int (__cdecl *cmp)(const ListItem *,const ListItem *))
{
	qsort(
		ctxList.item,
		ctxList.dwCount,
		sizeof (*ctxList.item),
		(int (__cdecl *)(const void *, const void *))cmp
		);
}

static BOOL LvSort(int iCol, BOOL bInvert)
{
	switch (iCol)
	{
		/* No sort */
		case -1:
		break;

		/* Sort by name */
		case 0:
		ItemSort(ItemSort_Name);
		break;

		/* Sort by value */
		case 1:
		ItemSort(ItemSort_Value);
		break;

		/* Sort by scope */
		case 2:
		ItemSort(ItemSort_Scope);
		break;

		/* We cannot sort by description because it is lazy-load */
		default:
		return FALSE;
	}

	if (bInvert)
		ItemInvert();

	return TRUE;
}

BOOL LvLoadFilter(HWND hctl, SearchFilter *filter)
{
	BOOL result = FALSE;
	DWORD i, j, k, iScope = 0;
	ListItem item;
	DWORD dwCount;
	ErrorCodeItem *p;
	TArray ary;
	DWORD dwRanges;
	HighlightRange *ranges;

	if (ArrayInit(&ary, sizeof (ListItem), 16))
	{
		i = 0, iScope = 0;
		while (1)
		{
			/* breaker: global or specific scopes */
			if (filter->dwMask & SFM_SCOPE)
			{
				if (i >= filter->dwScopeCount)
					break;
				iScope = filter->dwScopes[i];
			}
			else
			{
				if (iScope >= necs)
					break;
			}

			/* For each scope, we collect filtered items. */
			dwCount = pecs[iScope].dwCodeCount;
			p = pecs[iScope].items;
			for (j = 0; j < dwCount; ++j)
			{
				BOOL bMatch = TRUE;
				memset(&item, 0, sizeof (item));

				/* code filter */
				if ((filter->dwMask & SFM_CODE)
					&& p[j].dwCode != filter->dwCode
					)
					bMatch = FALSE;

				/* name filter */
				if (bMatch && (filter->dwMask & SFM_NAME))
				{
					BOOL bFound = FALSE;

					for (k = 0; k < filter->dwNameCount; ++k)
					{
						if (MatchWildCard(
							p[j].lpName,
							filter->lpNames[k],
							&dwRanges,
							&ranges
							))
						{
							bFound = TRUE;
							break;
						}
					}

					if (!bFound)
						bMatch = FALSE;
				}

				if (bMatch)
				{
					item.dwScope = iScope;
					item.dwItem = j;

					if (filter->dwMask & SFM_CODE)
						item.dwHintMask |= SFM_CODE;
					if (filter->dwMask & SFM_SCOPE)
						item.dwHintMask |= SFM_SCOPE;
					if ((filter->dwMask & SFM_NAME) && dwRanges)
					{
						item.dwHintMask |= SFM_NAME;
						item.dwRanges = dwRanges;
						item.ranges = ranges;
					}

					ArrayAppend(&ary, &item);
				}
			}

			/* iterator: global or specific scopes */
			if (filter->dwMask & SFM_SCOPE)
				++i;
			else
				++iScope;
		}

		ItemFree();
		ctxList.item = ArrayFinish(&ary, &ctxList.dwCount);
		LvSort(ctxSort.idx, ctxSort.sDirection < 0);
		LvSetCount(hctl, ctxList.dwCount);

		result = TRUE;
	}

	return result;
}

#define ELLIPSE_SUFFIX		TEXT("...")

/* We don't care about RTL text, as all charactors are in [A-Za-z0-9_]. */
static void PaintPartlyHighlight(
	HDC hdc,
	RECT *prc,
	LPCTSTR lpText,
	HighlightRange *ranges,
	DWORD dwRangeCount
	)
{
	int iOldMode;
	SIZE size;
	int iSuffixWidth;
	HFONT hFontOld;
	UINT fmt = DT_SINGLELINE | DT_VCENTER | DT_NOPREFIX;
	DWORD dwLen, now, next, idx, i;
	BOOL bBold;
	int *extent;
	int *part;
	int iEllipsePos = -1;
	int ox = 0, iTotalWidth;
	BOOL bToAddEllipse;
	RECT rcFinal;

	iTotalWidth = prc->right - prc->left;
	dwLen = _tcslen(lpText);
	if (!dwLen)
		return;

	/* worst case: extent = dwLen, part(include '...') = 2 * slice + 2 */
	extent = (int*)LocalAlloc(
		LMEM_FIXED,
		(dwLen + 2 * dwRangeCount + 2) * sizeof (int)
		);
	if (!extent)
		return;
	part = extent + dwLen;

	hFontOld = (HFONT)SelectObject(hdc, hFontNormal);
	GetTextExtentExPoint(
		hdc,
		ELLIPSE_SUFFIX,
		ARRAYSIZE(ELLIPSE_SUFFIX) - 1,
		0,
		NULL,
		NULL,
		&size
		);
	iSuffixWidth = size.cx;

	iOldMode = SetBkMode(hdc, TRANSPARENT);

	/* first loop we collect extent data */
	now = 0, idx = 0;
	while (now < dwLen)
	{
		if (idx < dwRangeCount)
		{
			if (now == ranges[idx].dwStart)
				next = ranges[idx++].dwEnd, bBold = TRUE;
			else
				next = ranges[idx].dwStart, bBold = FALSE;
		}
		else
			next = dwLen, bBold = FALSE;

		SelectObject(hdc, bBold ? hFontBold : hFontNormal);

		GetTextExtentExPoint(
			hdc,
			lpText + now,
			next - now,
			0,
			NULL,
			extent + now,
			&size
			);

		for (i = now; i < next; ++i)
			extent[i] += ox;
		ox += size.cx;

		/* draw part of the text and add a suffix */
		if (ox > iTotalWidth)
		{
			for (i = next - 1; i >= 0; --i)
			{
				if (extent[i] + iSuffixWidth <= iTotalWidth)
				{
					iEllipsePos = i + 1;
					break;
				}
			}
			/* We keep at least one charactor that can be seen. */
			if (iEllipsePos < 1)
				iEllipsePos = 1;
			break;
		}

		now = next;
	}

	bToAddEllipse = FALSE;
	rcFinal.right = prc->left;
	rcFinal.top = prc->top;
	rcFinal.bottom = prc->bottom;

	/* then we can draw these slices */
	now = 0, idx = 0;
	while (now < dwLen)
	{
		if (idx < dwRangeCount)
		{
			if (now == ranges[idx].dwStart)
				next = ranges[idx++].dwEnd, bBold = TRUE;
			else
				next = ranges[idx].dwStart, bBold = FALSE;
		}
		else
			next = dwLen, bBold = FALSE;

		SelectObject(hdc, bBold ? hFontBold : hFontNormal);

		if (iEllipsePos >= now && iEllipsePos < next)
		{
			next = iEllipsePos;
			bToAddEllipse = TRUE;
		}

		if (now < next)
		{
			rcFinal.left = prc->left + (now == 0 ? 0 : extent[now - 1]);
			rcFinal.right = prc->left + extent[next - 1];
			DrawText(hdc, lpText + now, next - now, &rcFinal, fmt);
		}

		if (bToAddEllipse)
		{
			SelectObject(hdc, hFontNormal);
			rcFinal.left = rcFinal.right;
			rcFinal.right = rcFinal.left + iSuffixWidth;
			DrawText(
				hdc,
				ELLIPSE_SUFFIX,
				ARRAYSIZE(ELLIPSE_SUFFIX) - 1,
				&rcFinal,
				fmt
				);
			break;
		}

		now = next;
	}

	SetBkMode(hdc, iOldMode);
	SelectObject(hdc, hFontOld);

	LocalFree(extent);
}

static void PaintNormal(HDC hdc, LPCTSTR lpText, int cch, LPRECT prc, UINT fmt)
{
	DrawText(hdc, lpText, cch, prc, fmt);
}

static void paint_item(
	HWND hwnd,
	HDC hdc,
	RECT *prc,
	int iItem,
	int iSubItem
	)
{
	ListItem *item = ctxList.item + iItem;
	TCHAR buf[LVITEM_MAXLEN];
	DWORD fmt = DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX;

	/* we must ensure the text is in pszText instead of one new pointer */
	{
		LVITEM it;

		it.mask = LVIF_TEXT;
		it.iItem = iItem;
		it.iSubItem = iSubItem;
		it.pszText = buf;
		it.cchTextMax = ARRAYSIZE(buf);

		ListView_GetItem(hwnd, &it);
	}

	/* get align info */
	{
		LVCOLUMN lvc;

		lvc.mask = LVCF_FMT;
		ListView_GetColumn(hwnd, iSubItem, &lvc);

		switch (lvc.fmt & LVCFMT_JUSTIFYMASK)
		{
			case LVCFMT_LEFT:
			fmt |= DT_LEFT;
			break;

			case LVCFMT_CENTER:
			fmt |= DT_CENTER;
			break;

			case LVCFMT_RIGHT:
			fmt |= DT_RIGHT;
			break;
		}
	}

	switch (iSubItem)
	{
		case 0:
		{
			if (item->dwHintMask & SFM_NAME)
			{
				PaintPartlyHighlight(
					hdc,
					prc,
					buf,
					item->ranges,
					item->dwRanges
					);
			}
			else
			{
				SelectObject(hdc, hFontNormal);
				PaintNormal(hdc, buf, -1, prc, fmt);
			}
		}
		break;

		case 1:
		{
			SelectObject(hdc, item->dwHintMask & SFM_CODE ? hFontBold : hFontNormal);
			PaintNormal(hdc, buf, -1, prc, fmt);
		}
		break;

		case 2:
		{
			SelectObject(hdc, item->dwHintMask & SFM_SCOPE ? hFontBold : hFontNormal);
			PaintNormal(hdc, buf, -1, prc, fmt);
		}
		break;

		case 3:
		{
			SelectObject(hdc, hFontNormal);
			PaintNormal(hdc, buf, -1, prc, fmt);
		}
		break;
	}
}

static int MeasurePartlyHighlight(
	HDC hdc,
	LPCTSTR lpText,
	HighlightRange *ranges,
	DWORD dwRangeCount
	)
{
	DWORD dwLen, idx;
	int width = 0;
	DWORD now, next;
	HFONT hFontOld;
	int iOldMode;
	SIZE size;
	BOOL bBold;

	dwLen = _tcslen(lpText);
	if (!dwLen)
		return width;

	hFontOld = (HFONT)SelectObject(hdc, hFontNormal);
	iOldMode = SetBkMode(hdc, TRANSPARENT);

	/* we collect extent data directly */
	now = 0, idx = 0;
	while (now < dwLen)
	{
		if (idx < dwRangeCount)
		{
			if (now == ranges[idx].dwStart)
				next = ranges[idx++].dwEnd, bBold = TRUE;
			else
				next = ranges[idx].dwStart, bBold = FALSE;
		}
		else
			next = dwLen, bBold = FALSE;

		SelectObject(hdc, bBold ? hFontBold : hFontNormal);

		GetTextExtentExPoint(
			hdc,
			lpText + now,
			next - now,
			0,
			NULL,
			NULL,
			&size
			);

		width += size.cx;

		now = next;
	}

	SetBkMode(hdc, iOldMode);
	SelectObject(hdc, hFontOld);

	return width;
}

static int MeasureItem(HWND hwnd, int iItem, int iSubItem, int *piWidthUnwrap)
{
	ListItem *item = ctxList.item + iItem;
	int width = 0;
	int times = 6;
	int cxEdge = GetSystemMetrics(SM_CXEDGE);
	HDC hdc = GetDC(hwnd);
	TCHAR buf[LVITEM_MAXLEN];
	BOOL bUseBoldFont;
	SIZE size;

	/* we must ensure the text is in pszText instead of one new pointer */
	{
		LVITEM it;

		it.mask = LVIF_TEXT;
		it.iItem = iItem;
		it.iSubItem = iSubItem;
		it.pszText = buf;
		it.cchTextMax = ARRAYSIZE(buf);

		ListView_GetItem(hwnd, &it);
	}

	switch (iSubItem)
	{
		case 0:
		{
			if (item->dwHintMask & SFM_NAME)
			{
				width = MeasurePartlyHighlight(hdc, buf, item->ranges, item->dwRanges);
				*piWidthUnwrap = width;
				width += 2 * cxEdge;
			}
			else
			{
				bUseBoldFont = FALSE;
				times = 2;
				goto MEASURE;
			}
		}
		break;

		case 1:
		bUseBoldFont = item->dwHintMask & SFM_CODE;
		goto MEASURE;

		case 2:
		bUseBoldFont = item->dwHintMask & SFM_SCOPE;
		goto MEASURE;

		case 3:
		bUseBoldFont = FALSE;
		goto MEASURE;

		MEASURE:
		{
			HFONT hFontOld;
			int iOldMode;

			iOldMode = SetBkMode(hdc, TRANSPARENT);
			hFontOld = (HFONT)SelectObject(
				hdc,
				bUseBoldFont ? hFontBold : hFontNormal
				);
			GetTextExtentExPoint(
			hdc,
			buf,
			_tcslen(buf),
			0,
			NULL,
			NULL,
			&size
			);
			SelectObject(hdc, hFontOld);
			SetBkMode(hdc, iOldMode);

			*piWidthUnwrap = size.cx;

			width = size.cx + times * cxEdge;
		}
		break;

		default:
		break;
	}

	ReleaseDC(hwnd, hdc);

	return width;
}

static void LvOnCmd(HWND hdlg, ListItem *p, UINT id, LPTSTR lpDesc)
{
	TCHAR buf[32];
	LPCTSTR szCopy = NULL;

	switch (id)
	{
		case ID_CTXMENU_COPYNAME:
		if (CopyText(
			hdlg,
			pecs[p->dwScope].items[p->dwItem].lpName
			))
		{
			szCopy = pecs[p->dwScope].items[p->dwItem].lpName;
		}
		break;

		case ID_CTXMENU_COPYVAL:
		_stprintf(
			buf,
			TEXT("%u"),
			pecs[p->dwScope].items[p->dwItem].dwCode
			);
		if (CopyText(hdlg, buf))
		{
			szCopy = buf;
		}
		break;

		case ID_CTXMENU_COPYHEX:
		_stprintf(
			buf,
			TEXT("0x%X"),
			pecs[p->dwScope].items[p->dwItem].dwCode
			);
		if (CopyText(hdlg, buf))
		{
			szCopy = buf;
		}
		break;

		case ID_CTXMENU_COPYSCOPE:
		{
			if (CopyText(hdlg, pecs[p->dwScope].lpName))
			{
				szCopy = pecs[p->dwScope].lpName;
			}
		}
		break;

		case ID_CTXMENU_COPYDESC:
		if (lpDesc)
		{
			if (CopyText(hdlg, lpDesc))
			{
				szCopy = lpDesc;
			}
		}
		break;

		case ID_CTXMENU_SEARCH:
		SearchInMSDN(pecs[p->dwScope].items[p->dwItem].lpName);
		break;
	}

	if (szCopy)
	{
		TCHAR fmt[MAX_STRING_LENGTH];
		TCHAR tip[STATUS_MAXLEN];

		LoadString(hInst, IDS_S_COPIED, fmt, MAX_STRING_LENGTH);
		_sntprintf(tip, STATUS_MAXLEN, fmt, szCopy);

		SetStatusTextTemp(hdlg, tip);
	}
}

static void LvAdjustArrow(HWND hctl, int iCol)
{
	HWND hHeader;
	HD_ITEM item;

	hHeader = ListView_GetHeader(hctl);
	item.mask = HDI_FORMAT;

	if (iCol == ctxSort.idx)
	{
		if (-1 != iCol)
		{
			ctxSort.sDirection *= -1;
			Header_GetItem(hHeader, iCol, &item);

			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (0 != ctxSort.sDirection)
			{
				item.fmt |= ctxSort.sDirection > 0 ? HDF_SORTUP : HDF_SORTDOWN;
			}
			Header_SetItem(hHeader, iCol, &item);
		}
	}
	else
	{
		if (-1 != ctxSort.idx)
		{
			Header_GetItem(hHeader, ctxSort.idx, &item);
			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			Header_SetItem(hHeader, ctxSort.idx, &item);
		}

		ctxSort.idx = iCol, ctxSort.sDirection = 1;
		if (-1 != iCol)
		{
			Header_GetItem(hHeader, iCol, &item);

			item.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (0 != ctxSort.sDirection)
			{
				item.fmt |= ctxSort.sDirection > 0 ? HDF_SORTUP : HDF_SORTDOWN;
			}
			Header_SetItem(hHeader, iCol, &item);
		}
	}
}

static void LvOnGetDispInfo(HWND hdlg, HWND hctl, LVITEM *item)
{
	TCHAR buf[MAX_STRING_LENGTH];
	DWORD dwCode;

	if (item->mask & LVIF_TEXT)
	{
		ListItem *it = ctxList.item + item->iItem;
		LPCTSTR lpText;

		switch (item->iSubItem)
		{
			case 0:
			lpText = pecs[it->dwScope].items[it->dwItem].lpName;
			lstrcpyn(item->pszText, lpText, item->cchTextMax);
			break;

			case 1:
			dwCode = pecs[it->dwScope].items[it->dwItem].dwCode;
			_stprintf(buf, TEXT("0x%X"), dwCode);
			lstrcpyn(item->pszText, buf, item->cchTextMax);
			break;

			case 2:
			lpText = pecs[it->dwScope].lpName;
			lstrcpyn(item->pszText, lpText, item->cchTextMax);
			break;

			case 3:
			dwCode = pecs[it->dwScope].items[it->dwItem].dwCode;
			lpText = pecs[it->dwScope].pfnMessageFromCode(dwCode);
			lstrcpyn(item->pszText, lpText, item->cchTextMax);
			pecs[it->dwScope].pfnFreeMessage((LPTSTR)lpText);
			break;
		}
	}
}

static int LvOnFindItem(HWND hdlg, HWND hctl, LVFINDINFO *fi)
{
	int idx = -1;

	if (fi->flags & (LVFI_SUBSTRING | LVFI_PARTIAL))
	{
		DWORD i, n, dwCurrent, dwStart;
		int iCurrentSel;
		ListItem *p;

		iCurrentSel = ListView_GetSelectionMark(hctl);
		if (-1 == iCurrentSel)
			dwStart = 0;
		else
			dwStart = iCurrentSel;

		n = ctxList.dwCount;
		for (i = 0; i < n; ++i)
		{
			dwCurrent = dwStart + i;
			if (dwCurrent >= n)
				dwCurrent -= n;
			p = ctxList.item + dwCurrent;

			if (TcsStartsWith(pecs[p->dwScope].items[p->dwItem].lpName, fi->psz))
			{
				idx = dwCurrent;
				break;
			}
		}
	}

	return idx;
}

static void LvOnColumnClick(HWND hdlg, HWND hctl, int iCol)
{
	int iCurrentSel = ListView_GetSelectionMark(hctl);
	BOOL bSelectedItemVisible =
		ListView_IsItemVisible(hctl, iCurrentSel);
	ListItem select;

	if (-1 != iCurrentSel)
	{
		select = ctxList.item[iCurrentSel];
	}

	if (iCol == ctxSort.idx)
	{
		LvSort(-1, TRUE);
	}
	else
	{
		if (!LvSort(iCol, FALSE))
			return;
	}

	LvAdjustArrow(hctl, iCol);
	if (-1 != iCurrentSel)
	{
		int iSel = -1;
		DWORD i, dwCount = ctxList.dwCount;
		ListItem *p = ctxList.item;

		for (i = 0; i < dwCount; ++i, ++p)
		{
			if (p->dwItem == select.dwItem && p->dwScope == select.dwScope)
				iSel = i;

		}
		LvSetSel(hctl, iSel, bSelectedItemVisible);
	}

	ListView_RedrawItems(hctl, 0, ctxList.dwCount - 1);
}

static void LvOnItemChanged(HWND hdlg, HWND hctl, NMLISTVIEW *item)
{
	ListItem *p = ctxList.item + item->iItem;
	if (item->uNewState & LVIS_SELECTED)
	{
		TCHAR fmt[MAX_STRING_LENGTH], buf[256];
		LoadString(hInst, IDS_S_ITEMMSG, fmt, MAX_STRING_LENGTH);
		_sntprintf(
			buf,
			256,
			fmt,
			pecs[p->dwScope].items[p->dwItem].lpName,
			pecs[p->dwScope].items[p->dwItem].dwCode
			);
		SetStatusText(hdlg, buf);
	}
}

static void LvOnRClick(HWND hdlg, HWND hctl, NMITEMACTIVATE *item)
{
	HMENU hPopup;
	POINT pt = item->ptAction;
	ListItem *p = ctxList.item + item->iItem;
	if (-1 != item->iItem)
	{
		hPopup = LoadPopupMenu(IDR_CONTEXTMENU);
		if (hPopup)
		{
			UINT id;
			LPTSTR lpDesc;

			SetMenuDefaultItem(
				hPopup,
				ID_CTXMENU_COPYNAME + item->iSubItem,
				MF_BYCOMMAND
				);
			lpDesc = pecs[p->dwScope].pfnMessageFromCode(
				pecs[p->dwScope].items[p->dwItem].dwCode
				);
			if (!lpDesc)
			{
				DeleteMenu(hPopup, ID_CTXMENU_COPYDESC, MF_BYCOMMAND);
			}

			ClientToScreen(hctl, &pt);
			id = (UINT)TrackPopupMenuEx(
				hPopup,
				TPM_RETURNCMD,
				pt.x,
				pt.y,
				hdlg,
				NULL
				);
			LvOnCmd(hdlg, p, id, lpDesc);

			if (lpDesc)
				pecs[p->dwScope].pfnFreeMessage(lpDesc);

			DestroyMenu(hPopup);
		}
	}
}

static void LvOnDBClick(HWND hdlg, HWND hctl, NMITEMACTIVATE *item)
{
	ListItem *p = ctxList.item + item->iItem;
	if (-1 != item->iItem)
	{
		LPTSTR lpDesc;
		lpDesc = pecs[p->dwScope].pfnMessageFromCode(
			pecs[p->dwScope].items[p->dwItem].dwCode
			);

		LvOnCmd(hdlg, p, ID_CTXMENU_COPYNAME + item->iSubItem, lpDesc);

		if (lpDesc)
			pecs[p->dwScope].pfnFreeMessage(lpDesc);
	}
}

static void LvOnDraw(HWND hdlg, HWND hctl, NMLVCUSTOMDRAW *ctx)
{
	LONG_PTR lResult = CDRF_DODEFAULT;
	switch (ctx->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		lResult = CDRF_NOTIFYITEMDRAW;
		break;

		case CDDS_ITEMPREPAINT:
		if (bAltColor && !(ctx->nmcd.dwItemSpec & 1))

			ctx->clrTextBk = BlendColor(ListView_GetBkColor(hctl));

		lResult = CDRF_NOTIFYSUBITEMDRAW;
		break;

		case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
		{
			ListItem *item = ctxList.item + ctx->nmcd.dwItemSpec;
			/* we don't care about listview calculating the text width */
			if (!(
				ctx->nmcd.rc.left == 0
				&& ctx->nmcd.rc.top == 0
				&& ctx->nmcd.rc.right == 0
				&& ctx->nmcd.rc.bottom == 0
				))
			{
				lResult = CDRF_DOERASE | CDRF_NOTIFYPOSTPAINT;
				switch (ctx->iSubItem)
				{
					case 0:
					if (item->dwHintMask & SFM_NAME)
						lResult = CDRF_DOERASE | CDRF_NOTIFYPOSTPAINT;
					else
						SelectObject(ctx->nmcd.hdc, hFontNormal);
					break;

					case 1:
					SelectObject(
						ctx->nmcd.hdc,
						(item->dwHintMask & SFM_CODE) ? hFontBold : hFontNormal
						);
					break;

					case 2:
					SelectObject(
						ctx->nmcd.hdc,
						(item->dwHintMask & SFM_SCOPE) ? hFontBold : hFontNormal
						);
					break;

					case 3:
					SelectObject(ctx->nmcd.hdc, hFontNormal);
					break;

				}
			}
		}
		break;
		case CDDS_ITEMPOSTPAINT | CDDS_SUBITEM:
		{
			RECT rc = ctx->nmcd.rc;
			int cxEdge = GetSystemMetrics(SM_CXEDGE);

			rc.left += 3 * cxEdge;
			rc.right -= 3 * cxEdge;
			if (ctx->iSubItem == 0)
			{
				rc.left -= 2 * cxEdge;
				rc.right += 2 * cxEdge;
			}

			paint_item(hctl, ctx->nmcd.hdc, &rc, ctx->nmcd.dwItemSpec, ctx->iSubItem);
		}
		break;

	}

	SetWindowLongPtr(hdlg, DWLP_MSGRESULT, lResult);
}

static void LvGetMarkup(HWND hdlg, HWND hctl, NMLVEMPTYMARKUP *markup)
{
	LONG_PTR lResult = TRUE;

	markup->dwFlags = EMF_CENTERED;
	LoadStringW(hInst, IDS_HINT_EMPTY, markup->szMarkup, L_MAX_URL_LENGTH);
	SetWindowLongPtr(hdlg, DWLP_MSGRESULT, lResult);
}

BOOL LvOnNotify(HWND hdlg, NMHDR *hdr)
{
	HWND hctl = hdr->hwndFrom;

	switch (hdr->code)
	{
		case LVN_GETDISPINFO:
		LvOnGetDispInfo(hdlg, hctl, &((LV_DISPINFO*)hdr)->item);
		break;

		case LVN_ODFINDITEM:
		SetWindowLongPtr(
			hdlg,
			DWLP_MSGRESULT,
			LvOnFindItem(hdlg, hctl, &((NMLVFINDITEM*)hdr)->lvfi)
			);
		break;

		case LVN_COLUMNCLICK:
		LvOnColumnClick(hdlg, hctl, ((NMLISTVIEW*)hdr)->iSubItem);
		break;

		case LVN_ITEMCHANGED:
		LvOnItemChanged(hdlg, hctl, (NMLISTVIEW*)hdr);
		break;

		case NM_RCLICK:
		LvOnRClick(hdlg, hctl, (NMITEMACTIVATE*)hdr);
		break;

		case NM_DBLCLK:
		LvOnDBClick(hdlg, hctl, (NMITEMACTIVATE*)hdr);
		break;

		case NM_CUSTOMDRAW:
		LvOnDraw(hdlg, hctl, (NMLVCUSTOMDRAW*)hdr);
		break;

		case LVN_GETEMPTYMARKUP:
		LvGetMarkup(hdlg, hctl, (NMLVEMPTYMARKUP*)hdr);
		break;

		default:
		return FALSE;
	}

	return TRUE;
}

extern WNDPROC pfnLVProc;
static int iTipTrack = -1;
static int extentTip;

static BOOL LvGetCurrentTip(HWND hwnd, int *piItem, int *piSubItem)
{
	if (iTipTrack == -1)
	{
		LVHITTESTINFO hti;

		GetMsgPosClient(hwnd, &hti.pt);
		hti.flags = LVHT_ONITEM;

		if (-1 != ListView_SubItemHitTest(hwnd, &hti))
		{
			*piItem = hti.iItem;
			*piSubItem = hti.iSubItem;
			return TRUE;
		}
	}
	else
	{
		if (iTipTrack < ListView_GetItemCount(hwnd))
		{
			*piItem = iTipTrack;
			*piSubItem = 0;
			return TRUE;
		}
	}

	return FALSE;
}

void LvInitFont(HWND hwnd)
{
	HFONT hFont;
	LOGFONT lf;

	if (hFontNormal)
		DeleteObject(hFontNormal);
	if (hFontBold)
		DeleteObject(hFontBold);

	hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
	if (!hFont)
		hFont = GetStockObject(SYSTEM_FONT);

	if (GetObject(hFont, sizeof (lf), &lf))
	{
		hFontNormal = CreateFontIndirect(&lf);

		lf.lfWeight = FW_BOLD;
		hFontBold = CreateFontIndirect(&lf);
	}
}

/* Dirty hack, but it works. */
#define IDT_TRACKINGTIP	 48

static LRESULT LvxhOnDbClick(HWND hwnd, NMHEADERW *header)
{
	HDITEMW item;
	LRESULT lResult = 0;
	int i, iStart, iEnd, width = 0, extent;

	iStart = ListView_GetTopIndex(hwnd);
	iEnd = iStart + ListView_GetCountPerPage(hwnd);

	if (iStart >= 0)
	{
		if (iEnd > ctxList.dwCount)
			iEnd = ctxList.dwCount;

		for (i = iStart; i < iEnd; ++i)
		{
			MeasureItem(hwnd, i, header->iItem, &extent);
			if (extent > width)
				width = extent;
		}
	}

	width += GetSystemMetrics(SM_CXEDGE) * (header->iItem ? 7 : 5);
	item.mask = HDI_WIDTH;
	item.cxy = width;
	SendMessage(
		header->hdr.hwndFrom,
		HDM_SETITEMW,
		header->iItem,
		(LPARAM)&item
		);

	SetFocus(hwnd);
	return lResult;
}

static void LvxtOnGetDispInfo(HWND hwnd, NMTTDISPINFOW *info)
{
	int iItem, iSubItem;

	if (LvGetCurrentTip(hwnd, &iItem, &iSubItem))
	{
		RECT rc, rcClient;
		int width;

		ListView_GetSubItemRect(hwnd, iItem, iSubItem, LVIR_LABEL, &rc);
		rc.right -= GetSystemMetrics(SM_CXEDGE);
		GetClientRect(hwnd, &rcClient);
		IntersectRect(&rc, &rc, &rcClient);

		width = MeasureItem(
			hwnd,
			iItem,
			iSubItem,
			&extentTip
			);

		/* we need to popup the tooltip */
		if (rc.left + width > rc.right)
		{
			/* to use tooltips, we have to set something */
			info->szText[0] = L' ';
			info->szText[1] = L'\0';
		}
	}
}

static LRESULT LvxtOnDraw(HWND hwnd, NMTTCUSTOMDRAW *ctx)
{
	int iItem, iSubItem;

	switch (ctx->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
		/* make the placeholder invisible */
		SetTextColor(
			ctx->nmcd.hdc,
			(COLORREF)SendMessage(ctx->nmcd.hdr.hwndFrom, TTM_GETTIPBKCOLOR, 0, 0)
			);
		return CDRF_NOTIFYPOSTPAINT;

		case CDDS_POSTPAINT:
		{
			if (LvGetCurrentTip(hwnd, &iItem, &iSubItem))
			{
				SetTextColor(ctx->nmcd.hdc, 0);
				paint_item(hwnd, ctx->nmcd.hdc, &ctx->nmcd.rc, iItem, iSubItem);
			}
		}
		return CDRF_SKIPDEFAULT;

		default:
		return CDRF_DODEFAULT;
	}
}

static LRESULT LvxtOnShow(HWND hwnd, HWND hctl)
{
	int iItem, iSubItem;

	if (LvGetCurrentTip(hwnd, &iItem, &iSubItem))
	{
		RECT rc;
		POINT pt;
		int height;

		ListView_GetSubItemRect(hwnd, iItem, iSubItem, LVIR_LABEL, &rc);
		height = rc.bottom - rc.top;

		pt.x = rc.left;
		pt.y = rc.top;
		ClientToScreen(hwnd, &pt);

		rc.left = pt.x + GetSystemMetrics(SM_CXEDGE)
			* (iSubItem == 0 ? 1 : 3);
		rc.top = pt.y;
		rc.right = rc.left + extentTip;
		rc.bottom = rc.top + height;

		SendMessage(
			hctl,
			TTM_ADJUSTRECT,
			TRUE,
			(LPARAM)&rc
			);

		SetWindowPos(
			hctl,
			NULL,
			rc.left,
			rc.top,
			rc.right - rc.left,
			rc.bottom - rc.top,
			SWP_NOACTIVATE
			);

		return TRUE;
	}

	return 0;
}

LRESULT CALLBACK LvxProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_NOTIFY:
		{
			NMHDR *hdr = (NMHDR*)lParam;

			if (hdr->hwndFrom == ListView_GetHeader(hwnd))
			{
				switch (hdr->code)
				{
					/* The oldproc is under unicode, so this is
					 * always 'W' no matter what our version is. */
					case HDN_DIVIDERDBLCLICKW:
					return LvxhOnDbClick(hwnd, (NMHEADERW*)hdr);

					default:
					break;
				}
			}

			if (hdr->hwndFrom == ListView_GetToolTips(hwnd))
			{
				switch (hdr->code)
				{
					/* use 'W' version */
					case TTN_GETDISPINFOW:
					LvxtOnGetDispInfo(hwnd, (NMTTDISPINFOW*)hdr);
					return 0;

					case NM_CUSTOMDRAW:
					return LvxtOnDraw(hwnd, (NMTTCUSTOMDRAW*)hdr);

					case TTN_SHOW:
					return LvxtOnShow(hwnd, hdr->hwndFrom);

				}
			}
		}
		goto DODEFPROC;

		case WM_SETFONT:
		{
			LRESULT lResult = CallWindowProc(pfnLVProc, hwnd, uMsg, wParam, lParam);

			LvInitFont(hwnd);
			return lResult;
		}

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			int iFocus = ListView_GetSelectionMark(hwnd);
			LRESULT lResult;
			int iFocusNew;

			lResult = CallWindowProc(pfnLVProc, hwnd, uMsg, wParam, lParam);
			iFocusNew = ListView_GetSelectionMark(hwnd);
			if (iFocusNew != iFocus)
			{
				iTipTrack = iFocusNew;
			}

			return lResult;
		}

		case WM_TIMER:
		{
			if (wParam == IDT_TRACKINGTIP)
			{
				LRESULT lResult = CallWindowProc(pfnLVProc, hwnd, uMsg, wParam, lParam);
				iTipTrack = -1;

				return lResult;
			}
		}
		goto DODEFPROC;

		case WM_DESTROY:
		ItemFree();
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)pfnLVProc);
		goto DODEFPROC;

		default:
DODEFPROC:
		return CallWindowProc(pfnLVProc, hwnd, uMsg, wParam, lParam);
	}
}

