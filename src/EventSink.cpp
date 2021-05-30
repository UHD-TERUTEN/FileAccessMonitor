#include "EventSink.h"
#include "util.h"
#include "Logger.h"
using namespace Log;

#include <iomanip>
using namespace std;

#ifdef _WIN64
constexpr auto DLL_NAME = L"DetoursReject64.dll";
#else
constexpr auto DLL_NAME = L"DetoursReject32.dll";
#endif

namespace WMIProcess
{
	EventSink::EventSink()
		: ref(0)
		, isDone(false)
	{
		InitializeCriticalSection(&threadLock);
	}
	
	EventSink::~EventSink()
	{
		isDone = true;
		DeleteCriticalSection(&threadLock);
	}
	
	ULONG EventSink::AddRef()
	{
		return InterlockedIncrement(&ref);
	}
	
	ULONG EventSink::Release()
	{
		LONG lref = InterlockedDecrement(&ref);
		if (lref == 0)
			delete this;
		return lref;
	}
	
	HRESULT EventSink::QueryInterface(REFIID riid, void** ppv)
	{
		if (riid == IID_IUnknown || riid == IID_IWbemObjectSink)
		{
			*ppv = (IWbemObjectSink*)this;
			AddRef();
			return WBEM_S_NO_ERROR;
		}
		return E_NOINTERFACE;
	}
	
	// https://stackoverflow.com/questions/28897897/c-monitor-process-creation-and-termination-in-windows
	HRESULT EventSink::Indicate(LONG nObjects, IWbemClassObject __RPC_FAR* __RPC_FAR* objArray)
	{
	    HRESULT hr = S_OK;
		_variant_t vtProp{};
	
	    for (int i = 0; i < nObjects; i++)
	    {
			hr = objArray[i]->Get(_bstr_t(L"TargetInstance"), 0, &vtProp, 0, 0);
			if (SUCCEEDED(hr))
			{
				IUnknown* str = vtProp;
				hr = str->QueryInterface(IID_IWbemClassObject, reinterpret_cast<void**>(&objArray[i]));
				if (SUCCEEDED(hr))
				{
					_variant_t cn;
#ifdef _DEBUG
					hr = objArray[i]->Get(L"Name", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
						Logger::Instance() << "Name : " << ToUtf8String(cn.bstrVal, SysStringLen(cn.bstrVal)) << endl;
					}
					VariantClear(&cn);
#endif
					hr = objArray[i]->Get(L"ProcessId", 0, &cn, NULL, NULL);
					if (SUCCEEDED(hr))
					{
#ifdef _DEBUG
						Logger::Instance()	<< "ProcessId : " << dec << cn.uintVal << endl;
						if (auto ret = InjectDll(cn.uintVal, DLL_NAME))
							Logger::Instance() << "Injection Succeeded : " << boolalpha << ret << endl;
#else
						InjectDll(cn.uintVal, DLL_NAME);
#endif
					}
					VariantClear(&cn);
				}
			}
	        VariantClear(&vtProp);
	    }
		return WBEM_S_NO_ERROR;
	}
	
	HRESULT EventSink::SetStatus(LONG flags, HRESULT hres, BSTR, IWbemClassObject*)
	{
		if (flags == WBEM_STATUS_COMPLETE)
		{
			Logger::Instance()	<< "Call complete. hResult = 0x"
								<< setw(8) << setfill('0') << hex << hres << endl;
			EnterCriticalSection(&threadLock);
			isDone = true;
			LeaveCriticalSection(&threadLock);
		}
		else if (flags == WBEM_STATUS_PROGRESS)
		{
			Logger::Instance() << "Call in progress." << endl;
		}
		return WBEM_S_NO_ERROR;
	}
	
	bool EventSink::IsDone()
	{
		bool done = true;
		EnterCriticalSection(&threadLock);
		isDone = done;
		LeaveCriticalSection(&threadLock);
		return isDone;
	}
}
