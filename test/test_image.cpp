#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "yoonvision/image.hpp"
#include "yoonvision/image_builder.hpp"

namespace {

std::string GetProjectRootPath() { return std::string(YOONVISION_PROJECT_DIR); }

std::string JoinPath(const std::string& lhs, const std::string& rhs) {
  std::filesystem::path lhs_path(lhs);
  lhs_path /= rhs;
  return lhs_path.string();
}

std::string GetLenaJpegPath() {
  return JoinPath(GetProjectRootPath(), "sample/lena/lena512.jpg");
}

std::string GetResultDirPath() {
  const std::string result_dir_path = JoinPath(GetProjectRootPath(), "result");
  std::filesystem::create_directories(result_dir_path);
  return result_dir_path;
}

}  // namespace

TEST(ImageTest, DefaultConstructorHasExpectedProperties) {
  using yoonvision::Image;

  Image image;

  ASSERT_EQ(image.GetWidth(), 640);
  ASSERT_EQ(image.GetHeight(), 480);
  ASSERT_EQ(image.GetChannel(), 1);
  ASSERT_EQ(image.GetStride(), 640);
  ASSERT_FALSE(image.GetBuffer().empty());
}

TEST(ImageTest, BuilderWithSizeHasExpectedProperties) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using ImageFormat = yoonvision::Image::ImageFormat;

  Image image = ImageBuilder().WithSize(10, 20, 3).Build();

  ASSERT_EQ(image.GetWidth(), 10);
  ASSERT_EQ(image.GetHeight(), 20);
  ASSERT_EQ(image.GetChannel(), 3);
  ASSERT_EQ(image.GetStride(), 30);
  ASSERT_EQ(image.GetImageFormat(), ImageFormat::kRgb);
}

TEST(ImageTest, ToGrayBufferConvertsRgbValuesUsingWeightedFormula) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using ImageFormat = yoonvision::Image::ImageFormat;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 1;
  // RGB parallel buffers: [R0, R1, G0, G1, B0, B1]
  // pixel0=(100,50,0), pixel1=(10,20,30)
  std::vector<unsigned char> buffer = {100, 10, 50, 20, 0, 30};
  Image image =
      ImageBuilder().FromBuffer(buffer, kWidth, kHeight, ImageFormat::kRgb).Build();

  std::vector<unsigned char> gray = image.ToGrayBuffer();

  ASSERT_EQ(gray[0],
            static_cast<unsigned char>(0.299 * 100 + 0.587 * 50 + 0.114 * 0));
  ASSERT_EQ(gray[1],
            static_cast<unsigned char>(0.299 * 10 + 0.587 * 20 + 0.114 * 30));
}

TEST(ImageTest, ToColorBuffersExtractExpectedChannelsFromRgbParallelBuffer) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using ImageFormat = yoonvision::Image::ImageFormat;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 1;
  std::vector<unsigned char> buffer = {1, 2, 3, 4, 5, 6};
  Image image =
      ImageBuilder().FromBuffer(buffer, kWidth, kHeight, ImageFormat::kRgb).Build();

  std::vector<unsigned char> red = image.ToRedBuffer();
  std::vector<unsigned char> green = image.ToGreenBuffer();
  std::vector<unsigned char> blue = image.ToBlueBuffer();

  ASSERT_EQ(red[0], 1);
  ASSERT_EQ(red[1], 2);
  ASSERT_EQ(green[0], 3);
  ASSERT_EQ(green[1], 4);
  ASSERT_EQ(blue[0], 5);
  ASSERT_EQ(blue[1], 6);
}

TEST(ImageTest, CloneEqualsAndCopyFromWorkAsExpected) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using ImageFormat = yoonvision::Image::ImageFormat;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 2;
  std::vector<unsigned char> buffer = {
      10, 20,  30,  40,  // R
      50, 60,  70,  80,  // G
      90, 100, 110, 120  // B
  };
  Image source =
      ImageBuilder().FromBuffer(buffer, kWidth, kHeight, ImageFormat::kRgb).Build();

  Image clone = source.Clone();
  ASSERT_TRUE(source.Equals(clone));

  clone.GetBuffer()[0] = static_cast<unsigned char>(clone.GetBuffer()[0] + 1);
  ASSERT_FALSE(source.Equals(clone));

  Image copied;
  copied.CopyFrom(source);
  ASSERT_TRUE(copied.Equals(source));
}

