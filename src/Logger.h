#pragma once
#include <fstream>
#include <string_view>
#include <chrono>
#include <cstring>
#include <ctime>

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

		std::ofstream& operator<<(std::string_view message)
		{
			file << GetTimestamp() << '\t' << message;
			return file;
		}

	private:
		Logger()
			: file("log", std::ios::app)
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

		std::ofstream file;
	};
}