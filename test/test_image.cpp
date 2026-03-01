#include <gtest/gtest.h>

#include <filesystem>
#include <string>

#include "yoonvision/image.hpp"

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

  ASSERT_EQ(image.Width(), 640);
  ASSERT_EQ(image.Height(), 480);
  ASSERT_EQ(image.Channel(), 1);
  ASSERT_EQ(image.Stride(), 640);
  ASSERT_NE(image.GetBuffer(), nullptr);
}

TEST(ImageTest, ConstructorWithSizeAndChannelHasExpectedProperties) {
  using yoonvision::Image;

  Image image(10, 20, 3);

  ASSERT_EQ(image.Width(), 10);
  ASSERT_EQ(image.Height(), 20);
  ASSERT_EQ(image.Channel(), 3);
  ASSERT_EQ(image.Stride(), 30);
  ASSERT_EQ(image.ImageFormat(), yoonvision::image::ImageFormat::kRgb);
}

TEST(ImageTest, ToGrayBufferConvertsRgbValuesUsingWeightedFormula) {
  using yoonvision::Image;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 1;
  // RGB parallel buffers: [R0, R1, G0, G1, B0, B1]
  // pixel0=(100,50,0), pixel1=(10,20,30)
  unsigned char buffer[] = {100, 10, 50, 20, 0, 30};
  Image image(buffer, kWidth, kHeight, yoonvision::image::ImageFormat::kRgb);

  unsigned char* gray = image.ToGrayBuffer();

  ASSERT_EQ(gray[0],
            static_cast<unsigned char>(0.299 * 100 + 0.587 * 50 + 0.114 * 0));
  ASSERT_EQ(gray[1],
            static_cast<unsigned char>(0.299 * 10 + 0.587 * 20 + 0.114 * 30));

  delete[] gray;
}

TEST(ImageTest, ToColorBuffersExtractExpectedChannelsFromRgbParallelBuffer) {
  using yoonvision::Image;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 1;
  unsigned char buffer[] = {1, 2, 3, 4, 5, 6};
  Image image(buffer, kWidth, kHeight, yoonvision::image::ImageFormat::kRgb);

  unsigned char* red = image.ToRedBuffer();
  unsigned char* green = image.ToGreenBuffer();
  unsigned char* blue = image.ToBlueBuffer();

  ASSERT_EQ(red[0], 1);
  ASSERT_EQ(red[1], 2);
  ASSERT_EQ(green[0], 3);
  ASSERT_EQ(green[1], 4);
  ASSERT_EQ(blue[0], 5);
  ASSERT_EQ(blue[1], 6);

  delete[] red;
  delete[] green;
  delete[] blue;
}

TEST(ImageTest, CloneEqualsAndCopyFromWorkAsExpected) {
  using yoonvision::Image;

  constexpr size_t kWidth = 2;
  constexpr size_t kHeight = 2;
  unsigned char buffer[] = {
      10, 20,  30,  40,  // R
      50, 60,  70,  80,  // G
      90, 100, 110, 120  // B
  };
  Image source(buffer, kWidth, kHeight, yoonvision::image::ImageFormat::kRgb);

  Image* clone = source.Clone();
  ASSERT_TRUE(source.Equals(*clone));

  clone->GetBuffer()[0] = static_cast<unsigned char>(clone->GetBuffer()[0] + 1);
  ASSERT_FALSE(source.Equals(*clone));

  Image copied;
  copied.CopyFrom(source);
  ASSERT_TRUE(copied.Equals(source));

  delete clone;
}

TEST(ImageTest, PaletteBarFactoryMethodsReturnExpectedDimensionsAndChannels) {
  using yoonvision::Image;

  Image* gray_bar = Image::GrayPaletteBar(4, 3, 2);
  ASSERT_EQ(gray_bar->Width(), 8);
  ASSERT_EQ(gray_bar->Height(), 3);
  ASSERT_EQ(gray_bar->Channel(), 1);

  Image* color_bar = Image::ColorPaletteBar(4, 3, 2);
  ASSERT_EQ(color_bar->Width(), 24);
  ASSERT_EQ(color_bar->Height(), 3);
  ASSERT_EQ(color_bar->Channel(), 3);

  delete gray_bar;
  delete color_bar;
}

