#include "util.h"
#include "ProcessMonitor.h"
using namespace WMIProcess;

#include "Logger.h"
using namespace Log;

#include <iomanip>
using namespace std;

#include <Windows.h>

bool isWindowOpen = true;

static BOOL WINAPI CtrlHandler(DWORD ctrlType)
{
	switch (ctrlType)
	{
	case CTRL_CLOSE_EVENT:		[[fallthrough]];
	case CTRL_LOGOFF_EVENT:		[[fallthrough]];
	case CTRL_SHUTDOWN_EVENT:	isWindowOpen = false;
	}
	return FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpcmdLine, int nCmdShow)
{
	// Initialize application
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE))
	{
		Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << GetLastError() << "] "
							<< "SetConsoleCtrlHandler failed." << endl;
		return WM_QUIT;
	}
	Logger::Instance() << "SetConsoleCtrlHandler succeeded." << endl;

	if (!LoadDllFunctions())
	{
		Logger::Instance() << "LoadDllFunctions failed." << endl;
		return WM_QUIT;
	}
	Logger::Instance() << "LoadDllFunctions succeeded." << endl;

	// Run the ProcessMonitor
	ProcessMonitor monitor{};

	if (FAILED(monitor.GetStatus()))
	{
		Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << monitor.GetStatus() << "] "
							<< monitor.GetErrorMessage() << endl;
		return WM_QUIT;
	}
	monitor.Run();
	return WM_QUIT;
}