#include "d0/platform/memory.h"

#include <Windows.h>

namespace d0 {

size_t GetPageSize() {
  SYSTEM_INFO si{};
  GetSystemInfo(&si);
  return static_cast<size_t>(si.dwPageSize);
}

}