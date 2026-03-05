#ifndef YOONCAMERA_CAMERA_HPP_
#define YOONCAMERA_CAMERA_HPP_

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "yoonvision/image.hpp"
#include "yooncamera/camera_context_manager.hpp"
#include "yooncamera/camera_stream.hpp"

namespace yoonvision::camera {

struct CameraState {
  enum class Status { kUnknown = 0, kSucceeded, kFailed };

  Status initialize = Status::kUnknown;
  Status open_camera = Status::kUnknown;
  Status stream_on = Status::kUnknown;
  Status stream_off = Status::kUnknown;
  Status capture = Status::kUnknown;
  Status close_camera = Status::kUnknown;
  Status finalize = Status::kUnknown;

  struct Monitor {
    std::size_t success_frame_counter = 0;
    std::size_t failed_frame_counter = 0;
  } monitor;
};

struct CameraParameter {
  std::string name;
  std::string device_path;
  std::string serial_number;
  uint16_t timeout_ms = 1000;

  struct Condition {
    bool use_flip = false;
    bool use_align = false;
    bool trace_singleshot = false;
  } condition;
};

class IFrameSubscriber {
 public:
  using Ptr = std::shared_ptr<IFrameSubscriber>;

  virtual ~IFrameSubscriber() = default;
  virtual void OnFrame(const Image::Ptr& frame) = 0;
};

class ICamera {
 public:
  using Ptr = std::shared_ptr<ICamera>;

  virtual ~ICamera() = default;

  virtual CameraState::Status Capture() = 0;
  virtual CameraState::Status StreamOn() = 0;
  virtual void StreamOff() = 0;

  virtual void SetupParameter(std::shared_ptr<CameraParameter> param) = 0;

  virtual Image::Ptr GetCaptureFrame() = 0;

  [[nodiscard]] virtual bool IsFinalized() const = 0;
  [[nodiscard]] virtual bool IsInitialized() const = 0;

  [[nodiscard]] virtual CameraState GetCameraState() const = 0;

  virtual void Subscribe(IFrameSubscriber::Ptr subscriber) = 0;
  virtual void Unsubscribe(IFrameSubscriber::Ptr subscriber) = 0;

  virtual CameraState::Status Open() = 0;
  virtual void Close() = 0;

  virtual CameraState::Status Initialize() = 0;
  virtual void Finalize() = 0;
};

}  // namespace yoonvision::camera

#endif  // YOONCAMERA_CAMERA_HPP_
