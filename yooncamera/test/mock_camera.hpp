#ifndef YOONCAMERA_TEST_MOCK_CAMERA_HPP_
#define YOONCAMERA_TEST_MOCK_CAMERA_HPP_

#include <gmock/gmock.h>

#include "yooncamera/camera.hpp"

namespace yoonvision::camera::test {

class MockCamera : public ICamera {
 public:
  MOCK_METHOD(CameraState::Status, Initialize, (), (override));
  MOCK_METHOD(void, Finalize, (), (override));
  MOCK_METHOD(CameraState::Status, Open, (), (override));
  MOCK_METHOD(void, Close, (), (override));

  MOCK_METHOD(CameraState::Status, Capture, (), (override));
  MOCK_METHOD(CameraState::Status, StreamOn, (), (override));
  MOCK_METHOD(void, StreamOff, (), (override));

  MOCK_METHOD(void, SetupParameter, (std::shared_ptr<CameraParameter> param),
              (override));

  MOCK_METHOD(Image::Ptr, GetCaptureFrame, (), (override));
  MOCK_METHOD(CameraState, GetCameraState, (), (const, override));

  MOCK_METHOD(bool, IsInitialized, (), (const, override));
  MOCK_METHOD(bool, IsFinalized, (), (const, override));

  MOCK_METHOD(void, Subscribe, (IFrameSubscriber::Ptr subscriber), (override));
  MOCK_METHOD(void, Unsubscribe, (IFrameSubscriber::Ptr subscriber), (override));
};

}  // namespace yoonvision::camera::test

#endif  // YOONCAMERA_TEST_MOCK_CAMERA_HPP_
