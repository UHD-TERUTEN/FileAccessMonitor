#pragma once
#define _WIN32_DCOM
#define _HAS_STD_BYTE 0

#include <comdef.h>
#include <WbemIdl.h>

#pragma comment(lib, "wbemuuid.lib")

namespace WMIProcess
{
	class EventSink : public IWbemObjectSink
	{
		LONG ref;
		bool isDone;
		CRITICAL_SECTION threadLock;
	
	public:
		EventSink();
		~EventSink();
	
		virtual ULONG STDMETHODCALLTYPE AddRef();
	
		virtual ULONG STDMETHODCALLTYPE Release();
	
		virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
	
		virtual HRESULT STDMETHODCALLTYPE Indicate(	LONG	nObjects,
													IWbemClassObject __RPC_FAR* __RPC_FAR* objArray);
	
		virtual HRESULT STDMETHODCALLTYPE SetStatus(LONG	flags,
													HRESULT	hres,
													[[maybe_unused]] BSTR strParam,
													[[maybe_unused]] IWbemClassObject __RPC_FAR* objParam);
	
		bool IsDone();
	};
}
