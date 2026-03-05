#include "yooncamera/camera_stream.hpp"

#include <algorithm>
#include <cctype>

namespace yoonvision::camera {

namespace {

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return value;
}

}  // namespace

CameraStream::CameraStream()
    : type_(Type::kNone),
      width_(0),
      height_(0),
      format_(""),
      frame_rate_(0),
      buffer_count_(0),
      enabled_(false) {}

CameraStream::CameraStream(Type type, uint16_t width, uint16_t height,
                           const std::string& format, uint16_t frame_rate,
                           uint16_t buffer_count, bool enabled)
    : type_(type),
      width_(width),
      height_(height),
      format_(format),
      frame_rate_(frame_rate),
      buffer_count_(buffer_count),
      enabled_(enabled) {}

CameraStream::CameraStream(const std::string& type, uint16_t width,
                           uint16_t height, const std::string& format,
                           uint16_t frame_rate, uint16_t buffer_count,
                           bool enabled)
    : CameraStream(GetStreamTypeFromString(type), width, height, format,
                   frame_rate, buffer_count, enabled) {}

CameraStream::Type CameraStream::GetStreamType() const { return type_; }
uint16_t CameraStream::GetWidth() const { return width_; }
uint16_t CameraStream::GetHeight() const { return height_; }
uint16_t CameraStream::GetFrameRate() const { return frame_rate_; }
uint16_t CameraStream::GetBufferCount() const { return buffer_count_; }
const std::string& CameraStream::GetFormat() const { return format_; }
bool CameraStream::GetEnabled() const { return enabled_; }

CameraStream::Type CameraStream::GetStreamTypeFromString(const std::string& type) {
  const std::string lower = ToLower(type);
  if (lower == "color" || lower == "rgb") {
    return Type::kColor;
  }
  if (lower == "depth") {
    return Type::kDepth;
  }
  if (lower == "infrared" || lower == "ir") {
    return Type::kInfrared;
  }
  if (lower == "output") {
    return Type::kOutput;
  }
  return Type::kNone;
}

std::string CameraStream::GetStreamTypeToString(Type type) {
  switch (type) {
    case Type::kColor:
      return "color";
    case Type::kDepth:
      return "depth";
    case Type::kInfrared:
      return "infrared";
    case Type::kOutput:
      return "output";
    case Type::kNone:
    case Type::kMaxType:
    default:
      return "none";
  }
}

std::string CameraStream::GetStreamTypeToString() const {
  return GetStreamTypeToString(type_);
}

}  // namespace yoonvision::camera
