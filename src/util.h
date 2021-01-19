#pragma once
#define _HAS_STD_BYTE 0

#include <Windows.h>
#include <tchar.h>

#include <string>

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

bool LoadDllFunctions();

bool InjectDll(DWORD PID, const wchar_t* dllName);

std::string ToUtf8String(const wchar_t* unicode, const size_t unicode_size);
