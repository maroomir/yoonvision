#ifndef YOONVISION_LOG_HPP_
#define YOONVISION_LOG_HPP_

#include <time.h>

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <string>

#include "build_mode.hpp"

namespace yoonvision::log_internal {

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

}  // namespace yoonvision::log_internal

#define LOG_INFO(fmt, ...)                                                \
  do {                                                                    \
    std::printf("%s I: %s:%-3d %-20s: " fmt "\n",                         \
                ::yoonvision::log_internal::CurrentTimestamp().c_str(),   \
                ::yoonvision::log_internal::BaseName(__FILE__), __LINE__, \
                ::yoonvision::log_internal::SafeFunctionName(__func__),   \
                ##__VA_ARGS__);                                           \
  } while (0)

#define LOG_WARN(fmt, ...)                                                \
  do {                                                                    \
    std::printf("%s W: %s:%-3d %-20s: " fmt "\n",                         \
                ::yoonvision::log_internal::CurrentTimestamp().c_str(),   \
                ::yoonvision::log_internal::BaseName(__FILE__), __LINE__, \
                ::yoonvision::log_internal::SafeFunctionName(__func__),   \
                ##__VA_ARGS__);                                           \
  } while (0)

#define LOG_ERROR(fmt, ...)                                               \
  do {                                                                    \
    std::printf("%s E: %s:%-3d %-20s: " fmt "\n",                         \
                ::yoonvision::log_internal::CurrentTimestamp().c_str(),   \
                ::yoonvision::log_internal::BaseName(__FILE__), __LINE__, \
                ::yoonvision::log_internal::SafeFunctionName(__func__),   \
                ##__VA_ARGS__);                                           \
  } while (0)

#define LOG_DEBUG(fmt, ...)                                               \
  do {                                                                    \
    std::printf("%s D: %s:%-3d %-20s: " fmt "\n",                         \
                ::yoonvision::log_internal::CurrentTimestamp().c_str(),   \
                ::yoonvision::log_internal::BaseName(__FILE__), __LINE__, \
                ::yoonvision::log_internal::SafeFunctionName(__func__),   \
                ##__VA_ARGS__);                                           \
  } while (0)

#define LOG_INFO2(fmt, ...)                                 \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_INFO(fmt, ##__VA_ARGS__);                         \
    }                                                       \
  } while (0)

#define LOG_WARN2(fmt, ...)                                 \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_WARN(fmt, ##__VA_ARGS__);                         \
    }                                                       \
  } while (0)

#define LOG_ERROR2(fmt, ...)                                \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_ERROR(fmt, ##__VA_ARGS__);                        \
    }                                                       \
  } while (0)

#define LOG_DEBUG2(fmt, ...)                                \
  do {                                                      \
    if constexpr (::yoonvision::BuildMode::IsDebugMode()) { \
      LOG_DEBUG(fmt, ##__VA_ARGS__);                        \
    }                                                       \
  } while (0)

#endif  // YOONVISION_LOG_HPP_
