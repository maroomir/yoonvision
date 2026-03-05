#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <memory>

#include "yooncamera/camera_context_manager.hpp"
#include "mock_camera_context_manager.hpp"

using namespace yoonvision::camera;
using namespace yoonvision::camera::test;
using ::testing::Return;

class MockContextManagerTest : public ::testing::Test {
 protected:
  std::shared_ptr<MockCameraContextManager> ctx =
      std::make_shared<MockCameraContextManager>();
};

TEST_F(MockContextManagerTest, Initialize_ReturnsTrue) {
  EXPECT_CALL(*ctx, Initialize()).WillOnce(Return(true));
  EXPECT_TRUE(ctx->Initialize());
}

TEST_F(MockContextManagerTest, Initialize_ReturnsFalseOnError) {
  EXPECT_CALL(*ctx, Initialize()).WillOnce(Return(false));
  EXPECT_FALSE(ctx->Initialize());
}

TEST_F(MockContextManagerTest, IsInitialized_ReturnsTrueAfterInit) {
  EXPECT_CALL(*ctx, Initialize()).WillOnce(Return(true));
  EXPECT_CALL(*ctx, IsInitialized()).WillOnce(Return(true));

  ctx->Initialize();
  EXPECT_TRUE(ctx->IsInitialized());
}

TEST_F(MockContextManagerTest, IsInitialized_ReturnsFalseBeforeInit) {
  EXPECT_CALL(*ctx, IsInitialized()).WillOnce(Return(false));
  EXPECT_FALSE(ctx->IsInitialized());
}

TEST_F(MockContextManagerTest, Finalize_CalledOnce) {
  EXPECT_CALL(*ctx, Finalize()).Times(1);
  ctx->Finalize();
}

TEST_F(MockContextManagerTest, IsInitialized_ReturnsFalseAfterFinalize) {
  EXPECT_CALL(*ctx, Initialize()).WillOnce(Return(true));
  EXPECT_CALL(*ctx, Finalize()).Times(1);
  EXPECT_CALL(*ctx, IsInitialized()).WillOnce(Return(false));

  ctx->Initialize();
  ctx->Finalize();
  EXPECT_FALSE(ctx->IsInitialized());
}