TEST(ImageTest, LoadAndSaveReturnFalseForInvalidPath) {
  using yoonvision::Image;

  const std::string root_path = GetProjectRootPath();
  const std::string invalid_source_path =
      JoinPath(root_path, "sample/lena/not_found.jpg");
  const std::string invalid_save_path =
      JoinPath(root_path, "not_exists_dir/out.bmp");

  Image image;
  ASSERT_FALSE(image.LoadJpeg(invalid_source_path));
  ASSERT_FALSE(image.SaveBitmap(invalid_save_path));
}

TEST(ImageTest, LoadJpegHasExpectedDimensions) {
  using yoonvision::Image;

  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  ASSERT_EQ(image.Width(), 512);
  ASSERT_EQ(image.Height(), 512);
}

TEST(ImageTest, OriginalImageCanBeSavedAsBitmapAndJpeg) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  ASSERT_TRUE(image.SaveBitmap(JoinPath(result_dir_path, "lena_origin.bmp")));
  ASSERT_TRUE(image.SaveJpeg(JoinPath(result_dir_path, "lena_origin.jpg")));
}

TEST(ImageTest, ToGrayImageCanBeSavedAsBitmap) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  Image* gray_image = image.ToGrayImage();
  ASSERT_TRUE(
      gray_image->SaveBitmap(JoinPath(result_dir_path, "lena_gray.bmp")));
  delete gray_image;
}

TEST(ImageTest, ToRedImageCanBeSavedAsBitmap) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  Image* red_image = image.ToRedImage();
  ASSERT_TRUE(red_image->SaveBitmap(JoinPath(result_dir_path, "lena_red.bmp")));
  delete red_image;
}

TEST(ImageTest, ToGreenImageCanBeSavedAsBitmap) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  Image* green_image = image.ToGreenImage();
  ASSERT_TRUE(
      green_image->SaveBitmap(JoinPath(result_dir_path, "lena_green.bmp")));
  delete green_image;
}

TEST(ImageTest, ToBlueImageCanBeSavedAsBitmap) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);

  Image* blue_image = image.ToBlueImage();
  ASSERT_TRUE(
      blue_image->SaveBitmap(JoinPath(result_dir_path, "lena_blue.bmp")));
  delete blue_image;
}

TEST(ImageTest, GrayPaletteBarCanBeSavedAsJpeg) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image* gray_bar = Image::GrayPaletteBar();

  ASSERT_TRUE(gray_bar->SaveJpeg(JoinPath(result_dir_path, "gray_bar.jpg")));
  delete gray_bar;
}

TEST(ImageTest, ColorPaletteBarCanBeSavedAsJpeg) {
  using yoonvision::Image;

  const std::string result_dir_path = GetResultDirPath();
  Image* color_bar = Image::ColorPaletteBar();

  ASSERT_TRUE(color_bar->SaveJpeg(JoinPath(result_dir_path, "color_bar.jpg")));
  delete color_bar;
}

TEST(ImageTest, LoadJpegAndCreateDerivedImages) {
  using yoonvision::Image;

  Image image(GetLenaJpegPath(), yoonvision::image::FileFormat::kJpeg);
  ASSERT_EQ(image.Width(), 512);
  ASSERT_EQ(image.Height(), 512);

  Image* gray_image = image.ToGrayImage();
  Image* red_image = image.ToRedImage();
  Image* green_image = image.ToGreenImage();
  Image* blue_image = image.ToBlueImage();
  ASSERT_NE(gray_image, nullptr);
  ASSERT_NE(red_image, nullptr);
  ASSERT_NE(green_image, nullptr);
  ASSERT_NE(blue_image, nullptr);

  delete gray_image;
  delete red_image;
  delete green_image;
  delete blue_image;
}
