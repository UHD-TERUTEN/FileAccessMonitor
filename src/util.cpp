#include "util.h"
#include "Logger.h"
using namespace Log;

#include <iomanip>
using namespace std;

#include <TlHelp32.h>

static bool SetPrivilege(_In_z_ const wchar_t* privilege, _In_ bool enable)
{
	if (!privilege)
		return false;

	HANDLE token = INVALID_HANDLE_VALUE;
	if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token))
	{
		if (GetLastError() == ERROR_NO_TOKEN)
		{
			if (!ImpersonateSelf(SecurityImpersonation))
				return false; 

			if (!OpenThreadToken(GetCurrentThread(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, FALSE, &token))
				return false;
		}
		else
			return false;
	}

	TOKEN_PRIVILEGES tp{};
	LUID luid{};
	DWORD cb = sizeof(TOKEN_PRIVILEGES);
	bool ret = false;

	do
	{
		if (!LookupPrivilegeValueW(NULL, privilege, &luid))
			break;

		tp.PrivilegeCount = 1;
		tp.Privileges[0].Luid = luid;

		if (enable)
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		else
			tp.Privileges[0].Attributes = 0;

		AdjustTokenPrivileges(token, FALSE, &tp, cb, NULL, NULL);
		if (GetLastError() != ERROR_SUCCESS)
			break;

		ret = true;
	} while (false);

	CloseHandle(token);
	return ret;
}

static HANDLE AdvancedOpenProcess(_In_ DWORD pid)
{
	if (!pid)
		return INVALID_HANDLE_VALUE;

	HANDLE ret{};
	if (!SetPrivilege(L"SeDebugPrivilege", true))
		return NULL;

	ret = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!ret)
		return NULL;

	if (!SetPrivilege(L"SeDebugPrivilege", false))
	{
		CloseHandle(ret);
		return NULL;
	}
	return ret;
}


static bool RtlCreateUserThread(_In_ HANDLE process_handle, _In_ wchar_t *buffer, _In_ SIZE_T buffer_size)
{
	HMODULE ntdll = NULL;
	HMODULE kernel32 = NULL;
	HANDLE thread_handle = NULL;
	CLIENT_ID cid;
	PROC_RtlCreateUserThread RtlCreateUserThread = NULL;
	PTHREAD_START_ROUTINE start_address = NULL;
	bool ret = false;

	__try
	{
		ntdll = LoadLibraryW(L"ntdll.dll");
		if (!ntdll)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to LoadLibrary(ntdll.dll)" << endl;
			__leave;
		}

		RtlCreateUserThread = (PROC_RtlCreateUserThread)GetProcAddress(ntdll, "RtlCreateUserThread");
		if (!RtlCreateUserThread)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to GetProcAddress(RtlCreateUserThread)" << endl;
			__leave;
		}

		kernel32 = LoadLibrary(L"kernel32.dll");
		if (!kernel32)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to LoadLibrary(kernel32.dll)" << endl;
			__leave;
		}

		start_address = (PTHREAD_START_ROUTINE)GetProcAddress(kernel32, "LoadLibraryW");
		if (!start_address)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to GetProcAddress(LoadLibraryW)" << endl;
			__leave;
		}

		NTSTATUS status = RtlCreateUserThread(	process_handle,
												NULL,
												false,
												0,
												0,
												0,
												start_address,
												buffer,
												&thread_handle,
												&cid);
		if (status > 0)
		{
			Logger::Instance() << "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
				<< "Failed to RtlCreateUserThread (status: " << status << ")" << endl;
			__leave;
		}

		status = WaitForSingleObject(thread_handle, INFINITE);
		if (status == WAIT_FAILED)
		{
			Logger::Instance() << "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
				<< "Failed to WaitForSingleObject (status: " << status << ")" << endl;
			__leave;
		}
		ret = true;
	}
	__finally
	{
		if (kernel32)		FreeLibrary(kernel32);
		if (ntdll)			FreeLibrary(ntdll);
		if (thread_handle)	CloseHandle(thread_handle);
	}
	return ret;
}

bool InjectDll(DWORD PID, const wchar_t* dllName)
{
	HANDLE hProcess{};
	wchar_t* buffer = NULL;
	const size_t bufferSize = wcslen(dllName) * sizeof(wchar_t) + 1;
	bool ret = false;

	__try
	{
		hProcess = AdvancedOpenProcess(PID);
		if (!hProcess)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to AdvancedOpenProcess" << endl;
			__leave;
		}

		buffer = (wchar_t*)VirtualAllocEx(hProcess, NULL, bufferSize, MEM_COMMIT, PAGE_READWRITE);
		if (!buffer)
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to VirtualAllocEx" << endl;
			__leave;
		}

		if (!WriteProcessMemory(hProcess, buffer, dllName, bufferSize, NULL))
		{
			Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
								<< "Failed to WriteProcessMemory" << endl;
			__leave;
		}

		ret = RtlCreateUserThread(hProcess, buffer, bufferSize);
	}
	__finally
	{
		if (hProcess)
		{
			if (buffer) VirtualFreeEx(hProcess, buffer, bufferSize, MEM_COMMIT);
			CloseHandle(hProcess);
		}
	}
	return ret;
}
