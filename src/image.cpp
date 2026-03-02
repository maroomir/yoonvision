//
// Created by maroomir on 2021-07-05.
//

#include "yoonvision/image.hpp"

#include "log.hpp"

namespace yoonvision {

Image::Image() { buffer_.resize(width_ * height_ * channel_, 0); }

Image::Image(const std::string &image_path, image::FileFormat format) {
  width_ = image::kDefaultWidth;
  height_ = image::kDefaultHeight;
  channel_ = image::kDefaultChannel;
  format_ = image::ImageFormat::kNone;
  LOG_DEBUG2("ctor from path: %s (file_format=%d)", image_path.c_str(),
             static_cast<int>(format));
  switch (format) {
    case image::FileFormat::kBitmap:
      LoadBitmap(image_path);
      break;
    case image::FileFormat::kJpeg:
      LoadJpeg(image_path);
      break;
    default:
      LOG_WARN("unsupported file format: %d (path=%s)",
               static_cast<int>(format), image_path.c_str());
      break;
  }
}

Image::Image(size_t width, size_t height, size_t channel) {
  width_ = width;
  height_ = height;
  channel_ = channel;
  format_ = image::kChannelToDefaultFormat[channel_];
  buffer_.resize(width_ * height_ * channel_, 0);
}

Image::Image(const std::vector<int> &buffer, size_t width, size_t height) {
  width_ = width;
  height_ = height;
  channel_ = 3;
  format_ = image::kChannelToDefaultFormat[channel_];
  buffer_.resize(width_ * height_ * channel_, 0);
  for (size_t i = 0; i < width * height; i++) {
    std::vector<unsigned char> byte_ptr = ToByte(buffer[i]);
    std::copy(byte_ptr.begin(), byte_ptr.begin() + channel_,
              buffer_.begin() + i * channel_);
  }
}

Image::Image(const std::vector<unsigned char> &red_buffer,
             const std::vector<unsigned char> &green_buffer,
             const std::vector<unsigned char> &blue_buffer, size_t width,
             size_t height) {
  width_ = width;
  height_ = height;
  channel_ = 3;
  format_ = image::ImageFormat::kRgb;
  size_t size = width_ * height_;
  buffer_.resize(size * channel_);
  std::copy(red_buffer.begin(), red_buffer.begin() + size, buffer_.begin());
  std::copy(green_buffer.begin(), green_buffer.begin() + size,
            buffer_.begin() + size);
  std::copy(blue_buffer.begin(), blue_buffer.begin() + size,
            buffer_.begin() + 2 * size);
}

Image::Image(const std::vector<unsigned char> &buffer, size_t width,
             size_t height, image::ImageFormat format) {
  width_ = width;
  height_ = height;
  channel_ = image::kFormatToChannel[static_cast<int>(format)];
  size_t size = width_ * height_ * channel_;
  switch (format) {
    case image::ImageFormat::kGray:
      format_ = image::ImageFormat::kGray;
      buffer_ = buffer;
      break;
    case image::ImageFormat::kRgb:
    case image::ImageFormat::kRgbParallel:
      format_ = image::ImageFormat::kRgb;
      buffer_ = buffer;
      break;
    case image::ImageFormat::kRgbMixed:  // Separate the pixel to Red, Green,
                                         // Blue buffer
      format_ = image::ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer);
      break;
    case image::ImageFormat::kBgr:
    case image::ImageFormat::kBgrParallel:
      format_ = image::ImageFormat::kRgb;
      buffer_.resize(size);
      for (size_t c = 0; c < channel_; c++) {
        size_t start = (channel_ - c - 1) * width_ * height_;
        std::copy(buffer.begin() + start,
                  buffer.begin() + start + width_ * height_,
                  buffer_.begin() + start);
      }
      break;
    case image::ImageFormat::kBgrMixed:
      format_ = image::ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer, true);
      break;
    default:
      LOG_WARN("unsupported image format: %d",
               static_cast<int>(format));
      format_ = image::ImageFormat::kGray;
      buffer_.resize(size, 0);
      break;
  }
}

std::vector<unsigned char> Image::ToParallelColorBuffer(
    const std::vector<unsigned char> &buffer, bool reverse_order) const {
  std::vector<unsigned char> result(width_ * height_ * channel_);
  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = (reverse_order) ? channel_ - c - 1 : c;  // BRG or RGB
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        result[start + y * width_ + x] =
            buffer[y * width_ * channel_ + x * channel_ + color];
      }
    }
  }
  return result;
}

