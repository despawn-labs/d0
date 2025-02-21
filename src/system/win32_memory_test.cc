#include <gtest/gtest.h>

#include <d0/system/memory.h>

#include <Windows.h>

using namespace d0;

TEST(Win32Memory, GetPageProtection) {
  auto page = VirtualAlloc(nullptr, GetPageSize(),
                           MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
}