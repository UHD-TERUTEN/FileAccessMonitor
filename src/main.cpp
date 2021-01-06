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

int main()
{
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);

	ProcessMonitor monitor{};

	if (FAILED(monitor.GetStatus()))
	{
		Logger::Instance()	<< "[0x" << setw(8) << setfill('0') << hex << monitor.GetStatus() << "] "
							<< monitor.GetErrorMessage() << endl;
		return 1;
	}
	monitor.Run();
	return 0;
}