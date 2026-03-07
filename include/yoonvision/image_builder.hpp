//
// Created by maroomir on 2026-03-07.
//

#ifndef YOONVISION_IMAGE_BUILDER_HPP_
#define YOONVISION_IMAGE_BUILDER_HPP_

#include <string>
#include <vector>

#include "byte.hpp"
#include "image.hpp"

namespace yoonvision {

class ImageBuilder {
 public:
  ImageBuilder() = default;
  ~ImageBuilder() = default;

  ImageBuilder& WithSize(size_t width, size_t height, size_t channel);

  ImageBuilder& FromBuffer(const std::vector<byte>& buffer,
                           size_t width, size_t height,
                           Image::ImageFormat format);
  ImageBuilder& FromIntBuffer(const std::vector<int>& buffer,
                              size_t width, size_t height);
  ImageBuilder& FromRgbChannels(const std::vector<byte>& red_buffer,
                                const std::vector<byte>& green_buffer,
                                const std::vector<byte>& blue_buffer,
                                size_t width, size_t height);
  ImageBuilder& FromFile(const std::string& path, Image::FileFormat format);

  [[nodiscard]] Image Build() const;

  static Image GrayPaletteBar(int width = 256, int height = 50, int step = 10);
  static Image ColorPaletteBar(int width = 256, int height = 50, int step = 10);

 private:
  enum class SourceType {
    kNone,
    kSize,
    kBuffer,
    kIntBuffer,
    kRgbChannels,
    kFile,
  };

  SourceType source_type_{SourceType::kNone};

  size_t width_{0};
  size_t height_{0};
  size_t channel_{0};
  std::vector<byte> buffer_{};
  Image::ImageFormat format_{Image::ImageFormat::kNone};

  std::vector<int> int_buffer_{};

  std::vector<byte> red_buffer_{};
  std::vector<byte> green_buffer_{};
  std::vector<byte> blue_buffer_{};

  std::string file_path_{};
  Image::FileFormat file_format_{Image::FileFormat::kNone};
};

}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_BUILDER_HPP_
