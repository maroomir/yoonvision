#include <gtest/gtest.h>

#include <memory>

#include "yooncamera/avfcam/avf_camera_context_manager.hpp"
#include "yooncamera/camera_factory.hpp"

namespace {

using yoonvision::camera::CameraFactory;
using yoonvision::camera::CameraParameter;
using yoonvision::camera::avfcam::AvfCameraContextManager;

TEST(AvfCameraFactoryTest, CreateRegisteredAvfComponents) {
  auto param = std::make_shared<CameraParameter>();
  auto camera = CameraFactory::CreateCamera("avfcam", param);
  auto context = CameraFactory::CreateContextManager("avfcam");

  ASSERT_NE(camera, nullptr);
  ASSERT_NE(context, nullptr);
}

TEST(AvfCameraContextManagerTest, InitializeAndFinalize) {
  AvfCameraContextManager context;

  if (!context.Initialize()) {
    GTEST_SKIP() << "카메라 권한 또는 디바이스가 없어 ContextManager 초기화를 건너뜁니다.";
  }

  EXPECT_TRUE(context.IsInitialized());

  context.Finalize();
  EXPECT_FALSE(context.IsInitialized());
}

}  // namespace
