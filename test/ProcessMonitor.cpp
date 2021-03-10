#include "pch.h"
#include "../src/ProcessMonitor.h"
#include "../src/ProcessMonitor.cpp"
#include "../src/EventSink.h"
#include "../src/EventSink.cpp"
using namespace WMIProcess;

#include <future>
#define TEST_TIMEOUT_BEGIN  std::promise<bool> promisedFinished; \
                            auto futureResult = promisedFinished.get_future(); \
                            std::thread([&](std::promise<bool>& finished) {

#define TEST_TIMEOUT_FAIL_END(X)    finished.set_value(true); \
                                    }, std::ref(promisedFinished)).detach(); \
                                    EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout);

#define TEST_TIMEOUT_SUCCESS_END(X) finished.set_value(true); \
                                    }, std::ref(promisedFinished)).detach(); \
                                    EXPECT_FALSE(futureResult.wait_for(std::chrono::milliseconds(X)) != std::future_status::timeout);

#include <iostream>

bool isWindowOpen = true;

TEST(ProcessMonitorTest, RunningTest)
{
  ProcessMonitor monitor{};

  ASSERT_TRUE(SUCCEEDED(monitor.GetStatus()));
  ASSERT_TRUE(monitor.GetErrorMessage().empty());

  TEST_TIMEOUT_BEGIN
      monitor.Run();
  TEST_TIMEOUT_FAIL_END(100)
}

TEST(ProcessMonitorTest, RunningFinished)
{
  ProcessMonitor monitor{};

  ASSERT_TRUE(SUCCEEDED(monitor.GetStatus()));
  ASSERT_TRUE(monitor.GetErrorMessage().empty());

  isWindowOpen = false;
  monitor.Run();
  EXPECT_TRUE(true);
}
