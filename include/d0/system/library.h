#pragma once

#include "d0/misc/error.h"
#include "d0/misc/result.h"

#include <string>

namespace d0 {

using LibraryHandle = void *;
using LibrarySymbol = void *;

class Library;
class LibraryError;

/// Provides the class-style API for interacting with system libraries.
class Library {
public:
  explicit Library(LibraryHandle &&handle);
  ~Library();

  Library(const Library &other) = delete;

  Library(Library &&other) noexcept;

  /*
  template <typename T>
  Result<T, LibraryError> GetSymbol(const std::string &name) {
    return Ok<T, LibraryError>(reinterpret_cast<T>(GetSymbolRaw(name)));
  }

  [[nodiscard]] Result<LibrarySymbol, LibraryError>
  GetSymbolRaw(const std::string &name) const;
  */

private:
  LibraryHandle handle_;
};

class LibraryError final : public Error {
public:
  enum class Kind {
    Open = 0,
    Symbol = 1,
  };

  const char *kTranslations[2] = {
      "Failed to open library.", "Failed to get symbol from library."};

public:
  explicit LibraryError(Kind kind);
  ~LibraryError() override;

  [[nodiscard]] const std::string &GetMessage() const override;

private:
  Kind kind_;
};

} // namespace d0