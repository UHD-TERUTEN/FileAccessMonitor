#include "pch.h"
#include "../src/ProcessMonitor.h"
#include "../src/ProcessMonitor.cpp"
#include "../src/EventSink.h"
#include "../src/EventSink.cpp"
using namespace WMIProcess;

#include <future>
#include <thread>

bool isWindowOpen = true;

TEST(ProcessMonitorTest, RunningTest)
{
  ProcessMonitor monitor{};

  ASSERT_TRUE(SUCCEEDED(monitor.GetStatus()));
  ASSERT_TRUE(monitor.GetErrorMessage().empty());

  std::promise<bool> promisedFinished{};
  auto futureResult = promisedFinished.get_future();
  std::thread t([&](std::promise<bool>& finished)
    {
      finished.set_value(false);
      monitor.Run();
      finished.set_value(true);
    },
    std::ref(promisedFinished)
  );
  t.detach();
  if (futureResult.valid())
  EXPECT_NE(futureResult.wait_for(std::chrono::milliseconds(100)), std::future_status::timeout);
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
