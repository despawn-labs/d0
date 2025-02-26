#include "d0/misc/logger.h"

#include <print>

namespace d0 {

const char *LoggerLevelToString(const LoggerLevel &level) {
  switch (level) {
  case LoggerLevel::kTrace:
    return "trace";
  case LoggerLevel::kDebug:
    return "debug";
  case LoggerLevel::kInfo:
    return "info";
  case LoggerLevel::kWarn:
    return "warn";
  case LoggerLevel::kError:
    return "error";
  }

  return "n/a";
}

Logger::Logger(const LoggerOptions &options)
    : context_(options.context), level_(options.level) {}

Logger::~Logger() = default;

void Logger::Send(const LoggerMessage &message) const {
  if (message.level < level_)
    return;

  std::println("{:%Y-%m-%d %H:%M:%S} | [{}] - {}: {}", message.timestamp,
               context_, LoggerLevelToString(message.level), message.content);
}

void Logger::Write(const LoggerLevel &level, const std::string &content) const {
  Send(LoggerMessage{
      .level = level,
      .content = content,
      .timestamp = std::chrono::system_clock::now(),
  });
}

} // namespace d0