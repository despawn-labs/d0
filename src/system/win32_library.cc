#include "d0/system/library.h"

#include <Windows.h>

namespace d0 {

void CloseLibrary(LibraryHandle handle) { CloseHandle(handle); }

LibrarySymbol GetLibrarySymbol(LibraryHandle handle, const std::string &name) {
  return GetProcAddress(static_cast<HMODULE>(handle), name.c_str());
}

} // namespace d0