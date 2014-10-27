#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <tchar.h>

//  Forward declarations:
BOOL ListProcessThreads(DWORD dwOwnerPID);
void printError(TCHAR* msg);

DWORD FindProcessID(LPCTSTR szProcessName)
{
	DWORD dwPID = 0xFFFFFFFF;
	HANDLE hSnapShot = INVALID_HANDLE_VALUE;        //그냥 초기화한것
	PROCESSENTRY32 pe;

	// Get the snapshot of the system
	pe.dwSize = sizeof(PROCESSENTRY32);              // 사용하기 전에 사이즈를 설정한다.
	hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);   //구한 스냅샷을 핸들로 저장

	// find process
	Process32First(hSnapShot, &pe);                           //열심히 구한다.
	do
	{
		if (!_stricmp(szProcessName, pe.szExeFile))     //구하고자 하는 프로세스명이  같으면
		{
			dwPID = pe.th32ProcessID;                            // PID 를 저정한다.
			break;
		}
	} while (Process32Next(hSnapShot, &pe));                // 다음 리스트검색

	CloseHandle(hSnapShot);

	return dwPID;
}

int main(void)
{
	DWORD pid;
	pid = FindProcessID("mspaint.exe");
	ListProcessThreads(4588);
	return 0;
}

BOOL ListProcessThreads(DWORD dwOwnerPID)
{
	HANDLE hThreadSnap = INVALID_HANDLE_VALUE;
	THREADENTRY32 te32;

	// Take a snapshot of all running threads  
	hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	if (hThreadSnap == INVALID_HANDLE_VALUE)
		return(FALSE);

	// Fill in the size of the structure before using it. 
	te32.dwSize = sizeof(THREADENTRY32);

	// Retrieve information about the first thread,
	// and exit if unsuccessful
	if (!Thread32First(hThreadSnap, &te32))
	{
		printError(TEXT("Thread32First"));  // Show cause of failure
		CloseHandle(hThreadSnap);     // Must clean up the snapshot object!
		return(FALSE);
	}

	// Now walk the thread list of the system,
	// and display information about each thread
	// associated with the specified process
	do
	{
		if (te32.th32OwnerProcessID == dwOwnerPID)
		{
			//_tprintf(TEXT("\n     THREAD ID      = 0x%08X"), te32.th32ThreadID);
			_tprintf(TEXT("\n     THREAD ID      = %d"), te32.th32ThreadID);
			//_tprintf(TEXT("\n     base priority  = %d"), te32.tpBasePri);
			//_tprintf(TEXT("\n     delta priority = %d"), te32.tpDeltaPri);
			if (te32.th32ThreadID == 396)
			{
				HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te32.th32ThreadID);
				SuspendThread(hThread);
				Sleep(5000);
				ResumeThread(hThread);
			}
			
		}
	} while (Thread32Next(hThreadSnap, &te32));

	_tprintf(TEXT("\n"));

	//  Don't forget to clean up the snapshot object.
	CloseHandle(hThreadSnap);
	return(TRUE);
}

void printError(TCHAR* msg)
{
	DWORD eNum;
	TCHAR sysMsg[256];
	TCHAR* p;

	eNum = GetLastError();
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, eNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		sysMsg, 256, NULL);

	// Trim the end of the line and terminate it with a null
	p = sysMsg;
	while ((*p > 31) || (*p == 9))
		++p;
	do { *p-- = 0; } while ((p >= sysMsg) &&
		((*p == '.') || (*p < 33)));

	// Display the message
	_tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}