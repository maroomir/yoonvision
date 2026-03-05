#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "yooncamera/camera.hpp"
#include "mock_camera.hpp"

using namespace yoonvision::camera;
using namespace yoonvision::camera::test;
using ::testing::Return;

class MockCameraTest : public ::testing::Test {
 protected:
  std::shared_ptr<MockCamera> camera = std::make_shared<MockCamera>();
};

TEST_F(MockCameraTest, Initialize_ReturnsSucceeded) {
  EXPECT_CALL(*camera, Initialize())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_EQ(camera->Initialize(), CameraState::Status::kSucceeded);
}

TEST_F(MockCameraTest, Initialize_ReturnsFailed) {
  EXPECT_CALL(*camera, Initialize())
      .WillOnce(Return(CameraState::Status::kFailed));
  EXPECT_EQ(camera->Initialize(), CameraState::Status::kFailed);
}

TEST_F(MockCameraTest, Open_ReturnsSucceeded) {
  EXPECT_CALL(*camera, Open())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_EQ(camera->Open(), CameraState::Status::kSucceeded);
}

TEST_F(MockCameraTest, InitializeAndOpen_BothSucceeded) {
  EXPECT_CALL(*camera, Initialize())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_CALL(*camera, Open())
      .WillOnce(Return(CameraState::Status::kSucceeded));

  EXPECT_EQ(camera->Initialize(), CameraState::Status::kSucceeded);
  EXPECT_EQ(camera->Open(), CameraState::Status::kSucceeded);
}

TEST_F(MockCameraTest, StreamOn_ReturnsSucceeded) {
  EXPECT_CALL(*camera, StreamOn())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_EQ(camera->StreamOn(), CameraState::Status::kSucceeded);
}

TEST_F(MockCameraTest, StreamOff_CalledOnce) {
  EXPECT_CALL(*camera, StreamOff()).Times(1);
  camera->StreamOff();
}

TEST_F(MockCameraTest, StreamOnAndOff_Sequence) {
  EXPECT_CALL(*camera, StreamOn())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_CALL(*camera, StreamOff()).Times(1);

  EXPECT_EQ(camera->StreamOn(), CameraState::Status::kSucceeded);
  camera->StreamOff();
}

TEST_F(MockCameraTest, Capture_ReturnsSucceeded) {
  EXPECT_CALL(*camera, Capture())
      .WillOnce(Return(CameraState::Status::kSucceeded));
  EXPECT_EQ(camera->Capture(), CameraState::Status::kSucceeded);
}

TEST_F(MockCameraTest, Capture_ReturnsFailed) {
  EXPECT_CALL(*camera, Capture())
      .WillOnce(Return(CameraState::Status::kFailed));
  EXPECT_EQ(camera->Capture(), CameraState::Status::kFailed);
}

TEST_F(MockCameraTest, IsInitialized_ReturnsTrue) {
  EXPECT_CALL(*camera, IsInitialized()).WillOnce(Return(true));
  EXPECT_TRUE(camera->IsInitialized());
}

TEST_F(MockCameraTest, IsInitialized_ReturnsFalse) {
  EXPECT_CALL(*camera, IsInitialized()).WillOnce(Return(false));
  EXPECT_FALSE(camera->IsInitialized());
}

TEST_F(MockCameraTest, IsFinalized_ReturnsTrue) {
  EXPECT_CALL(*camera, IsFinalized()).WillOnce(Return(true));
  EXPECT_TRUE(camera->IsFinalized());
}

TEST_F(MockCameraTest, IsFinalized_ReturnsFalse) {
  EXPECT_CALL(*camera, IsFinalized()).WillOnce(Return(false));
  EXPECT_FALSE(camera->IsFinalized());
}

TEST_F(MockCameraTest, Close_CalledOnce) {
  EXPECT_CALL(*camera, Close()).Times(1);
  camera->Close();
}

TEST_F(MockCameraTest, Finalize_CalledOnce) {
  EXPECT_CALL(*camera, Finalize()).Times(1);
  camera->Finalize();
}

TEST_F(MockCameraTest, GetCaptureFrame_ReturnsNull) {
  EXPECT_CALL(*camera, GetCaptureFrame()).WillOnce(Return(nullptr));
  EXPECT_EQ(camera->GetCaptureFrame(), nullptr);
}

TEST_F(MockCameraTest, SetupParameter_CalledWithExpectedParam) {
  auto param = std::make_shared<CameraParameter>();
  param->name = "test_camera";
  param->device_path = "/dev/video0";
  param->timeout_ms = 500;

  EXPECT_CALL(*camera, SetupParameter(param)).Times(1);
  camera->SetupParameter(param);
}

TEST_F(MockCameraTest, Subscribe_And_Unsubscribe_CalledOnce) {
  class DummySubscriber : public IFrameSubscriber {
   public:
    void OnFrame(const yoonvision::Image::Ptr& /*frame*/) override {}
  };
  IFrameSubscriber::Ptr subscriber = std::make_shared<DummySubscriber>();

  EXPECT_CALL(*camera, Subscribe(subscriber)).Times(1);
  EXPECT_CALL(*camera, Unsubscribe(subscriber)).Times(1);

  camera->Subscribe(subscriber);
  camera->Unsubscribe(subscriber);
}
