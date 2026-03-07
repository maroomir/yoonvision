#ifndef YOONVISION_HTTP_RESOURCE_UTILS_HPP_
#define YOONVISION_HTTP_RESOURCE_UTILS_HPP_

#include <fstream>
#include <map>
#include <sstream>
#include <string>

#include "log.hpp"

namespace yoonvision {
namespace http {
namespace resource_utils {

inline std::string LoadHTMLFile(const std::string& filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) {
    LOG_ERROR("Failed to open resource file: %s", filepath.c_str());
    return "";
  }
  std::ostringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

inline std::string AdjustTemplateToHTML(
    const std::string& template_str,
    const std::map<std::string, std::string>& vars) {
  std::string result = template_str;

  for (const auto& [key, value] : vars) {
    std::string placeholder = "${" + key + "}";
    size_t pos = 0;

    while ((pos = result.find(placeholder, pos)) != std::string::npos) {
      result.replace(pos, placeholder.length(), value);
      pos += value.length();
    }
  }

  return result;
}

}  // namespace resource_utils
}  // namespace http
}  // namespace yoonvision

#endif  // YOONVISION_HTTP_RESOURCE_UTILS_HPP_
