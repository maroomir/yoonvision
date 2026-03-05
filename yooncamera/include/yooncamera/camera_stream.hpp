#ifndef YOONCAMERA_CAMERA_STREAM_HPP_
#define YOONCAMERA_CAMERA_STREAM_HPP_

#include <cstdint>
#include <string>

namespace yoonvision::camera {

class CameraStream {
 public:
  enum class Type : int {
    kNone = -1,
    kColor = 0,
    kDepth,
    kInfrared,
    kOutput,
    kMaxType,
  };

  CameraStream();
  CameraStream(Type type, uint16_t width, uint16_t height,
               const std::string& format, uint16_t frame_rate,
               uint16_t buffer_count = 10, bool enabled = true);
  CameraStream(const std::string& type, uint16_t width, uint16_t height,
               const std::string& format, uint16_t frame_rate,
               uint16_t buffer_count = 10, bool enabled = true);

  [[nodiscard]] Type GetStreamType() const;
  [[nodiscard]] uint16_t GetWidth() const;
  [[nodiscard]] uint16_t GetHeight() const;
  [[nodiscard]] uint16_t GetFrameRate() const;
  [[nodiscard]] uint16_t GetBufferCount() const;
  [[nodiscard]] const std::string& GetFormat() const;
  [[nodiscard]] bool GetEnabled() const;

  static Type GetStreamTypeFromString(const std::string& type);
  static std::string GetStreamTypeToString(Type type);
  [[nodiscard]] std::string GetStreamTypeToString() const;

 private:
  Type type_;
  uint16_t width_;
  uint16_t height_;
  std::string format_;
  uint16_t frame_rate_;
  uint16_t buffer_count_;
  bool enabled_;
};

}  // namespace yoonvision::camera

#endif  // YOONCAMERA_CAMERA_STREAM_HPP_
