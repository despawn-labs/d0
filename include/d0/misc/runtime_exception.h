#pragma once

#include <exception>
#include <format>
#include <string>

namespace d0 {

/// Used as the primary exception type by `d0`.
/// "Runtime" implies that the exception source was a programming error which
/// still compiled.
class RuntimeException final : public std::exception {
public:
  /// RuntimeException constructor.
  /// @param message The message of the exception.
  explicit RuntimeException(std::string message);

  /// RuntimeException constructor.
  /// @param format The format of the exception.
  /// @param args The arguments to use when formatting the exception.
  /// @tparam Args The argument types.
  template <typename... Args>
  explicit RuntimeException(const std::string &format, Args &&...args)
      : RuntimeException(std::vformat(format, std::make_format_args(args...))) {
  }

  /// RuntimeException destructor.
  ~RuntimeException() override;

  /// Returns the exception's message.
  /// @return The exception's message.
  [[nodiscard]] const char *what() const override;

private:
  std::string message_;
};

} // namespace d0