#include "ProcessMonitor.h"
#include "EventSink.h"

#include <string>

namespace wql
{
	static constexpr auto notifyConsoleCreation =
		"SELECT * "
		"FROM __InstanceCreationEvent WITHIN 1 "
		"WHERE TargetInstance ISA 'Win32_Process'"
		"AND TargetInstance.Name LIKE '%.exe'";
}

namespace wmi_error_message
{
	static constexpr auto CoInitializeFailed				= "Failed to initialize COM library.";

	static constexpr auto CoInitializeSecurityFailed		= "Failed to initialize security.";

	static constexpr auto CoCreateInstanceFailed			= "Failed to create IWbemLocator object.";

	static constexpr auto ConnectServerFailed				= "Could not connect to WMI server.";

	static constexpr auto CoSetProxyBlanketFailed			= "Could not set proxy blanket.";

	static constexpr auto QueryInterfaceFailed				= "Failed to query interface.";

	static constexpr auto CreateObjectStubFailed			= "Failed to create ObjectStub.";
	
	static constexpr auto ExecNotificationQueryAsyncFailed	= "Failed to ExecNotificationQueryAsync.";
}

namespace WMIProcess
{
	struct ProcessMonitor::Impl
	{
		HRESULT					hres;
		std::string				errorMessage;
		IWbemLocator*			ploc		= nullptr;
		IWbemServices*			psvc		= nullptr;
		IUnsecuredApartment*	punsecApp	= nullptr;
		EventSink*				psink		= new EventSink{};
		IUnknown*				pstubUnk	= nullptr;
		IWbemObjectSink*		pstubSink	= nullptr;
	};
	
	ProcessMonitor::ProcessMonitor()
		: pimpl(new Impl)
	{
		// init COM
		if (FAILED(pimpl->hres = CoInitializeEx(0, COINIT_MULTITHREADED)))
		{
			pimpl->errorMessage = wmi_error_message::CoInitializeFailed;
			return;
		}
	
		// set general COM security levels
		if (FAILED(pimpl->hres = CoInitializeSecurity(	NULL,
														-1,								// COM authentication
														NULL,							// Authentication services
														NULL,							// Reserved
														RPC_C_AUTHN_LEVEL_DEFAULT,		// Default authentication
														RPC_C_IMP_LEVEL_IMPERSONATE,	// Default Impersonation
														NULL,							// Authentication info
														EOAC_NONE,						// Additional capabilities
														NULL)))							// Reserved
		{
			pimpl->errorMessage = wmi_error_message::CoInitializeSecurityFailed;
			return;
		}
	
		// obtain the initial locator to WMI
		if (FAILED(pimpl->hres = CoCreateInstance(	CLSID_WbemLocator,
													0,
													CLSCTX_INPROC_SERVER,
													IID_IWbemLocator,
													(LPVOID*)&pimpl->ploc)))
		{
			pimpl->errorMessage = wmi_error_message::CoCreateInstanceFailed;
			return;
		}
	
		// Connect to WMI through the IWbemLocator::ConnectServer method
		if (FAILED(pimpl->hres = pimpl->ploc->ConnectServer(_bstr_t(R"(ROOT\CIMV2)"),	// object path of WMI namespace
															NULL,			// User name		(NULL is current)
															NULL,			// User password	(NULL is current)
															0,				// Locale			(NULL is current)
															NULL,			// Security flags
															0,				// Authority
															0,				// Context object
															&pimpl->psvc)))	// pointer to IWbemServices proxy
		{
			pimpl->errorMessage = wmi_error_message::ConnectServerFailed;
			return;
		}
	
		// set security levels on the proxy
		if (FAILED(pimpl->hres = CoSetProxyBlanket(	pimpl->psvc,					// Indicates the proxy to set
													RPC_C_AUTHN_WINNT,				// RPC_C_AUTHN_xxx
													RPC_C_AUTHZ_NONE,				// RPC_C_AUTHZ_xxx
													NULL,							// Server principal name
													RPC_C_AUTHN_LEVEL_CALL,			// RPC_C_AUTHN_LEVEL_xxx
													RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx
													NULL,							// client identity
													EOAC_NONE)))					// proxy capabilities
		{
			pimpl->errorMessage = wmi_error_message::CoSetProxyBlanketFailed;
			return;
		}
	
		// receive event notifications
		// use an unsecured apartment for security
		if (FAILED(pimpl->hres = CoCreateInstance(	CLSID_UnsecuredApartment,
													NULL,
													CLSCTX_LOCAL_SERVER,
													IID_IUnsecuredApartment,
													(void**)&pimpl->punsecApp)))
		{
			pimpl->errorMessage = wmi_error_message::CoCreateInstanceFailed;
			return;
		}
	
		pimpl->psink->AddRef();
		if (FAILED(pimpl->hres = pimpl->punsecApp->CreateObjectStub(pimpl->psink, &pimpl->pstubUnk)))
		{
			pimpl->errorMessage = wmi_error_message::CreateObjectStubFailed;
			return;
		}
		if (FAILED(pimpl->hres = pimpl->pstubUnk->QueryInterface(IID_IWbemObjectSink, (void**)&pimpl->pstubSink)))
		{
			pimpl->errorMessage = wmi_error_message::QueryInterfaceFailed;
			return;
		}
	
		// the ExecNotificationQueryAsync method will call
		// the EventQuery::Indicate method when an event occurs
		if (FAILED(pimpl->hres = pimpl->psvc->ExecNotificationQueryAsync(	_bstr_t("WQL"),
																			_bstr_t(wql::notifyConsoleCreation),
																			WBEM_FLAG_SEND_STATUS,
																			NULL,
																			pimpl->pstubSink)))
		{
			pimpl->errorMessage = wmi_error_message::ExecNotificationQueryAsyncFailed;
			return;
		}
	}
	
	ProcessMonitor::~ProcessMonitor()
	{
		if (SUCCEEDED(pimpl->hres))
			pimpl->hres = pimpl->psvc->CancelAsyncCall(pimpl->pstubSink);
	
		if (pimpl->psvc)		pimpl->psvc->Release();
		if (pimpl->ploc)		pimpl->ploc->Release();
		if (pimpl->punsecApp)	pimpl->punsecApp->Release();
		if (pimpl->pstubUnk)	pimpl->pstubUnk->Release();
		if (pimpl->psink)		pimpl->psink->Release();
		if (pimpl->pstubSink)	pimpl->pstubSink->Release();
	
		if (pimpl->errorMessage != wmi_error_message::CoInitializeFailed)
			CoUninitialize();
	}
	
	long ProcessMonitor::GetStatus() const noexcept
	{
		return (pimpl->hres);
	}
	
	std::string_view ProcessMonitor::GetErrorMessage() const noexcept
	{
		return (pimpl->errorMessage);
	}
	
	void ProcessMonitor::Run() const noexcept
	{
		while (isWindowOpen)
			;
	}
}
