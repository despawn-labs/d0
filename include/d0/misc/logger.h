#pragma once

#include <chrono>
#include <format>
#include <string>

namespace d0 {

enum class LoggerLevel {
  kTrace = 0,
  kDebug = 1,
  kInfo = 2,
  kWarn = 3,
  kError = 4,
};

const char *LoggerLevelToString(const LoggerLevel &level);

struct LoggerMessage;

struct LoggerOptions;

class Logger {
public:
  explicit Logger(const LoggerOptions &options);
  ~Logger();

  void Send(const LoggerMessage &message) const;

  void Write(const LoggerLevel &level, const std::string &content) const;

  void Trace(const std::string &content) const {
    Write(LoggerLevel::kTrace, content);
  }

  template <typename... Args>
  void TraceF(const std::format_string<Args...> &format, Args &&...args) {
    Trace(std::vformat(format, std::make_format_args(args)));
  }

  void Debug(const std::string &content) const {
    Write(LoggerLevel::kDebug, content);
  }

  template <typename... Args>
  void DebugF(const std::format_string<Args...> &format, Args &&...args) {
    Debug(std::vformat(format, std::make_format_args(args)));
  }

  void Info(const std::string &content) const {
    Write(LoggerLevel::kInfo, content);
  }

  template <typename... Args>
  void InfoF(const std::format_string<Args...> &format, Args &&...args) {
    Info(std::vformat(format, std::make_format_args(args)));
  }

  void Warn(const std::string &content) const {
    Write(LoggerLevel::kWarn, content);
  }

  template <typename... Args>
  void WarnF(const std::format_string<Args...> &format, Args &&...args) {
    Warn(std::vformat(format, std::make_format_args(args)));
  }

  void Error(const std::string &content) const {
    Write(LoggerLevel::kError, content);
  }

  template <typename... Args>
  void ErrorF(const std::format_string<Args...> &format, Args &&...args) {
    Error(std::vformat(format, std::make_format_args(args)));
  }

private:
  std::string context_;
  LoggerLevel level_;
};

struct LoggerMessage {
  LoggerLevel level;
  std::string content;
  std::chrono::time_point<std::chrono::system_clock> timestamp;
};

struct LoggerOptions {
  std::string context;
  LoggerLevel level;
};

} // namespace d0