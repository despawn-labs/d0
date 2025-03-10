///
/// Provides a cross-platform interface for working with dynamic libraries.
///
/// On Windows these are referred to as *dynamic link libraries*, while on
/// Linux these are referred to as *shared objects*.
///
/// They effectively provide the same functionality which is dynamically
/// linkable code. This is often used in software where the code base contains
/// many independent systems split into libraries orchestrated by either the
/// executable or yet another library.
///
/// The interfaces provided come in two flavors: C-style and C++-style.
/// The C-style interface uses global functions to work with an opaque handle
/// type. The C++-style interface uses a class to represent a *library handle*.
///

#pragma once

#include "d0/defs.h"

#include <result.hpp>
#include <string>

namespace d0 {

/// Represents an error which a library can throw.
enum class D0_API LibraryError {
  kInvalidLibrary = 0,
  kInvalidSymbol = 1,
};

/// Opaque type used to represent a library handle.
using LibraryHandle = void *;

/// Opaque type used to represent a library symbol. This is effectively the
/// address of the symbol.
using LibrarySymbol = void *;

/// Opens or resolves a [`LibraryHandle`] from the given `path`.
/// @param path The path of the library to open or resolve.
/// @return A result containing a [`LibraryHandle`] or a [`LibraryError`].
D0_API result<LibraryHandle, LibraryError> OpenLibrary(const std::string &path);

/// Closes a [`LibraryHandle`] instance.
/// @param handle The handle to close.
D0_API void CloseLibrary(LibraryHandle handle);

/// Resolves a [`LibrarySymbol`] from the given `handle` and `name`.
/// @param handle The handle of the library to resolve the symbol from.
/// @param name The name of the symbol to resolve.
/// @return A result containing a [`LibrarySymbol`] or a [`LibraryError`].
D0_API result<LibrarySymbol, LibraryError>
GetLibrarySymbol(LibraryHandle handle, const std::string &name);

} // namespace d0