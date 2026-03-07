//
// Created by maroomir on 2026-03-07.
//

#include "yoonvision/image_builder.hpp"

#include "log.hpp"
#include "yoonvision/bitmap.hpp"
#include "yoonvision/jpeg.hpp"

namespace yoonvision {

ImageBuilder& ImageBuilder::WithSize(size_t width, size_t height,
                                     size_t channel) {
  source_type_ = SourceType::kSize;
  width_ = width;
  height_ = height;
  channel_ = channel;
  return *this;
}

ImageBuilder& ImageBuilder::FromBuffer(const std::vector<byte>& buffer,
                                       size_t width, size_t height,
                                       Image::ImageFormat format) {
  source_type_ = SourceType::kBuffer;
  buffer_ = buffer;
  width_ = width;
  height_ = height;
  format_ = format;
  return *this;
}

ImageBuilder& ImageBuilder::FromIntBuffer(const std::vector<int>& buffer,
                                          size_t width, size_t height) {
  source_type_ = SourceType::kIntBuffer;
  int_buffer_ = buffer;
  width_ = width;
  height_ = height;
  return *this;
}

ImageBuilder& ImageBuilder::FromRgbChannels(
    const std::vector<byte>& red_buffer,
    const std::vector<byte>& green_buffer,
    const std::vector<byte>& blue_buffer, size_t width, size_t height) {
  source_type_ = SourceType::kRgbChannels;
  red_buffer_ = red_buffer;
  green_buffer_ = green_buffer;
  blue_buffer_ = blue_buffer;
  width_ = width;
  height_ = height;
  return *this;
}

ImageBuilder& ImageBuilder::FromFile(const std::string& path,
                                     Image::FileFormat format) {
  source_type_ = SourceType::kFile;
  file_path_ = path;
  file_format_ = format;
  return *this;
}

Image ImageBuilder::Build() const {
  switch (source_type_) {
    case SourceType::kSize:
      return Image(width_, height_, channel_);

    case SourceType::kBuffer:
      return Image(buffer_, width_, height_, format_);

    case SourceType::kIntBuffer: {
      constexpr size_t kChannel = 3;
      Image image(width_, height_, kChannel);
      std::vector<byte>& dst = image.GetBuffer();
      for (size_t i = 0; i < width_ * height_; i++) {
        std::vector<byte> bytes = ToByte(int_buffer_[i]);
        std::copy(bytes.begin(), bytes.begin() + kChannel,
                  dst.begin() + i * kChannel);
      }
      return image;
    }

    case SourceType::kRgbChannels: {
      constexpr size_t kChannel = 3;
      size_t size = width_ * height_;
      std::vector<byte> parallel_buffer(size * kChannel);
      std::copy(red_buffer_.begin(), red_buffer_.begin() + size,
                parallel_buffer.begin());
      std::copy(green_buffer_.begin(), green_buffer_.begin() + size,
                parallel_buffer.begin() + size);
      std::copy(blue_buffer_.begin(), blue_buffer_.begin() + size,
                parallel_buffer.begin() + 2 * size);
      return Image(parallel_buffer, width_, height_, Image::ImageFormat::kRgb);
    }

    case SourceType::kFile: {
      switch (file_format_) {
        case Image::FileFormat::kBitmap: {
          std::ifstream stream(file_path_.c_str(), std::ios::binary);
          if (!stream) {
            LOG_ERROR("ImageBuilder::Build: bitmap load failed (path=%s)",
                      file_path_.c_str());
            return Image();
          }
          image::bitmap::BitmapFileHeader file_header{};
          image::bitmap::BitmapInfoHeader info_header{};
          file_header.Read(stream);
          info_header.Read(stream);
          if (info_header.size != info_header.HeaderSize()) {
            LOG_ERROR("ImageBuilder::Build: invalid bitmap header size");
            stream.close();
            return Image();
          }
          size_t width = info_header.width;
          size_t height = info_header.height;
          size_t channel = info_header.bit_count >> 3;
          Image::ImageFormat format =
              (channel == 1) ? Image::ImageFormat::kGray
                             : Image::ImageFormat::kRgb;
          try {
            if (format == Image::ImageFormat::kGray)
              image::bitmap::ReadBitmapPaletteTable(stream);
            std::vector<byte> temp_buffer =
                image::bitmap::ReadBitmapBuffer(stream, width, height, channel);
            stream.close();
            // BGR Mixed → RGB Parallel 변환을 위해 kBgrMixed 로 지정
            return Image(temp_buffer, width, height,
                         Image::ImageFormat::kBgrMixed);
          } catch (int code) {
            LOG_ERROR(
                "ImageBuilder::Build: bitmap buffer reading error (code=%d)",
                code);
            stream.close();
            return Image();
          }
        }

        case Image::FileFormat::kJpeg: {
          std::vector<byte> temp_buffer;
          size_t width = 0, height = 0, channel = 0;
          if (!image::jpeg::ReadJpegBuffer(file_path_.c_str(), temp_buffer,
                                           width, height, channel)) {
            LOG_ERROR("ImageBuilder::Build: jpeg load failed (path=%s)",
                      file_path_.c_str());
            return Image();
          }
          // JPEG는 RGB Mixed 포맷으로 읽힘 → RGB Parallel로 변환
          return Image(temp_buffer, width, height,
                       Image::ImageFormat::kRgbMixed);
        }

        default:
          LOG_WARN("ImageBuilder::Build: unsupported file format (%d)",
                   static_cast<int>(file_format_));
          return Image();
      }
    }

    default:
      LOG_WARN("ImageBuilder::Build: no source specified, returning default");
      return Image();
  }
}

Image ImageBuilder::GrayPaletteBar(int width, int height, int step) {
  int actual_width = width * step;
  std::vector<byte> buffer(actual_width * height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < actual_width; x++) {
      buffer[y * actual_width + x] = static_cast<byte>(x / step);
    }
  }
  return Image(buffer, actual_width, height, Image::ImageFormat::kGray);
}

Image ImageBuilder::ColorPaletteBar(int width, int height, int step) {
  int actual_width = width * step;
  constexpr int kChannel = 3;
  std::vector<byte> red_buf(actual_width * height * kChannel);
  std::vector<byte> green_buf(actual_width * height * kChannel);
  std::vector<byte> blue_buf(actual_width * height * kChannel);
  for (int y = 0; y < height; y++) {
    for (int page = 0; page < kChannel; page++) {
      for (int x = 0; x < actual_width; x++) {
        int i = y * actual_width * kChannel + page * actual_width + x;
        switch (page) {
          case 0:  // RED Area
            red_buf[i] = 255 - static_cast<byte>(x / step);
            green_buf[i] = static_cast<byte>(x / step);
            blue_buf[i] = 0;
            break;
          case 1:  // GREEN Area
            red_buf[i] = 0;
            green_buf[i] = 255 - static_cast<byte>(x / step);
            blue_buf[i] = static_cast<byte>(x / step);
            break;
          case 2:  // BLUE Area
            red_buf[i] = static_cast<byte>(x / step);
            green_buf[i] = 0;
            blue_buf[i] = 255 - static_cast<byte>(x / step);
            break;
          default:
            break;
        }
      }
    }
  }
  return ImageBuilder()
      .FromRgbChannels(red_buf, green_buf, blue_buf, actual_width * kChannel,
                       height)
      .Build();
}

}  // namespace yoonvision
