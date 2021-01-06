#pragma once
#define _HAS_STD_BYTE 0

#include <Windows.h>
#include <tchar.h>

typedef struct _CLIENT_ID
{
	PVOID UniqueProcess;
	PVOID UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef NTSTATUS(WINAPI* PROC_RtlCreateUserThread)
(
	HANDLE ProcessHandle,
	PSECURITY_DESCRIPTOR SecurityDescriptor,
	BOOLEAN CreateSuspended,
	ULONG StackZeroBits,
	SIZE_T StackReserve,
	SIZE_T StackCommit,
	PTHREAD_START_ROUTINE StartAddress,
	PVOID Parameter,
	PHANDLE ThreadHandle,
	PCLIENT_ID ClientId
);

bool InjectDll(DWORD PID, const wchar_t* dllName);
