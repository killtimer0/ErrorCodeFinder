#include "ecf.h"

#define ARRAY_EXPAND_THRESHOLD	0.5F

BOOL ArrayInit(TArray *ary, DWORD dwCell, DWORD dwSizeHint)
{
	ary->dwCount = 0;
	ary->dwCell = dwCell;
	ary->dwMaxCount = dwSizeHint ? dwSizeHint : 1;
	ary->lpData = (LPVOID)LocalAlloc(
		LMEM_FIXED,
		ary->dwCell * ary->dwMaxCount
		);

	return NULL != ary->lpData;
}

void ArrayClear(TArray *ary)
{
	LocalFree(ary->lpData);

	ary->lpData = NULL;
	ary->dwCell = 0;
	ary->dwCount = 0;
	ary->dwMaxCount = 0;
}

LPVOID ArrayFinish(TArray *ary, DWORD *pdwCount)
{
	LPVOID lpData = ary->lpData;
	DWORD dwSize = ary->dwCell * ary->dwCount;
	DWORD dwCount = ary->dwCount;

	ary->lpData = NULL;
	ary->dwCell = 0;
	ary->dwCount = 0;
	ary->dwMaxCount = 0;

	*pdwCount = dwCount;
	if (!dwSize)
	{
		LocalFree(lpData);
		return NULL;
	}

	/* shrink size */
	LocalReAlloc(lpData, dwSize, 0);
	return lpData;
}

BOOL ArrayAppend(TArray *ary, LPVOID data)
{
	/* make sure we have enough space */
	if (ary->dwCount == ary->dwMaxCount)
	{
		LPVOID p;
		DWORD dwMaxCount = ary->dwMaxCount;

		dwMaxCount += (DWORD)(dwMaxCount * ARRAY_EXPAND_THRESHOLD);
		if (dwMaxCount <= ary->dwCount)
			dwMaxCount = ary->dwCount + 1;

		p = LocalReAlloc(
			ary->lpData,
			ary->dwCell * dwMaxCount,
			LMEM_MOVEABLE
			);

		if (!p)
			return FALSE;

		ary->lpData = p;
		ary->dwMaxCount = dwMaxCount;
	}

	memmove(
		(LPVOID)((SIZE_T)ary->lpData + ary->dwCell * ary->dwCount),
		data,
		ary->dwCell
		);
	++ary->dwCount;

	return TRUE;
}

static BOOL tciequal(TCHAR c1, TCHAR c2, BOOL bWildcard)
{
	TCHAR c = c1;

	if (c1 == c2)
		return TRUE;

	if (bWildcard && c2 == TEXT('?'))
		return TRUE;

	if (c1 > c2)
		c1 = c2, c2 = c;

	return c1 >= TEXT('A')
		&& c1 <= TEXT('Z')
		&& c1 + TEXT('a') == c2 + TEXT('A');
}

static LPTSTR TcsMatchWildcard(LPCTSTR str, LPCTSTR sub, SIZE_T len)
{
	LPTSTR cp = (LPTSTR)str;
	LPTSTR s1, s2, end;

	if (!*sub)
		return (LPTSTR)str;

	end = (LPTSTR)sub + len;
	while (*cp)
	{
		s1 = cp;
		s2 = (LPTSTR)sub;

		while (*s1 && s2 < end && tciequal(*s1, *s2, TRUE))
			++s1, ++s2;

		if (s2 == end)
			return cp;

		++cp;
	}

	return NULL;
}

