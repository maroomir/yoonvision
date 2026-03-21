#ifndef YOONVISION_LOG_HPP_
#define YOONVISION_LOG_HPP_

#include <time.h>

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

#include "build_mode.hpp"

namespace yoonvision::log {

inline std::string CurrentTimestamp() {
  using namespace std::chrono;
  const auto now = system_clock::now();
  const auto now_time_t = system_clock::to_time_t(now);
  const auto millis =
      duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

  struct tm local_tm {};
#if defined(_WIN32)
  localtime_s(&local_tm, &now_time_t);
#else
  localtime_r(&now_time_t, &local_tm);
#endif

  std::ostringstream oss;
  oss << std::setfill('0') << std::setw(2) << (local_tm.tm_mon + 1) << "-"
      << std::setw(2) << local_tm.tm_mday << " " << std::setw(2)
      << local_tm.tm_hour << ":" << std::setw(2) << local_tm.tm_min << ":"
      << std::setw(2) << local_tm.tm_sec << "." << std::setw(3)
      << millis.count();
  return oss.str();
}

inline const char* BaseName(const char* path) {
  if (path == nullptr) {
    return "";
  }
  const char* slash = path;
  for (const char* p = path; *p != '\0'; ++p) {
    if (*p == '/' || *p == '\\') {
      slash = p + 1;
    }
  }
  return slash;
}

inline const char* SafeFunctionName(const char* function_name) {
  return function_name != nullptr ? function_name : "unknown";
}

inline void PrintLogMessage(const char* level, const char* file_name, int line,
                            const char* function_name, const char* fmt) {
  std::printf("%s %s: %s:%-3d %-20s: %s\n", CurrentTimestamp().c_str(), level,
              file_name, line, function_name, fmt != nullptr ? fmt : "");
}

template <typename... Args>
inline void PrintLogMessage(const char* level, const char* file_name, int line,
                            const char* function_name, const char* fmt,
                            Args&&... args) {
  std::printf("%s %s: %s:%-3d %-20s: ", CurrentTimestamp().c_str(), level,
              file_name, line, function_name);
  std::printf(fmt != nullptr ? fmt : "", std::forward<Args>(args)...);
  std::printf("\n");
}

}  // namespace yoonvision::log

#define LOG_INFO(...)                                                     \
  do {                                                                    \
    ::yoonvision::log::PrintLogMessage(                                   \
        "I", ::yoonvision::log::BaseName(__FILE__), __LINE__,            \
        ::yoonvision::log::SafeFunctionName(__func__), __VA_ARGS__);      \
  } while (0)

#define LOG_WARN(...)                                                     \
  do {                                                                    \
    ::yoonvision::log::PrintLogMessage(                                   \
        "W", ::yoonvision::log::BaseName(__FILE__), __LINE__,            \
        ::yoonvision::log::SafeFunctionName(__func__), __VA_ARGS__);      \
  } while (0)

#define LOG_ERROR(...)                                                    \
  do {                                                                    \
    ::yoonvision::log::PrintLogMessage(                                   \
        "E", ::yoonvision::log::BaseName(__FILE__), __LINE__,            \
        ::yoonvision::log::SafeFunctionName(__func__), __VA_ARGS__);      \
  } while (0)

#define LOG_DEBUG(...)                                                    \
  do {                                                                    \
    ::yoonvision::log::PrintLogMessage(                                   \
        "D", ::yoonvision::log::BaseName(__FILE__), __LINE__,            \
        ::yoonvision::log::SafeFunctionName(__func__), __VA_ARGS__);      \
  } while (0)

#define LOG_INFO2(...)                                      \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_INFO(__VA_ARGS__);                                \
    }                                                       \
  } while (0)

#define LOG_WARN2(...)                                      \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_WARN(__VA_ARGS__);                                \
    }                                                       \
  } while (0)

#define LOG_ERROR2(...)                                     \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_ERROR(__VA_ARGS__);                               \
    }                                                       \
  } while (0)

#define LOG_DEBUG2(...)                                     \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_DEBUG(__VA_ARGS__);                               \
    }                                                       \
  } while (0)

#endif  // YOONVISION_LOG_HPP_
