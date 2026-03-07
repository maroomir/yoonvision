#ifndef YOONCAMERA_AVFCAM_AVF_CAMERA_HPP_
#define YOONCAMERA_AVFCAM_AVF_CAMERA_HPP_

#include <memory>
#include <mutex>
#include <vector>

#include "yooncamera/camera.hpp"

struct AvfCameraImpl;

namespace yoonvision::camera::avfcam {

class AvfCamera : public ICamera {
 public:
  explicit AvfCamera(std::shared_ptr<CameraParameter> param);
  ~AvfCamera() override;

  CameraState::Status Initialize() override;
  void Finalize() override;

  CameraState::Status Open() override;
  void Close() override;

  CameraState::Status StreamOn() override;
  void StreamOff() override;
  CameraState::Status Capture() override;

  void SetupParameter(std::shared_ptr<CameraParameter> param) override;

  [[nodiscard]] Image::Ptr GetCaptureFrame() override;
  [[nodiscard]] bool IsInitialized() const override;
  [[nodiscard]] bool IsFinalized() const override;
  [[nodiscard]] CameraState GetCameraState() const override;

  void Subscribe(IFrameSubscriber::Ptr subscriber) override;
  void Unsubscribe(IFrameSubscriber::Ptr subscriber) override;

  void OnSampleBuffer(void* sample_buffer_ref);

 private:
  std::shared_ptr<CameraParameter> param_;
  CameraState state_;

  mutable std::mutex frame_mutex_;
  Image::Ptr latest_frame_;

  std::mutex subscriber_mutex_;
  std::vector<IFrameSubscriber::Ptr> subscribers_;

  AvfCameraImpl* impl_; // PIMPL for Objective-C objects

  void NotifySubscribers(const Image::Ptr& frame);
};

}  // namespace yoonvision::camera::avfcam

#endif  // YOONCAMERA_AVFCAM_AVF_CAMERA_HPP_
