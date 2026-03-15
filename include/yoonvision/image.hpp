//
// Created by maroomir on 2021-07-05.
//

#ifndef YOONVISION_IMAGE_HPP_
#define YOONVISION_IMAGE_HPP_

#include <string>
#include <vector>
#include <memory>

#include "byte.hpp"

namespace yoonvision {

class ImageBuilder;
class ImageProcessor;

class Image {
 public:
  using Ptr = std::shared_ptr<Image>;
  
  enum class ImageFormat {
    kNone,
    kGray,
    kRgb,
    kRgbParallel,
    kRgbMixed,
    kBgr,
    kBgrParallel,
    kBgrMixed,
    kMaxImageFormat
  };
  enum class FileFormat { kNone, kBitmap, kJpeg, kMaxFileFormat };

  Image();
  ~Image() = default;

  Image(const Image& image) = default;
  Image& operator=(const Image& image) = default;
  Image(Image&& image) = default;
  Image& operator=(Image&& image) = default;

  [[nodiscard]] size_t GetWidth() const;
  [[nodiscard]] size_t GetHeight() const;
  [[nodiscard]] size_t GetChannel() const;
  [[nodiscard]] size_t GetStride() const;
  std::vector<byte>& GetBuffer();
  const std::vector<byte>& GetBuffer() const;

  std::vector<byte> CopyBuffer() const;
  std::vector<byte> GetMixedColorBuffer() const;

  ImageFormat GetImageFormat() const;

  byte& operator()(size_t y, size_t x, size_t c);
  const byte& operator()(size_t y, size_t x, size_t c) const;

  bool operator==(const Image& image) const;
  bool operator!=(const Image& image) const;

  void CopyFrom(const Image& image);
  Image Clone() const;

  bool SaveBitmap(const std::string& path) const;
  bool SaveJpeg(const std::string& path) const;

 private:
  friend class ImageBuilder;  // private 생성자 접근 허용
  friend class ImageProcessor;  // private 생성자 접근 허용

  Image(size_t width, size_t height, size_t channel);
  Image(const std::vector<byte>& buffer, size_t width, size_t height,
        ImageFormat format);

  ImageFormat ChannelToDefaultFormat(size_t channel) const;

  std::vector<byte> ToMixedColorBuffer(const std::vector<byte>& buffer,
                                       bool reverse_order = false) const;
  std::vector<byte> ToParallelColorBuffer(const std::vector<byte>& buffer,
                                          bool reverse_order = false) const;

  size_t width_;
  size_t height_;
  size_t channel_;
  ImageFormat format_;
  std::vector<byte> buffer_;  // "Gray" or Parallel Color Buffers (R + G + B)
};

}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_HPP_
