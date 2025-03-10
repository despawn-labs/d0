#include "d0/system/library.h"

#include <Windows.h>

namespace d0 {

D0_API
result<LibraryHandle, LibraryError> OpenLibrary(const std::string &path) {
  SetLastError(ERROR_SUCCESS);
  const auto handle = LoadLibraryA(path.c_str());
  if (handle == nullptr || GetLastError() != ERROR_SUCCESS)
    return fail(LibraryError::kInvalidLibrary);
  return handle;
}

D0_API
void CloseLibrary(LibraryHandle handle) { CloseHandle(handle); }

D0_API
result<LibrarySymbol, LibraryError> GetLibrarySymbol(LibraryHandle handle,
                                                     const std::string &name) {
  SetLastError(ERROR_SUCCESS);
  const auto symbol =
      GetProcAddress(static_cast<HMODULE>(handle), name.c_str());
  if (symbol == nullptr || GetLastError() != ERROR_SUCCESS)
    return fail(LibraryError::kInvalidSymbol);
  return symbol;
}

} // namespace d0