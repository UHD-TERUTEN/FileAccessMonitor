#include "pch.h"
#include "../src/Logger.h"
#include <string>
#include <regex>
using namespace Log;

TEST(LoggerTest, WriteHelloWorld)
{
#ifndef _DEBUG
  char* msg1              = "hello world!";
  const char* msg2        = msg1;
  char* const msg3        = msg1;
  constexpr char* msg4    = "hello world!";
  std::string msg5        = msg1;
  const std::string msg6  = msg1;

  Logger::Instance() << msg1 << std::endl;
  Logger::Instance() << msg2 << std::endl;
  Logger::Instance() << msg3 << std::endl;
  Logger::Instance() << msg4 << std::endl;
  Logger::Instance() << msg5 << std::endl;
  Logger::Instance() << msg6 << std::endl;

  std::regex timestampLogRegex
  {
    R"#((Sun|Mon|Tue|Wed|Thu|Fri|Sat) )#"
    R"#((Jan|Feb|Mar|Apr|May|Jun|Jul|Aug|Sep|Oct|Nov|Dec) )#"
    R"#((\d{2}) (\d{2}:\d{2}:\d{2}) (\d{4}))#"
    R"#(.*)#"
  };
  
  std::ifstream logfile("log");
  std::string line{};

  while (std::getline(logfile, line))
    EXPECT_TRUE(std::regex_match(line, timestampLogRegex));
#else
    EXPECT_TRUE(true);
#endif
}