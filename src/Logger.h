#pragma once
#include <string_view>
#include <chrono>
#include <cstring>
#include <ctime>

#ifdef _DEBUG
#include <iostream>
#define out			std::cout
#define outstream	std::ostream
#else
#include <fstream>
#define outstream	std::ofstream
#endif

namespace Log
{
	class Logger
	{
	public:
		static Logger& Instance()
		{
			static Logger* logger = new Logger{};
			return *logger;
		}

		outstream& operator<<(std::string_view message)
		{
			out << GetTimestamp() << '\t' << message;
			return out;
		}

	private:
		Logger()
#ifndef _DEBUG
			: out("log", std::ios::app)
#endif
		{
		}

		[[nodiscard]] char* GetTimestamp() const
		{
			static char timeBuf[BUFSIZ];
			auto now = std::chrono::system_clock::now();
			auto now_time_t = std::chrono::system_clock::to_time_t(now);

			memset(timeBuf, 0, BUFSIZ);
			ctime_s(timeBuf, BUFSIZ, &now_time_t);
			timeBuf[strlen(timeBuf) - 1] = 0;
			return timeBuf;
		}

#ifndef _DEBUG
		std::ofstream out;
#endif
	};
}