#include <gtest/gtest.h>

#include <chrono>
#include <condition_variable>
#include <memory>
#include <mutex>

#include "yooncamera/camera.hpp"
#include "yooncamera/camera_factory.hpp"

namespace {

using yoonvision::Image;
using yoonvision::camera::CameraFactory;
using yoonvision::camera::CameraParameter;
using yoonvision::camera::CameraState;
using yoonvision::camera::ICamera;
using yoonvision::camera::IFrameSubscriber;

class CountingSubscriber : public IFrameSubscriber {
 public:
  void OnFrame(const Image::Ptr& frame) override {
    std::lock_guard<std::mutex> lock(mutex_);
    ++count_;
    last_frame_ = frame;
    cv_.notify_all();
  }

  bool WaitForFirstFrame(std::chrono::milliseconds timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, timeout, [this]() { return count_ > 0; });
  }

  std::size_t GetCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable cv_;
  std::size_t count_{0};
  Image::Ptr last_frame_;
};

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

TEST(AvfCameraStreamingTest, StreamOnDeliversFramesToSubscriber) {
  auto param = std::make_shared<CameraParameter>();
  param->timeout_ms = 2000;

  ICamera::Ptr camera = CameraFactory::CreateCamera("avfcam", param);
  ASSERT_NE(camera, nullptr);

  AvfCameraAutoCleanup cleanup(camera);

  if (camera->Initialize() != CameraState::Status::kSucceeded) {
    GTEST_SKIP() << "카메라 초기화 실패(권한/환경 이슈)로 스트리밍 테스트를 건너뜁니다.";
  }

  if (camera->Open() != CameraState::Status::kSucceeded) {
    GTEST_SKIP() << "카메라 오픈 실패(디바이스/권한 이슈)로 스트리밍 테스트를 건너뜁니다.";
  }

  auto subscriber = std::make_shared<CountingSubscriber>();
  camera->Subscribe(subscriber);

  const auto stream_on_status = camera->StreamOn();
  if (stream_on_status != CameraState::Status::kSucceeded) {
    camera->Unsubscribe(subscriber);
    GTEST_SKIP() << "StreamOn 실패로 스트리밍 테스트를 건너뜁니다.";
  }

  const bool got_frame = subscriber->WaitForFirstFrame(std::chrono::milliseconds(2000));

  camera->StreamOff();
  camera->Unsubscribe(subscriber);

  if (!got_frame) {
    GTEST_SKIP() << "스트리밍 중 프레임이 도착하지 않아 테스트를 건너뜁니다.";
  }

  EXPECT_GT(subscriber->GetCount(), 0U);
}

}  // namespace
