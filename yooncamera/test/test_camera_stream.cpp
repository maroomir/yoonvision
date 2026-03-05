#include <gtest/gtest.h>

#include <string>

#include "yooncamera/camera_stream.hpp"

using namespace yoonvision::camera;

class CameraStreamTest : public ::testing::Test {};

TEST_F(CameraStreamTest, DefaultConstructor_TypeIsNone) {
  CameraStream stream;
  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kNone);
}

TEST_F(CameraStreamTest, TypeConstructor_ColorStream) {
  CameraStream stream(CameraStream::Type::kColor, 1920, 1080, "RGB8", 30);

  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kColor);
  EXPECT_EQ(stream.GetWidth(), 1920);
  EXPECT_EQ(stream.GetHeight(), 1080);
  EXPECT_EQ(stream.GetFormat(), "RGB8");
  EXPECT_EQ(stream.GetFrameRate(), 30);
  EXPECT_TRUE(stream.GetEnabled());
}

TEST_F(CameraStreamTest, TypeConstructor_DepthStream) {
  CameraStream stream(CameraStream::Type::kDepth, 640, 480, "Z16", 15);

  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kDepth);
  EXPECT_EQ(stream.GetWidth(), 640);
  EXPECT_EQ(stream.GetHeight(), 480);
  EXPECT_EQ(stream.GetFormat(), "Z16");
  EXPECT_EQ(stream.GetFrameRate(), 15);
}

TEST_F(CameraStreamTest, TypeConstructor_InfraredStream) {
  CameraStream stream(CameraStream::Type::kInfrared, 640, 480, "Y8", 30);

  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kInfrared);
}

TEST_F(CameraStreamTest, TypeConstructor_OutputStream) {
  CameraStream stream(CameraStream::Type::kOutput, 1280, 720, "NV12", 30);

  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kOutput);
}

TEST_F(CameraStreamTest, TypeConstructor_BufferCountDefault) {
  CameraStream stream(CameraStream::Type::kColor, 640, 480, "RGB8", 30);
  EXPECT_EQ(stream.GetBufferCount(), 10);
}

TEST_F(CameraStreamTest, TypeConstructor_BufferCountCustom) {
  CameraStream stream(CameraStream::Type::kColor, 640, 480, "RGB8", 30, 5);
  EXPECT_EQ(stream.GetBufferCount(), 5);
}

TEST_F(CameraStreamTest, TypeConstructor_EnabledFalse) {
  CameraStream stream(CameraStream::Type::kColor, 640, 480, "RGB8", 30, 10,
                      false);
  EXPECT_FALSE(stream.GetEnabled());
}

TEST_F(CameraStreamTest, StringConstructor_ColorStream) {
  CameraStream stream("color", 1920, 1080, "RGB8", 30);
  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kColor);
}

TEST_F(CameraStreamTest, StringConstructor_DepthStream) {
  CameraStream stream("depth", 640, 480, "Z16", 15);
  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kDepth);
}

TEST_F(CameraStreamTest, StringConstructor_InfraredStream) {
  CameraStream stream("infrared", 640, 480, "Y8", 30);
  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kInfrared);
}

TEST_F(CameraStreamTest, StringConstructor_OutputStream) {
  CameraStream stream("output", 1280, 720, "NV12", 30);
  EXPECT_EQ(stream.GetStreamType(), CameraStream::Type::kOutput);
}

TEST_F(CameraStreamTest, GetStreamTypeFromString_Color) {
  EXPECT_EQ(CameraStream::GetStreamTypeFromString("color"),
            CameraStream::Type::kColor);
}

TEST_F(CameraStreamTest, GetStreamTypeFromString_Depth) {
  EXPECT_EQ(CameraStream::GetStreamTypeFromString("depth"),
            CameraStream::Type::kDepth);
}

TEST_F(CameraStreamTest, GetStreamTypeFromString_Infrared) {
  EXPECT_EQ(CameraStream::GetStreamTypeFromString("infrared"),
            CameraStream::Type::kInfrared);
}

TEST_F(CameraStreamTest, GetStreamTypeFromString_Output) {
  EXPECT_EQ(CameraStream::GetStreamTypeFromString("output"),
            CameraStream::Type::kOutput);
}

TEST_F(CameraStreamTest, GetStreamTypeFromString_Unknown_ReturnsNone) {
  EXPECT_EQ(CameraStream::GetStreamTypeFromString("unknown"),
            CameraStream::Type::kNone);
}

TEST_F(CameraStreamTest, GetStreamTypeToString_Color) {
  EXPECT_EQ(CameraStream::GetStreamTypeToString(CameraStream::Type::kColor),
            "color");
}

TEST_F(CameraStreamTest, GetStreamTypeToString_Depth) {
  EXPECT_EQ(CameraStream::GetStreamTypeToString(CameraStream::Type::kDepth),
            "depth");
}

TEST_F(CameraStreamTest, GetStreamTypeToString_Infrared) {
  EXPECT_EQ(
      CameraStream::GetStreamTypeToString(CameraStream::Type::kInfrared),
      "infrared");
}

TEST_F(CameraStreamTest, GetStreamTypeToString_Output) {
  EXPECT_EQ(CameraStream::GetStreamTypeToString(CameraStream::Type::kOutput),
            "output");
}

TEST_F(CameraStreamTest, TypeRoundTrip_ColorStringAndBack) {
  const auto type = CameraStream::Type::kColor;
  const auto str = CameraStream::GetStreamTypeToString(type);
  EXPECT_EQ(CameraStream::GetStreamTypeFromString(str), type);
}

TEST_F(CameraStreamTest, TypeRoundTrip_DepthStringAndBack) {
  const auto type = CameraStream::Type::kDepth;
  const auto str = CameraStream::GetStreamTypeToString(type);
  EXPECT_EQ(CameraStream::GetStreamTypeFromString(str), type);
}

TEST_F(CameraStreamTest, MemberGetStreamTypeToString_MatchesStatic) {
  CameraStream stream(CameraStream::Type::kColor, 640, 480, "RGB8", 30);
  EXPECT_EQ(stream.GetStreamTypeToString(),
            CameraStream::GetStreamTypeToString(CameraStream::Type::kColor));
}
