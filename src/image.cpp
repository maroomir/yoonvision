//
// Created by maroomir on 2021-07-05.
//

#include "yoonvision/image.hpp"

#include "log.hpp"
#include "yoonvision/bitmap.hpp"
#include "yoonvision/jpeg.hpp"

namespace yoonvision {
namespace {
constexpr size_t kDefaultChannel = 1;
constexpr size_t kDefaultWidth = 640;
constexpr size_t kDefaultHeight = 480;
constexpr int kMaxChannel = 3;

constexpr int
    kFormatToChannel[static_cast<int>(Image::ImageFormat::kMaxImageFormat)] = {
        0,  // kNone    (invalid)
        1,  // kGray
        3,  // kRgb
        3,  // kRgbParallel
        3,  // kRgbMixed
        3,  // kBgr
        3,  // kBgrParallel
        3   // kBgrMixed
};

constexpr Image::ImageFormat kChannelToDefaultFormat[kMaxChannel + 1] = {
    Image::ImageFormat::kNone,  // 0-channel (invalid)
    Image::ImageFormat::kGray,  // 1-channel
    Image::ImageFormat::kNone,  // 2-channel (unsupported)
    Image::ImageFormat::kRgb    // 3-channel
};
}  // namespace

Image::ImageFormat Image::ChannelToDefaultFormat(size_t channel) const {
  if (channel > static_cast<size_t>(kMaxChannel)) {
    return ImageFormat::kGray;
  }
  return kChannelToDefaultFormat[channel];
}

Image::Image()
    : width_(kDefaultWidth),
      height_(kDefaultHeight),
      channel_(kDefaultChannel),
      format_(ChannelToDefaultFormat(kDefaultChannel)),
      buffer_(kDefaultWidth * kDefaultHeight * kDefaultChannel, 0) {}

Image::Image(size_t width, size_t height, size_t channel)
    : width_(width),
      height_(height),
      channel_(channel),
      format_(ChannelToDefaultFormat(channel)),
      buffer_(width * height * channel, 0) {}

Image::Image(const std::vector<byte>& buffer, size_t width, size_t height,
             ImageFormat format)
    : width_(width), height_(height) {
  const int format_index = static_cast<int>(format);
  if (format_index < 0 ||
      format_index >= static_cast<int>(ImageFormat::kMaxImageFormat)) {
    channel_ = kDefaultChannel;
  } else {
    channel_ = static_cast<size_t>(kFormatToChannel[format_index]);
  }
  size_t size = width_ * height_ * channel_;
  switch (format) {
    case ImageFormat::kGray:
      format_ = ImageFormat::kGray;
      buffer_ = buffer;
      break;
    case ImageFormat::kRgb:
    case ImageFormat::kRgbParallel:
      format_ = ImageFormat::kRgb;
      buffer_ = buffer;
      break;
    case ImageFormat::kRgbMixed:
      format_ = ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer);
      break;
    case ImageFormat::kBgr:
    case ImageFormat::kBgrParallel:
      format_ = ImageFormat::kRgb;
      buffer_.resize(size);
      for (size_t c = 0; c < channel_; c++) {
        size_t start = (channel_ - c - 1) * width_ * height_;
        std::copy(buffer.begin() + start,
                  buffer.begin() + start + width_ * height_,
                  buffer_.begin() + start);
      }
      break;
    case ImageFormat::kBgrMixed:
      format_ = ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer, true);
      break;
    default:
      LOG_WARN("Image ctor: unsupported image format: %d",
               static_cast<int>(format));
      format_ = ImageFormat::kGray;
      buffer_.resize(size, 0);
      break;
  }
}

std::vector<byte> Image::ToParallelColorBuffer(const std::vector<byte>& buffer,
                                               bool reverse_order) const {
  std::vector<byte> result(width_ * height_ * channel_);
  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = reverse_order ? channel_ - c - 1 : c;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        result[start + y * width_ + x] =
            buffer[y * width_ * channel_ + x * channel_ + color];
      }
    }
  }
  return result;
}

std::vector<byte> Image::ToMixedColorBuffer(const std::vector<byte>& buffer,
                                            bool reverse_order) const {
  std::vector<byte> result(width_ * height_ * channel_);
  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = reverse_order ? channel_ - c - 1 : c;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        result[y * width_ * channel_ + x * channel_ + color] =
            buffer[start + y * width_ + x];
      }
    }
  }
  return result;
}

