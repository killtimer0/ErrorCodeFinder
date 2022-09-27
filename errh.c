#include "ecf.h"
#include <dbghelp.h>

static DWORD WINAPI ErrorReport(LPVOID e){
	return MessageBox(NULL, (LPCTSTR)e, NULL, MB_OK | MB_ICONERROR);
}

LONG WINAPI BugHandler(EXCEPTION_POINTERS *e)
{
	TCHAR s[MAX_PATH + 4];
	TCHAR t[512];
	DWORD dwLen;
	HANDLE hFile;
	HANDLE hThread;

	dwLen = GetModuleFileName(NULL, s, MAX_PATH);
	_tcscpy(s + dwLen, TEXT(".dmp"));

	hFile = CreateFile(
		s,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);

	if (INVALID_HANDLE_VALUE != hFile){
		MINIDUMP_EXCEPTION_INFORMATION info;

		info.ThreadId = GetCurrentThreadId();
		info.ExceptionPointers = e;
		info.ClientPointers = TRUE;
		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpWithFullMemory,
			&info,
			NULL,
			NULL
			);

		CloseHandle(hFile);
	}

	_sntprintf(
		t,
		512,
		TEXT("An error occurred and the program must be closed.\n")
		TEXT("Address: 0x%p\tErrcode: 0x%08X\n")
		TEXT("See more information in %s."),
		e->ExceptionRecord->ExceptionAddress,
		e->ExceptionRecord->ExceptionCode,
		s
		);

	hThread = CreateThread(NULL, 0, ErrorReport, t, 0, NULL);
	if (hThread)
	{
		WaitForSingleObject(hThread, INFINITE);
		CloseHandle(hThread);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

