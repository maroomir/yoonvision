#ifndef YOONCAMERA_TEST_MOCK_CAMERA_CONTEXT_MANAGER_HPP_
#define YOONCAMERA_TEST_MOCK_CAMERA_CONTEXT_MANAGER_HPP_

#include <gmock/gmock.h>

#include "yooncamera/camera_context_manager.hpp"

namespace yoonvision::camera::test {

class MockCameraContextManager : public ICameraContextManager {
 public:
  MOCK_METHOD(bool, Initialize, (), (override));
  MOCK_METHOD(void, Finalize, (), (override));
  MOCK_METHOD(bool, IsInitialized, (), (const, override));
};

}  // namespace yoonvision::camera::test

#endif  // YOONCAMERA_TEST_MOCK_CAMERA_CONTEXT_MANAGER_HPP_