std::vector<unsigned char> Image::ToMixedColorBuffer(
    const std::vector<unsigned char> &buffer, bool reverse_order) const {
  std::vector<unsigned char> result(width_ * height_ * channel_);
  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = (reverse_order) ? channel_ - c - 1 : c;
    for (size_t y = 0; y < height_; y++) {
      for (size_t x = 0; x < width_; x++) {
        result[y * width_ * channel_ + x * channel_ + color] =
            buffer[start + y * width_ + x];
      }
    }
  }
  return result;
}

size_t Image::Width() const { return width_; }
size_t Image::Height() const { return height_; }
size_t Image::Channel() const { return channel_; }
size_t Image::Stride() const { return width_ * channel_; }

std::vector<unsigned char> &Image::GetBuffer() { return buffer_; }
const std::vector<unsigned char> &Image::GetBuffer() const { return buffer_; }

std::vector<unsigned char> Image::CopyBuffer() const { return buffer_; }

image::ImageFormat Image::ImageFormat() const { return format_; }

std::vector<unsigned char> Image::GetMixedColorBuffer() const {
  std::vector<unsigned char> result;
  switch (format_) {
    case image::ImageFormat::kGray: {
      size_t size = width_ * height_;
      result.resize(size * 3);
      for (int c = 0; c < 3; c++) {
        size_t offset = size * c;
        std::copy(buffer_.begin() + offset, buffer_.begin() + offset + size,
                  result.begin() + offset);
      }
      break;
    }
    case image::ImageFormat::kRgb:
      result = ToMixedColorBuffer(buffer_, true);
      break;
    case image::ImageFormat::kBgr:
      result = ToMixedColorBuffer(buffer_, false);
      break;
    default:
      LOG_WARN("unsupported image format for mixed color buffer: %d",
               static_cast<int>(format_));
      break;
  }
  return result;
}

void Image::CopyFrom(const Image &image) {
  width_ = image.width_;
  height_ = image.height_;
  channel_ = image.channel_;
  format_ = image.format_;
  buffer_ = image.buffer_;
}

Image Image::Clone() const { return Image(buffer_, width_, height_, format_); }

bool Image::Equals(const Image &image) const {
  if (width_ != image.width_ || height_ != image.height_ ||
      channel_ != image.channel_ || format_ != image.format_) {
    return false;
  }
  return buffer_ == image.buffer_;
}

