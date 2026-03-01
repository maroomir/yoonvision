//
// Created by maroomir on 2021-07-05.
//

#ifndef YOONVISION_IMAGE_HPP_
#define YOONVISION_IMAGE_HPP_

#include <string>

#include "bitmap.hpp"
#include "figure.hpp"
#include "jpeg.hpp"

namespace yoonvision {
namespace image {
constexpr int kInvalidNum = -65536;
constexpr int kDefaultChannel = 1;
constexpr int kDefaultWidth = 640;
constexpr int kDefaultHeight = 480;
constexpr int kMaxChannel = 3;

enum class ImageFormat {
  kNone = -1,
  kGray,
  kRgb,
  kRgbParallel,
  kRgbMixed,
  kBgr,
  kBgrParallel,
  kBgrMixed,
  kMaxImageFormat
};

enum class FileFormat { kNone = -1, kBitmap = 0, kJpeg, kMaxFileFormat };

constexpr int kFormatToChannel[static_cast<int>(ImageFormat::kMaxImageFormat)] =
    {1, 3, 3, 3, 3, 3, 3};
constexpr ImageFormat kChannelToDefaultFormat[kMaxChannel + 1] = {
    ImageFormat::kNone,  // 0-channel (invalid)
    ImageFormat::kGray,  // 1-channel
    ImageFormat::kNone,  // 2-channel (unsupported)
    ImageFormat::kRgb    // 3-channel
};
}  // namespace image

static unsigned char *ToByte(const int &number) {
  auto *bytes = new unsigned char[4];
  bytes[0] = (number & 0xFF);
  bytes[1] = (number >> 8) & 0xFF;
  bytes[2] = (number >> 16) & 0xFF;
  bytes[3] = (number >> 24) & 0xFF;
  return bytes;
}

static int ToInteger(const unsigned char *bytes) {
  if (bytes == nullptr) return image::kInvalidNum;
  return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
}

class Image {
 private:
  unsigned char *ToMixedColorBuffer(const unsigned char *buffer,
                                    bool reverse_order = false) const;

  unsigned char *ToParallelColorBuffer(const unsigned char *buffer,
                                       bool reverse_order = false) const;

 private:
  unsigned char *buffer_;  // "Gray" or Parallel Color Buffers (R + G + B)
  size_t width_, height_, channel_;
  image::ImageFormat format_;

 public:
  Image();

  ~Image();

  Image(const Image &image);

  explicit Image(const std::string &image_path, image::FileFormat format);

  Image(size_t width, size_t height, size_t channel);

  Image(int *buffer, size_t width, size_t height);

  Image(unsigned char *red_buffer, unsigned char *green_buffer,
        unsigned char *blue_buffer, size_t width, size_t height);

  Image(unsigned char *buffer, size_t width, size_t height,
        image::ImageFormat format);

  [[nodiscard]] size_t Width() const;

  [[nodiscard]] size_t Height() const;

  [[nodiscard]] size_t Channel() const;

  [[nodiscard]] size_t Stride() const;

  unsigned char *GetBuffer();

  unsigned char *CopyBuffer();

  unsigned char *GetMixedColorBuffer();

  image::ImageFormat ImageFormat();

  Image *ToGrayImage();

  Image *ToRedImage();

  Image *ToGreenImage();

  Image *ToBlueImage();

  unsigned char *ToGrayBuffer();

  unsigned char *ToRedBuffer();

  unsigned char *ToGreenBuffer();

  unsigned char *ToBlueBuffer();

  void CopyFrom(const Image &image);

  Image *Clone();

  bool Equals(const Image &image);

  bool LoadBitmap(const std::string &path);

  bool SaveBitmap(const std::string &path);

  bool LoadJpeg(const std::string &path);

  bool SaveJpeg(const std::string &path);

 public:
  static Image *GrayPaletteBar(int width = 256, int height = 50, int step = 10);

  static Image *ColorPaletteBar(int width = 256, int height = 50,
                                int step = 10);
};
}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_HPP_