size_t Image::GetWidth() const { return width_; }
size_t Image::GetHeight() const { return height_; }
size_t Image::GetChannel() const { return channel_; }
size_t Image::GetStride() const { return width_ * channel_; }
std::vector<byte>& Image::GetBuffer() { return buffer_; }
const std::vector<byte>& Image::GetBuffer() const { return buffer_; }

std::vector<byte> Image::CopyBuffer() const { return buffer_; }

Image::ImageFormat Image::GetImageFormat() const { return format_; }

std::vector<byte> Image::GetMixedColorBuffer() const {
  std::vector<byte> result;
  switch (format_) {
    case Image::ImageFormat::kGray: {
      size_t size = width_ * height_;
      result.resize(size * 3);
      for (int c = 0; c < 3; c++) {
        size_t offset = size * c;
        std::copy(buffer_.begin() + offset, buffer_.begin() + offset + size,
                  result.begin() + offset);
      }
      break;
    }
    case Image::ImageFormat::kRgb:
      result = ToMixedColorBuffer(buffer_, true);  // RGB→BGR Mixed
      break;
    case Image::ImageFormat::kBgr:
      result = ToMixedColorBuffer(buffer_, false);  // BGR→BGR Mixed
      break;
    default:
      LOG_WARN("GetMixedColorBuffer: unsupported format: %d",
               static_cast<int>(format_));
      break;
  }
  return result;
}

void Image::CopyFrom(const Image& image) {
  width_ = image.width_;
  height_ = image.height_;
  channel_ = image.channel_;
  format_ = image.format_;
  buffer_ = image.buffer_;
}

Image Image::Clone() const {
  return Image(buffer_, width_, height_, format_);
}

bool Image::Equals(const Image& image) const {
  if (width_ != image.width_ || height_ != image.height_ ||
      channel_ != image.channel_ || format_ != image.format_) {
    return false;
  }
  return buffer_ == image.buffer_;
}