BOOL MatchWildCard(
	LPCTSTR lpText,
	LPCTSTR lpPattern,
	DWORD *pdwRanges,
	HighlightRange **ppRange
	)
{
	LPTSTR str = (LPTSTR)lpText;
	LPTSTR pat = (LPTSTR)lpPattern;
	LPTSTR p, q, next, s = str;
	TArray ary;
	HighlightRange range;
	BOOL bNoCache = FALSE;
	DWORD dwPatLen;

	if (!*pat)
		return FALSE;

	if (!ArrayInit(&ary, sizeof (HighlightRange), 1))
		bNoCache = TRUE;

	while (*pat)
	{
		q = _tcschr(pat, TEXT('*'));
		if (q)
		{
			next = q + 1;
		}
		else
		{
			q = _tcschr(pat, TEXT('\0'));
			next = q;
		}

		dwPatLen = q - pat;
		s = TcsMatchWildcard(s, pat, dwPatLen);

		if (!s)
		{
			if (!bNoCache)
				ArrayClear(&ary);

			return FALSE;
		}

		range.dwStart = s - lpText;
		range.dwEnd = range.dwStart + dwPatLen;
		if (!bNoCache)
			ArrayAppend(&ary, &range);

		s += dwPatLen;
		pat = next;
	}

	/* collect search data */
	if (bNoCache)
	{
		*pdwRanges = 0;
		*ppRange = NULL;
	}
	else
	{
		*ppRange = (HighlightRange*)ArrayFinish(&ary, pdwRanges);
	}

	return TRUE;
}

static int tcsicmpn(LPCTSTR s, LPCTSTR t, SIZE_T l)
{
	SIZE_T i;

	for (i = 0; i < l; ++i)
	{
		if (!tciequal(s[i], t[i], FALSE))
			return s[i] - t[i];
	}

	return s[l];
}

BOOL TcsStartsWith(LPCTSTR p, LPCTSTR q)
{
	while (*p && *q)
	{
		if (!tciequal(*p, *q, FALSE))
			return FALSE;

		++p, ++q;
	}

	return TRUE;
}

static BOOL FmtFindNextToken(
	LPCTSTR fmt,
	DWORD *pos,
	DWORD *l,
	DWORD *r
	)
{
	LPCTSTR p = fmt + *pos;

	while (_istspace(*p)) ++p;
	if (!*p) return FALSE;
	*l = p - fmt;

	while (*p && !_istspace(*p)) ++p;
	*r = p - fmt;

	*pos = *r;
	return TRUE;
}

static BOOL FmtParseHexDigit(TCHAR c, DWORD *r)
{
	if (c >= TEXT('0') && c <= TEXT('9'))
		*r = c - TEXT('0');
	else if (c >= TEXT('A') && c <= TEXT('F'))
		*r = c - TEXT('A') + 10;
	else if (c >= TEXT('a') && c <= TEXT('f'))
		*r = c - TEXT('a') + 10;
	else
		return FALSE;

	return TRUE;
}