TEST(ImageTest, BuilderPaletteBarFactoryMethodsReturnExpectedDimensionsAndChannels) {
  using yoonvision::ImageBuilder;

  yoonvision::Image gray_bar = ImageBuilder::GrayPaletteBar(4, 3, 2);
  ASSERT_EQ(gray_bar.GetWidth(), 8);
  ASSERT_EQ(gray_bar.GetHeight(), 3);
  ASSERT_EQ(gray_bar.GetChannel(), 1);

  yoonvision::Image color_bar = ImageBuilder::ColorPaletteBar(4, 3, 2);
  ASSERT_EQ(color_bar.GetWidth(), 24);
  ASSERT_EQ(color_bar.GetHeight(), 3);
  ASSERT_EQ(color_bar.GetChannel(), 3);
}

TEST(ImageTest, BuilderFromFileReturnsFalseForInvalidPath) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;

  const std::string root_path = GetProjectRootPath();
  const std::string invalid_path =
      JoinPath(root_path, "sample/lena/not_found.jpg");
  const std::string invalid_save_path =
      JoinPath(root_path, "not_exists_dir/out.bmp");

  Image image =
      ImageBuilder().FromFile(invalid_path, Image::FileFormat::kJpeg).Build();
  ASSERT_EQ(image.GetWidth(), 640);
  ASSERT_EQ(image.GetHeight(), 480);

  ASSERT_FALSE(image.SaveBitmap(invalid_save_path));
}

TEST(ImageTest, BuilderFromJpegFileHasExpectedDimensions) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  ASSERT_EQ(image.GetWidth(), 512);
  ASSERT_EQ(image.GetHeight(), 512);
}

TEST(ImageTest, OriginalImageCanBeSavedAsBitmapAndJpeg) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  const std::string result_dir_path = GetResultDirPath();
  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  ASSERT_TRUE(image.SaveBitmap(JoinPath(result_dir_path, "lena_origin.bmp")));
  ASSERT_TRUE(image.SaveJpeg(JoinPath(result_dir_path, "lena_origin.jpg")));
}

TEST(ImageTest, ToGrayImageCanBeSavedAsBitmap) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  const std::string result_dir_path = GetResultDirPath();
  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  Image gray_image = image.ToGrayImage();
  ASSERT_TRUE(
      gray_image.SaveBitmap(JoinPath(result_dir_path, "lena_gray.bmp")));
}

TEST(ImageTest, ToRedImageCanBeSavedAsBitmap) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  const std::string result_dir_path = GetResultDirPath();
  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  Image red_image = image.ToRedImage();
  ASSERT_TRUE(red_image.SaveBitmap(JoinPath(result_dir_path, "lena_red.bmp")));
}

TEST(ImageTest, ToGreenImageCanBeSavedAsBitmap) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  const std::string result_dir_path = GetResultDirPath();
  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  Image green_image = image.ToGreenImage();
  ASSERT_TRUE(
      green_image.SaveBitmap(JoinPath(result_dir_path, "lena_green.bmp")));
}

TEST(ImageTest, ToBlueImageCanBeSavedAsBitmap) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  const std::string result_dir_path = GetResultDirPath();
  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();

  Image blue_image = image.ToBlueImage();
  ASSERT_TRUE(
      blue_image.SaveBitmap(JoinPath(result_dir_path, "lena_blue.bmp")));
}

TEST(ImageTest, GrayPaletteBarCanBeSavedAsJpeg) {
  using yoonvision::ImageBuilder;

  const std::string result_dir_path = GetResultDirPath();
  yoonvision::Image gray_bar = ImageBuilder::GrayPaletteBar();

  ASSERT_TRUE(gray_bar.SaveJpeg(JoinPath(result_dir_path, "gray_bar.jpg")));
}

TEST(ImageTest, ColorPaletteBarCanBeSavedAsJpeg) {
  using yoonvision::ImageBuilder;

  const std::string result_dir_path = GetResultDirPath();
  yoonvision::Image color_bar = ImageBuilder::ColorPaletteBar();

  ASSERT_TRUE(color_bar.SaveJpeg(JoinPath(result_dir_path, "color_bar.jpg")));
}

TEST(ImageTest, BuilderFromJpegAndCreateDerivedImages) {
  using yoonvision::Image;
  using yoonvision::ImageBuilder;
  using FileFormat = yoonvision::Image::FileFormat;

  Image image =
      ImageBuilder().FromFile(GetLenaJpegPath(), FileFormat::kJpeg).Build();
  ASSERT_EQ(image.GetWidth(), 512);
  ASSERT_EQ(image.GetHeight(), 512);

  Image gray_image = image.ToGrayImage();
  Image red_image = image.ToRedImage();
  Image green_image = image.ToGreenImage();
  Image blue_image = image.ToBlueImage();

  ASSERT_EQ(gray_image.GetChannel(), 1);
  ASSERT_EQ(red_image.GetChannel(), 1);
  ASSERT_EQ(green_image.GetChannel(), 1);
  ASSERT_EQ(blue_image.GetChannel(), 1);
}