std::vector<byte> Image::ToGrayBuffer() const {
  size_t size = width_ * height_;
  std::vector<byte> result(size);
  switch (format_) {
    case Image::ImageFormat::kGray:
      result = buffer_;
      break;
    case Image::ImageFormat::kRgb:
      for (size_t h = 0; h < height_; h++) {
        for (size_t w = 0; w < width_; w++) {
          size_t pos = h * width_ + w;
          byte red = buffer_[pos];
          byte green = buffer_[size + pos];
          byte blue = buffer_[2 * size + pos];
          result[pos] =
              static_cast<byte>(0.299 * red + 0.587 * green + 0.114 * blue);
        }
      }
      break;
    case Image::ImageFormat::kBgr:
      for (size_t h = 0; h < height_; h++) {
        for (size_t w = 0; w < width_; w++) {
          size_t pos = h * width_ + w;
          byte blue = buffer_[pos];
          byte green = buffer_[size + pos];
          byte red = buffer_[2 * size + pos];
          result[pos] =
              static_cast<byte>(0.299 * red + 0.587 * green + 0.114 * blue);
        }
      }
      break;
    default:
      LOG_WARN("ToGrayBuffer: unsupported format: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToGrayImage() const {
  return Image(ToGrayBuffer(), width_, height_, Image::ImageFormat::kGray);
}

std::vector<byte> Image::ToRedBuffer() const {
  size_t size = width_ * height_;
  std::vector<byte> result(size);
  switch (format_) {
    case Image::ImageFormat::kGray:
    case Image::ImageFormat::kRgb:  // "R" G B
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case Image::ImageFormat::kBgr:  // B G "R"
      std::copy(buffer_.begin() + size * 2, buffer_.begin() + size * 3,
                result.begin());
      break;
    default:
      LOG_WARN("ToRedBuffer: unsupported format: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToRedImage() const {
  return Image(ToRedBuffer(), width_, height_, ImageFormat::kGray);
}

std::vector<byte> Image::ToGreenBuffer() const {
  size_t size = width_ * height_;
  std::vector<byte> result(size);
  switch (format_) {
    case Image::ImageFormat::kGray:
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case Image::ImageFormat::kRgb:  // R "G" B
    case Image::ImageFormat::kBgr:  // B "G" R
      std::copy(buffer_.begin() + size, buffer_.begin() + size * 2,
                result.begin());
      break;
    default:
      LOG_WARN("ToGreenBuffer: unsupported format: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToGreenImage() const {
  return Image(ToGreenBuffer(), width_, height_, ImageFormat::kGray);
}

std::vector<byte> Image::ToBlueBuffer() const {
  size_t size = width_ * height_;
  std::vector<byte> result(size);
  switch (format_) {
    case Image::ImageFormat::kGray:
    case Image::ImageFormat::kBgr:  // "B" G R
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case Image::ImageFormat::kRgb:  // R G "B"
      std::copy(buffer_.begin() + size * 2, buffer_.begin() + size * 3,
                result.begin());
      break;
    default:
      LOG_WARN("ToBlueBuffer: unsupported format: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToBlueImage() const {
  return Image(ToBlueBuffer(), width_, height_, ImageFormat::kGray);
}

bool Image::SaveBitmap(const std::string& path) const {
  LOG_DEBUG2("save bitmap requested: %s", path.c_str());
  std::ofstream stream(path.c_str(), std::ios::binary);
  if (!stream) {
    LOG_ERROR("save bitmap failed: invalid file path");
    return false;
  }
  if (buffer_.empty()) {
    LOG_ERROR("save bitmap failed: buffer is empty");
    return false;
  }

  image::bitmap::BitmapInfoHeader info_header{};
  info_header.width = width_;
  info_header.height = height_;
  info_header.bit_count = channel_ << 3;
  info_header.compression = 0;
  info_header.planes = 1;
  info_header.size = info_header.HeaderSize();
  info_header.xpels_per_meter = 0;
  info_header.ypels_per_meter = 0;
  info_header.buffer_size =
      (((info_header.width * channel_) + 3) & 0x0000FFFC) * info_header.height;
  info_header.important_color = 0;
  info_header.used_color = 0;

  image::bitmap::BitmapFileHeader file_header{};
  file_header.type = 0x4D42;
  file_header.size = file_header.HeaderSize() + info_header.HeaderSize() +
                     info_header.buffer_size;
  file_header.reserved1 = 0;
  file_header.reserved2 = 0;
  file_header.off_bits = info_header.HeaderSize() + file_header.HeaderSize();
  if (format_ == ImageFormat::kGray)
    file_header.off_bits += sizeof(image::bitmap::RgbquadPalette) * 256;

  file_header.Write(stream);
  info_header.Write(stream);

  std::vector<byte> buffer;
  switch (format_) {
    case Image::ImageFormat::kGray:
      image::bitmap::WriteBitmapPaletteTable(stream);
      buffer = buffer_;
      break;
    case Image::ImageFormat::kRgb:
    case Image::ImageFormat::kRgbParallel:
      buffer = ToMixedColorBuffer(buffer_, true);   // TO_BGR_MIXED
      break;
    case Image::ImageFormat::kBgr:
    case Image::ImageFormat::kBgrParallel:
      buffer = ToMixedColorBuffer(buffer_, false);  // TO_BGR_MIXED
      break;
    case Image::ImageFormat::kRgbMixed:
    case Image::ImageFormat::kBgrMixed:
      buffer = buffer_;
      break;
    default:
      buffer.resize(width_ * height_ * channel_, 0);
      break;
  }
  image::bitmap::WriteBitmapBuffer(stream, buffer, width_, height_, channel_);
  stream.close();
  LOG_INFO("save bitmap success: %s (%zux%zu, ch=%zu)", path.c_str(), width_,
           height_, channel_);
  return true;
}

bool Image::SaveJpeg(const std::string& path) const {
  LOG_DEBUG2("save jpeg requested: %s", path.c_str());
  std::vector<byte> buffer = (format_ != Image::ImageFormat::kRgbMixed)
                                 ? ToMixedColorBuffer(buffer_)
                                 : buffer_;
  if (!image::jpeg::WriteJpegBuffer(path.c_str(), buffer, width_, height_,
                                    static_cast<int>(channel_))) {
    LOG_ERROR("save jpeg failed");
    return false;
  }
  LOG_INFO("save jpeg success: %s (%zux%zu, ch=%zu)", path.c_str(), width_,
           height_, channel_);
  return true;
}

}  // namespace yoonvision
