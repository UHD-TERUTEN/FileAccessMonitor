#pragma once
#define _HAS_STD_BYTE 0

#include <string_view>
#include <memory>

extern bool isWindowOpen;

namespace WMIProcess
{
	class ProcessMonitor
	{
	public:
		ProcessMonitor();
		~ProcessMonitor();

		long GetStatus() const noexcept;
		std::string_view GetErrorMessage() const noexcept;

		void Run() const noexcept;

	private:
		struct Impl;
		std::unique_ptr<Impl> pimpl;
	};
}
