#include "pch.h"
#include "../src/util.h"
#include "../src/util.cpp"

TEST(UtilTest, LoadRtlCreateUserThreadFunction)
{
  EXPECT_TRUE(LoadDllFunctions());
  EXPECT_FALSE(ntdll == NULL);
  EXPECT_FALSE(kernel32 == NULL);
  EXPECT_FALSE(rtlCreateUserThread == NULL);
  EXPECT_FALSE(start_address == NULL);
}

TEST(UtilTest, ConvertToUtt8String)
{
  constexpr wchar_t* hello = L"\uC548\uB155";
  
  EXPECT_STREQ(ToUtf8String(hello, std::wcslen(hello)).c_str(), "\uC548\uB155");
}