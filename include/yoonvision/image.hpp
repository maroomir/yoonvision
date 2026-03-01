//
// Created by maroomir on 2021-07-05.
//

#ifndef YOONVISION_IMAGE_HPP_
#define YOONVISION_IMAGE_HPP_

#include <memory>
#include <string>
#include <vector>

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

static std::vector<unsigned char> ToByte(const int &number) {
  std::vector<unsigned char> bytes(4);
  bytes[0] = (number & 0xFF);
  bytes[1] = (number >> 8) & 0xFF;
  bytes[2] = (number >> 16) & 0xFF;
  bytes[3] = (number >> 24) & 0xFF;
  return bytes;
}

static int ToInteger(const std::vector<unsigned char> &bytes) {
  if (bytes.size() < 4) return image::kInvalidNum;
  return bytes[0] | bytes[1] << 8 | bytes[2] << 16 | bytes[3] << 24;
}

class Image {
 private:
  std::vector<unsigned char> ToMixedColorBuffer(
      const std::vector<unsigned char> &buffer,
      bool reverse_order = false) const;

  std::vector<unsigned char> ToParallelColorBuffer(
      const std::vector<unsigned char> &buffer,
      bool reverse_order = false) const;

 private:
  std::vector<unsigned char> buffer_;  // "Gray" or Parallel Color Buffers (R + G + B)
  size_t width_ = image::kDefaultWidth;
  size_t height_ = image::kDefaultHeight;
  size_t channel_ = image::kDefaultChannel;
  image::ImageFormat format_ = image::ImageFormat::kNone;

 public:
  Image();

  ~Image() = default;

  Image(const Image &image) = default;
  Image &operator=(const Image &image) = default;
  Image(Image &&image) = default;
  Image &operator=(Image &&image) = default;

  explicit Image(const std::string &image_path, image::FileFormat format);

  Image(size_t width, size_t height, size_t channel);

  Image(const std::vector<int> &buffer, size_t width, size_t height);

  Image(const std::vector<unsigned char> &red_buffer,
        const std::vector<unsigned char> &green_buffer,
        const std::vector<unsigned char> &blue_buffer, size_t width,
        size_t height);

  Image(const std::vector<unsigned char> &buffer, size_t width, size_t height,
        image::ImageFormat format);

  [[nodiscard]] size_t Width() const;

  [[nodiscard]] size_t Height() const;

  [[nodiscard]] size_t Channel() const;

  [[nodiscard]] size_t Stride() const;

  std::vector<unsigned char> &GetBuffer();
  const std::vector<unsigned char> &GetBuffer() const;

  std::vector<unsigned char> CopyBuffer() const;

  std::vector<unsigned char> GetMixedColorBuffer() const;

  image::ImageFormat ImageFormat() const;

  Image ToGrayImage() const;

  Image ToRedImage() const;

  Image ToGreenImage() const;

  Image ToBlueImage() const;

  std::vector<unsigned char> ToGrayBuffer() const;

  std::vector<unsigned char> ToRedBuffer() const;

  std::vector<unsigned char> ToGreenBuffer() const;

  std::vector<unsigned char> ToBlueBuffer() const;

  void CopyFrom(const Image &image);

  Image Clone() const;

  bool Equals(const Image &image) const;

  bool LoadBitmap(const std::string &path);

  bool SaveBitmap(const std::string &path) const;

  bool LoadJpeg(const std::string &path);

  bool SaveJpeg(const std::string &path) const;

 public:
  static Image GrayPaletteBar(int width = 256, int height = 50, int step = 10);

  static Image ColorPaletteBar(int width = 256, int height = 50, int step = 10);
};
}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_HPP_