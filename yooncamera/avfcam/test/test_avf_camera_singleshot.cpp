#include <gtest/gtest.h>

#include <memory>

#include "yooncamera/camera.hpp"
#include "yooncamera/camera_factory.hpp"

namespace {

using yoonvision::Image;
using yoonvision::camera::CameraFactory;
using yoonvision::camera::CameraParameter;
using yoonvision::camera::CameraState;
using yoonvision::camera::ICamera;

class AvfCameraAutoCleanup {
 public:
  explicit AvfCameraAutoCleanup(ICamera::Ptr camera) : camera_(std::move(camera)) {}

  ~AvfCameraAutoCleanup() {
    if (!camera_) {
      return;
    }
    camera_->StreamOff();
    camera_->Close();
    if (!camera_->IsFinalized()) {
      camera_->Finalize();
    }
  }

 private:
  ICamera::Ptr camera_;
};

TEST(AvfCameraSingleShotTest, CaptureFrameWithFactoryFlow) {
  auto param = std::make_shared<CameraParameter>();
  param->timeout_ms = 2000;

  ICamera::Ptr camera = CameraFactory::CreateCamera("avfcam", param);
  ASSERT_NE(camera, nullptr);

  AvfCameraAutoCleanup cleanup(camera);

  if (camera->Initialize() != CameraState::Status::kSucceeded) {
    GTEST_SKIP() << "카메라 초기화 실패(권한/환경 이슈)로 싱글샷 테스트를 건너뜁니다.";
  }

  if (camera->Open() != CameraState::Status::kSucceeded) {
    GTEST_SKIP() << "카메라 오픈 실패(디바이스/권한 이슈)로 싱글샷 테스트를 건너뜁니다.";
  }

  const auto capture_status = camera->Capture();
  if (capture_status != CameraState::Status::kSucceeded) {
    GTEST_SKIP() << "Capture 실패(프레임 미수신 가능)로 싱글샷 테스트를 건너뜁니다.";
  }

  Image::Ptr frame = camera->GetCaptureFrame();
  ASSERT_NE(frame, nullptr);
  EXPECT_GT(frame->GetWidth(), 0U);
  EXPECT_GT(frame->GetHeight(), 0U);
  EXPECT_GE(frame->GetChannel(), 1U);
}

}  // namespace
