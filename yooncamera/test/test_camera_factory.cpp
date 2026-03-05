#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>
#include <string>

#include "yooncamera/camera_factory.hpp"
#include "mock_camera.hpp"
#include "mock_camera_context_manager.hpp"

using namespace yoonvision::camera;
using namespace yoonvision::camera::test;
using ::testing::Return;

class CameraFactoryTest : public ::testing::Test {
 protected:
  void SetUp() override { CameraFactory::ClearAllCreators(); }
  void TearDown() override { CameraFactory::ClearAllCreators(); }
};

TEST_F(CameraFactoryTest, HasCreator_ReturnsFalseBeforeRegister) {
  EXPECT_FALSE(CameraFactory::HasCameraCreator("mock"));
}

TEST_F(CameraFactoryTest, RegisterAndHasCreator_ReturnsTrueAfterRegister) {
  CameraFactory::RegisterCameraCreator("mock", [](auto /*param*/) {
    return std::make_shared<MockCamera>();
  });
  EXPECT_TRUE(CameraFactory::HasCameraCreator("mock"));
}

TEST_F(CameraFactoryTest, CreateCamera_ReturnsNonNullForRegisteredType) {
  CameraFactory::RegisterCameraCreator("mock", [](auto /*param*/) {
    return std::make_shared<MockCamera>();
  });
  auto param = std::make_shared<CameraParameter>();
  auto camera = CameraFactory::CreateCamera("mock", param);
  ASSERT_NE(camera, nullptr);
}

TEST_F(CameraFactoryTest, CreateCamera_ReturnsNullForUnregisteredType) {
  auto param = std::make_shared<CameraParameter>();
  auto camera = CameraFactory::CreateCamera("unregistered", param);
  EXPECT_EQ(camera, nullptr);
}

TEST_F(CameraFactoryTest, CreateCamera_ReturnsNullAfterClearAllCreators) {
  CameraFactory::RegisterCameraCreator("mock", [](auto /*param*/) {
    return std::make_shared<MockCamera>();
  });
  CameraFactory::ClearAllCreators();

  auto param = std::make_shared<CameraParameter>();
  auto camera = CameraFactory::CreateCamera("mock", param);
  EXPECT_EQ(camera, nullptr);
}

TEST_F(CameraFactoryTest, RegisterCreator_EmptyTypeIsIgnored) {
  CameraFactory::RegisterCameraCreator("", [](auto /*param*/) {
    return std::make_shared<MockCamera>();
  });
  EXPECT_FALSE(CameraFactory::HasCameraCreator(""));
}

TEST_F(CameraFactoryTest, RegisterCreator_NullCreatorIsIgnored) {
  CameraFactory::RegisterCameraCreator("mock", nullptr);
  EXPECT_FALSE(CameraFactory::HasCameraCreator("mock"));
}

TEST_F(CameraFactoryTest, HasContextManagerCreator_ReturnsFalseBeforeRegister) {
  EXPECT_FALSE(CameraFactory::HasContextManagerCreator("mock"));
}

TEST_F(CameraFactoryTest, RegisterContextManager_ReturnsTrueAfterRegister) {
  CameraFactory::RegisterContextManagerCreator("mock", []() {
    return std::make_shared<MockCameraContextManager>();
  });
  EXPECT_TRUE(CameraFactory::HasContextManagerCreator("mock"));
}

TEST_F(CameraFactoryTest, CreateContextManager_ReturnsNonNullForRegisteredType) {
  CameraFactory::RegisterContextManagerCreator("mock", []() {
    return std::make_shared<MockCameraContextManager>();
  });
  auto ctx = CameraFactory::CreateContextManager("mock");
  ASSERT_NE(ctx, nullptr);
}

TEST_F(CameraFactoryTest, CreateContextManager_ReturnsNullForUnregisteredType) {
  auto ctx = CameraFactory::CreateContextManager("unregistered");
  EXPECT_EQ(ctx, nullptr);
}

TEST_F(CameraFactoryTest, ClearAllCreators_RemovesBothCameraAndContextCreators) {
  CameraFactory::RegisterCameraCreator("mock", [](auto /*param*/) {
    return std::make_shared<MockCamera>();
  });
  CameraFactory::RegisterContextManagerCreator("mock", []() {
    return std::make_shared<MockCameraContextManager>();
  });
  CameraFactory::ClearAllCreators();

  EXPECT_FALSE(CameraFactory::HasCameraCreator("mock"));
  EXPECT_FALSE(CameraFactory::HasContextManagerCreator("mock"));
}
