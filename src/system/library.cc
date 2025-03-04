#include "d0/system/library.h"
#include "d0/defs.h"

#ifdef D0_WINDOWS
#include <Windows.h>
#endif

namespace d0 {

Library::Library(LibraryHandle &&handle) { handle_ = handle; }

Library::~Library() {
  if (!handle_)
    return;

#ifdef D0_WINDOWS
  CloseHandle(handle_);
#endif
}

Library::Library(Library &&other) noexcept {
  handle_ = other.handle_;
  other.handle_ = nullptr;
}

} // namespace d0