BOOL FmtParseDword(LPCTSTR fmt, DWORD l, DWORD r, DWORD *dwResult)
{
	DWORD i, c, d;
	ULONGLONG e;

	/* is 0xXXX? */
	if (r > l + 2 && r <= l + 2 + 8)
	{
		if (fmt[l] == TEXT('0') && (
			fmt[l + 1] == TEXT('X')
			|| fmt[l + 1] == TEXT('x')
			))
		{
			BOOL bHex = TRUE;

			c = 0;
			/* from MSB to LSB */
			for (i = l + 2; i < r; ++i)
			{
				if (FmtParseHexDigit(fmt[i], &d))
				{
					c = c << 4 | d;
				}
				else
				{
					bHex = FALSE;
					break;
				}
			}

			if (bHex)
			{
				*dwResult = c;
				return TRUE;
			}
		}
	}

	/* is dXXXh? */
	if (r >= l + 2
		&& r <= l + 1 + 8 + 1
		&& (fmt[r - 1] == TEXT('H') || fmt[r - 1] == TEXT('h'))
		&& fmt[l] >= TEXT('0')
		&& fmt[l] <= TEXT('9')
		)
	{
		BOOL bHex = TRUE;

		e = 0;
		for (i = l; i < r - 1; ++i)
		{
			if (FmtParseHexDigit(fmt[i], &d))
			{
				e = e << 4 | d;
			}
			else
			{
				bHex = FALSE;
				break;
			}
		}

		if (bHex && (e >> 32) == 0)
		{
			*dwResult = e;
			return TRUE;
		}
	}

	/* is [+-]ddd? */
	if (r >= l + 1 && r <= l + 1 + 10)
	{
		BOOL bSign = FALSE;
		BOOL bDone = FALSE;

		if (fmt[l] == TEXT('-'))
		{
			bSign = TRUE;
			++l;
		}
		else if (fmt[l] == TEXT('+'))
			++l;

		e = 0;
		for (i = l; i < r; ++i)
		{
			if (fmt[i] >= TEXT('0') && fmt[i] <= TEXT('9'))
			{
				e = e * 10 + (fmt[i] - TEXT('0'));
			}
			else
			{
				bDone = FALSE;
				break;
			}

			/* at least one digit */
			bDone = TRUE;
		}

		if (bDone)
		{
			if (bSign && e <= 0x80000000ULL)
			{
				*dwResult = -e;
				return TRUE;
			}
			else if (!bSign && (e >> 32) == 0)
			{
				*dwResult = e;
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL FilterFromString(LPCTSTR fmt, SearchFilter *filter)
{
	BOOL result = FALSE;
	DWORD pos = 0, l, r, i, j;

	filter->dwMask = 0;
	filter->dwCode = 0;
	filter->dwNameCount = 0;
	filter->dwScopeCount = 0;

	result = TRUE;
	while (FmtFindNextToken(fmt, &pos, &l, &r))
	{
		if (FmtParseDword(fmt, l, r, &filter->dwCode))
			filter->dwMask |= SFM_CODE;
		else
		{
			BOOL bExist = FALSE;

			/* keywords prefixed with ':' are resolved to scopes */
			if (fmt[l] == TEXT(':'))
			{
				filter->dwMask |= SFM_SCOPE;
				++l;

				/* is the keyword in scope? */
				for (i = 0; i < necs; ++i)
				{
					if (!tcsicmpn(pecs[i].lpName, fmt + l, r - l))
					{
						if (filter->dwScopeCount < SF_MAX_NAMES)
						{
							for (j = 0; j < filter->dwScopeCount; ++j)
							{
								if (filter->dwScopes[j] == i)
								{
									bExist = TRUE;
									break;
								}
							}
							if (!bExist)
							{
								filter->dwScopes[filter->dwScopeCount++] = i;
							}
						}
						break;
					}
				}
			}
			else
			{
				/* it's a pattern for names */
				if (filter->dwNameCount < SF_MAX_NAMES)
				{
					if (l < r)
					{
						for (i = 0; i < filter->dwNameCount; ++i)
						{
							if (!_tcsnicmp(filter->lpNames[i], fmt + l, r - l))
							{
								bExist = TRUE;
								break;
							}
						}

						if (!bExist)
						{
							LPTSTR lpName =
								(LPTSTR)LocalAlloc(LPTR, (r - l + 1) * sizeof (TCHAR));
							if (lpName)
							{
								memmove(lpName, fmt + l, (r - l) * sizeof (TCHAR));
								lpName[r - l] = TEXT('\0');
								filter->lpNames[filter->dwNameCount++] = lpName;
								filter->dwMask |= SFM_NAME;
							}
						}
					}
				}
			}
		}
	}

	return result;
}

void FilterClear(SearchFilter *filter)
{
	DWORD dwNameCount, i;
	
	dwNameCount = filter->dwNameCount;

	for (i = 0; i < dwNameCount; ++i)
	{
		LocalFree(filter->lpNames[i]);
		filter->lpNames[i] = NULL;
	}

	filter->dwMask = 0;
	filter->dwCode = 0;
	filter->dwNameCount = 0;
	filter->dwScopeCount = 0;
}

COLORREF BlendColor(COLORREF color)
{
	BYTE r = (color >> 0) & 0xFF;
	BYTE g = (color >> 8) & 0xFF;
	BYTE b = (color >> 16) & 0xFF;
	BYTE a = (color >> 24) & 0xFF;

	if (a)
		return color;
	else
		return RGB(
			r * (1 - ALTALPHABK) + 0x80 * ALTALPHABK,
			g * (1 - ALTALPHABK) + 0x80 * ALTALPHABK,
			b * (1 - ALTALPHABK) + 0x80 * ALTALPHABK
			);
}

