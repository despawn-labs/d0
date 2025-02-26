#include "d0/system/memory.h"

#include <Windows.h>

namespace d0 {

D0_API void PageProtection::FromPlatform(const uint32_t in) {
  value = 0;

  switch (in) {
  case PAGE_EXECUTE:
    SetExecute();
    break;
  case PAGE_EXECUTE_READ:
    SetExecute();
    SetRead();
    break;
  case PAGE_EXECUTE_READWRITE:
    SetExecute();
    SetRead();
    SetWrite();
    break;
  case PAGE_READONLY:
    SetRead();
    break;
  case PAGE_READWRITE:
    SetRead();
    SetWrite();
    break;
  default:
    break;
  }
}

D0_API uint32_t PageProtection::ToPlatform() const {
  return GetExecute()
             ? (GetRead()
                    ? (GetWrite() ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ)
                    : PAGE_EXECUTE)
         : GetRead() && GetWrite() ? PAGE_READWRITE
                                   : PAGE_READONLY;
}

D0_API PageState GetPageState(uptr address) {
  MEMORY_BASIC_INFORMATION mbi{};
  VirtualQuery((void *)address, &mbi, sizeof(mbi));

  PageProtection p{};
  p.FromPlatform(mbi.Protect);

  return {
      p,
      mbi.State == MEM_FREE,
  };
}

D0_API PageProtection GetPageProtection(uintptr_t address) {
  return GetPageState(address).protection;
}

D0_API void SetPageProtection(uintptr_t address,
                              const PageProtection &protection,
                              PageProtection &old_protection) {
  uint32_t old_p;
  VirtualProtect(reinterpret_cast<void *>(address), GetPageSize(),
                 protection.ToPlatform(),
                 reinterpret_cast<unsigned long *>(&old_p));
  old_protection.FromPlatform(old_p);
}

D0_API usize GetPageSize() {
  SYSTEM_INFO si{};
  GetSystemInfo(&si);
  return si.dwPageSize;
}

D0_API uptr AllocatePage(const uptr address) {
  return reinterpret_cast<uptr>(
      VirtualAlloc(reinterpret_cast<LPVOID>(address), GetPageSize(),
                   MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
}

D0_API void FreePage(const uptr address) {
  VirtualFree(reinterpret_cast<LPVOID>(address), GetPageSize(), MEM_RELEASE);
}

} // namespace d0