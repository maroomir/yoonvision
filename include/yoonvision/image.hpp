//
// Created by maroomir on 2021-07-05.
//

#ifndef YOONVISION_IMAGE_HPP_
#define YOONVISION_IMAGE_HPP_

#include <memory>
#include <string>
#include <vector>

#include "bitmap.hpp"
#include "byte.hpp"
#include "figure.hpp"
#include "jpeg.hpp"

namespace yoonvision {

class Image {
public:
  enum class ImageFormat { kNone, kGray,
                           kRgb, kRgbParallel, kRgbMixed,
                           kBgr, kBgrParallel, kBgrMixed,
                           kMaxImageFormat };
  enum class FileFormat { kNone, kBitmap, kJpeg, kMaxFileFormat };

  Image();
  ~Image() = default;

  Image(const Image &image) = default;
  Image &operator=(const Image &image) = default;
  Image(Image &&image) = default;
  Image &operator=(Image &&image) = default;

  explicit Image(const std::string &image_path, FileFormat format);

  Image(size_t width, size_t height, size_t channel);
  Image(const std::vector<int> &buffer, 
        size_t width, size_t height);
  Image(const std::vector<byte> &red_buffer,
        const std::vector<byte> &green_buffer,
        const std::vector<byte> &blue_buffer, 
        size_t width, size_t height);

  Image(const std::vector<byte> &buffer, 
        size_t width, size_t height,
        ImageFormat format);

  [[nodiscard]] size_t GetWidth() const;
  [[nodiscard]] size_t GetHeight() const;
  [[nodiscard]] size_t GetChannel() const;
  [[nodiscard]] size_t GetStride() const;
  std::vector<byte> &GetBuffer();
  const std::vector<byte> &GetBuffer() const;

  std::vector<byte> CopyBuffer() const;
  std::vector<byte> GetMixedColorBuffer() const;

  ImageFormat GetImageFormat() const;

  Image ToGrayImage() const;
  Image ToRedImage() const;
  Image ToGreenImage() const;
  Image ToBlueImage() const;

  std::vector<byte> ToGrayBuffer() const;
  std::vector<byte> ToRedBuffer() const;
  std::vector<byte> ToGreenBuffer() const;
  std::vector<byte> ToBlueBuffer() const;

  void CopyFrom(const Image &image);

  Image Clone() const;

  bool Equals(const Image &image) const;

  bool LoadBitmap(const std::string &path);
  bool SaveBitmap(const std::string &path) const;

  bool LoadJpeg(const std::string &path);
  bool SaveJpeg(const std::string &path) const;
 
  static Image GrayPaletteBar(int width = 256, int height = 50, int step = 10);
  static Image ColorPaletteBar(int width = 256, int height = 50, int step = 10);

private:
  ImageFormat ChannelToDefaultFormat(size_t channel) const;

  std::vector<byte> ToMixedColorBuffer(const std::vector<byte> &buffer,
                                       bool reverse_order = false) const;
  std::vector<byte> ToParallelColorBuffer(const std::vector<byte> &buffer,
                                          bool reverse_order = false) const;

  size_t width_;
  size_t height_;
  size_t channel_;
  ImageFormat format_;
  std::vector<byte> buffer_;  // "Gray" or Parallel Color Buffers (R + G + B)

};
}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_HPP_