std::vector<unsigned char> Image::ToGrayBuffer() const {
  size_t size = width_ * height_;
  std::vector<unsigned char> result(size);
  switch (format_) {
    case image::ImageFormat::kGray:
      result = buffer_;
      break;
    case image::ImageFormat::kRgb:
      for (size_t h = 0; h < height_; h++) {
        for (size_t w = 0; w < width_; w++) {
          size_t pos = h * width_ + w;
          unsigned char red = buffer_[pos];
          unsigned char green = buffer_[size + pos];
          unsigned char blue = buffer_[2 * size + pos];
          result[pos] = static_cast<unsigned char>(0.299 * red + 0.587 * green +
                                                   0.114 * blue);
        }
      }
      break;
    case image::ImageFormat::kBgr:
      for (size_t h = 0; h < height_; h++) {
        for (size_t w = 0; w < width_; w++) {
          size_t pos = h * width_ + w;
          unsigned char blue = buffer_[pos];
          unsigned char green = buffer_[size + pos];
          unsigned char red = buffer_[2 * size + pos];
          result[pos] = static_cast<unsigned char>(0.299 * red + 0.587 * green +
                                                   0.114 * blue);
        }
      }
      break;
    default:
      LOG_WARN("unsupported image format for gray conversion: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToGrayImage() const {
  std::vector<unsigned char> result_buffer = ToGrayBuffer();
  return Image(result_buffer, width_, height_, image::ImageFormat::kGray);
}

std::vector<unsigned char> Image::ToRedBuffer() const {
  size_t size = width_ * height_;
  std::vector<unsigned char> result(size);
  switch (format_) {
    case image::ImageFormat::kGray:
    case image::ImageFormat::kRgb:  // "R" G B
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case image::ImageFormat::kBgr:  // B G "R"
      std::copy(buffer_.begin() + size * 2, buffer_.begin() + size * 3,
                result.begin());
      break;
    default:
      LOG_WARN("unsupported image format for red conversion: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToRedImage() const {
  return Image(ToRedBuffer(), width_, height_, image::ImageFormat::kGray);
}

std::vector<unsigned char> Image::ToGreenBuffer() const {
  size_t size = width_ * height_;
  std::vector<unsigned char> result(size);
  switch (format_) {
    case image::ImageFormat::kGray:
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case image::ImageFormat::kRgb:  // R "G" B
    case image::ImageFormat::kBgr:  // B "G" R
      std::copy(buffer_.begin() + size, buffer_.begin() + size * 2,
                result.begin());
      break;
    default:
      LOG_WARN("unsupported image format for green conversion: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToGreenImage() const {
  return Image(ToGreenBuffer(), width_, height_, image::ImageFormat::kGray);
}

std::vector<unsigned char> Image::ToBlueBuffer() const {
  size_t size = width_ * height_;
  std::vector<unsigned char> result(size);
  switch (format_) {
    case image::ImageFormat::kGray:
    case image::ImageFormat::kBgr:  // "B" G R
      std::copy(buffer_.begin(), buffer_.begin() + size, result.begin());
      break;
    case image::ImageFormat::kRgb:  // R G "B"
      std::copy(buffer_.begin() + size * 2, buffer_.begin() + size * 3,
                result.begin());
      break;
    default:
      LOG_WARN("unsupported image format for blue conversion: %d",
               static_cast<int>(format_));
      std::fill(result.begin(), result.end(), 0);
      break;
  }
  return result;
}

Image Image::ToBlueImage() const {
  return Image(ToBlueBuffer(), width_, height_, image::ImageFormat::kGray);
}

bool Image::LoadBitmap(const std::string &path) {
  LOG_DEBUG2("load bitmap requested: %s", path.c_str());
  std::ifstream stream(path.c_str(), std::ios::binary);
  if (!stream) {
    LOG_ERROR("load bitmap failed: invalid file path");
    return false;
  }

  buffer_.clear();
  width_ = 0;
  height_ = 0;
  channel_ = 0;
  image::bitmap::BitmapFileHeader file_header{};
  image::bitmap::BitmapInfoHeader info_header{};
  file_header.Read(stream);
  info_header.Read(stream);
  if (info_header.size != info_header.HeaderSize()) {
    LOG_ERROR("load bitmap failed: invalid bitmap header size");
    file_header.Clear();
    info_header.Clear();
    stream.close();

    width_ = image::kDefaultWidth;
    height_ = image::kDefaultHeight;
    channel_ = image::kDefaultChannel;
    format_ = image::kChannelToDefaultFormat[channel_];
    buffer_.resize(width_ * height_ * channel_, 0);
    return false;
  }

  width_ = info_header.width;
  height_ = info_header.height;
  channel_ = info_header.bit_count >> 3;  // 00011000 => 00000011
  format_ = image::kChannelToDefaultFormat[channel_];
  try {
    if (format_ == image::ImageFormat::kGray)
      image::bitmap::ReadBitmapPaletteTable(stream);
    std::vector<unsigned char> temp_buffer =
        image::bitmap::ReadBitmapBuffer(stream, width_, height_, channel_);
    buffer_ = ToParallelColorBuffer(temp_buffer, true);
    stream.close();
  } catch (int code) {
    LOG_ERROR("load bitmap failed: buffer reading error (code=%d)",
              code);
    file_header.Clear();
    info_header.Clear();
    stream.close();

    width_ = image::kDefaultWidth;
    height_ = image::kDefaultHeight;
    channel_ = image::kDefaultChannel;
    format_ = image::kChannelToDefaultFormat[channel_];
    buffer_.resize(width_ * height_ * channel_, 0);
    return false;
  }
  LOG_INFO("load bitmap success: %s (%zux%zu, ch=%zu)", path.c_str(),
           width_, height_, channel_);
  return true;
}

bool Image::SaveBitmap(const std::string &path) const {
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
  info_header.bit_count = channel_ << 3;  // 00000011 => 00011000
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
  file_header.type = 0x4D42;  // 0x4D42 (19778, Static Number)
  file_header.size = file_header.HeaderSize() + info_header.HeaderSize() +
                     info_header.buffer_size;
  file_header.reserved1 = 0;
  file_header.reserved2 = 0;
  file_header.off_bits = info_header.HeaderSize() + file_header.HeaderSize();
  if (format_ == image::ImageFormat::kGray)
    file_header.off_bits += sizeof(image::bitmap::RgbquadPalette) * 256;
  file_header.Write(stream);
  info_header.Write(stream);

  size_t size = width_ * height_;
  std::vector<unsigned char> buffer;
  switch (format_) {
    case image::ImageFormat::kGray:
      image::bitmap::WriteBitmapPaletteTable(stream);
      buffer = buffer_;
      break;
    case image::ImageFormat::kRgb:
    case image::ImageFormat::kRgbParallel:
      buffer = ToMixedColorBuffer(buffer_, true);  // TO_BGR_MIXED
      break;
    case image::ImageFormat::kBgr:
    case image::ImageFormat::kBgrParallel:
      buffer = ToMixedColorBuffer(buffer_, false);  // TO_BGR_MIXED
      break;
    case image::ImageFormat::kRgbMixed:
    case image::ImageFormat::kBgrMixed:
      buffer = buffer_;
      break;
    default:
      buffer.resize(size * channel_, 0);
      break;
  }
  image::bitmap::WriteBitmapBuffer(stream, buffer, width_, height_, channel_);
  stream.close();
  LOG_INFO("save bitmap success: %s (%zux%zu, ch=%zu)", path.c_str(),
           width_, height_, channel_);
  return true;
}

bool Image::LoadJpeg(const std::string &path) {
  LOG_DEBUG2("load jpeg requested: %s", path.c_str());
  std::vector<unsigned char> temp_buffer;
  if (image::jpeg::ReadJpegBuffer(path.c_str(), temp_buffer, width_, height_,
                                  channel_)) {
    buffer_ = ToParallelColorBuffer(temp_buffer);
    format_ = image::kChannelToDefaultFormat[channel_];
    LOG_INFO("load jpeg success: %s (%zux%zu, ch=%zu)", path.c_str(),
             width_, height_, channel_);
    return true;
  }
  LOG_ERROR("load jpeg failed: invalid jpeg path");
  return false;
}

bool Image::SaveJpeg(const std::string &path) const {
  LOG_DEBUG2("save jpeg requested: %s", path.c_str());
  std::vector<unsigned char> buffer = (format_ != image::ImageFormat::kRgbMixed)
                                          ? ToMixedColorBuffer(buffer_)
                                          : buffer_;
  if (!image::jpeg::WriteJpegBuffer(path.c_str(), buffer, width_, height_,
                                    (int)channel_)) {
    LOG_ERROR("save jpeg failed");
    return false;
  } else {
    LOG_INFO("save jpeg success: %s (%zux%zu, ch=%zu)", path.c_str(),
             width_, height_, channel_);
    return true;
  }
}

Image Image::GrayPaletteBar(int width, int height, int step) {
  width *= step;
  std::vector<unsigned char> buffer(width * height);
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      buffer[y * width + x] = static_cast<unsigned char>(x / step);
    }
  }
  return Image(buffer, width, height, image::ImageFormat::kGray);
}

Image Image::ColorPaletteBar(int width, int height, int step) {
  width *= step;
  int channel = 3;
  std::vector<unsigned char> red_buffer(width * height * channel);
  std::vector<unsigned char> green_buffer(width * height * channel);
  std::vector<unsigned char> blue_buffer(width * height * channel);
  for (int y = 0; y < height; y++) {
    for (int page = 0; page < channel; page++) {
      for (int x = 0; x < width; x++) {
        int i = y * width * channel + page * width + x;
        switch (page) {
          case 0:  // RED Area
            red_buffer[i] = 255 - static_cast<unsigned char>(x / step);
            green_buffer[i] = static_cast<unsigned char>(x / step);
            blue_buffer[i] = 0;
            break;
          case 1:  // GREEN Area
            red_buffer[i] = 0;
            green_buffer[i] = 255 - static_cast<unsigned char>(x / step);
            blue_buffer[i] = static_cast<unsigned char>(x / step);
            break;
          case 2:  // BLUE Area
            red_buffer[i] = static_cast<unsigned char>(x / step);
            green_buffer[i] = 0;
            blue_buffer[i] = 255 - static_cast<unsigned char>(x / step);
            break;
          default:
            break;
        }
      }
    }
  }
  return Image(red_buffer, green_buffer, blue_buffer, width * channel, height);
}

}  // namespace yoonvision