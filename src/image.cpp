//
// Created by maroomir on 2021-07-05.
//

#include "yoonvision/image.hpp"

namespace yoonvision {

Image::Image() {
  width_ = image::kDefaultWidth;
  height_ = image::kDefaultHeight;
  channel_ = image::kDefaultChannel;
  format_ = image::kChannelToDefaultFormat[channel_];
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  memset(buffer_, 0, sizeof(char) * width_ * height_);
}

Image::Image(const Image &image) {
  width_ = image.width_;
  height_ = image.height_;
  channel_ = image.channel_;
  format_ = image.format_;
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  memcpy(buffer_, image.buffer_, sizeof(char) * width_ * height_ * channel_);
}

Image::Image(const std::string &image_path, image::FileFormat format) {
  width_ = image::kDefaultWidth;
  height_ = image::kDefaultHeight;
  channel_ = image::kDefaultChannel;
  format_ = image::ImageFormat::kNone;
  buffer_ = nullptr;
  switch (format) {
    case image::FileFormat::kBitmap:
      LoadBitmap(image_path);
      break;
    case image::FileFormat::kJpeg:
      LoadJpeg(image_path);
      break;
    default:
      std::printf("[Image] Abnormal File Format (file: %s)\n",
                  image_path.c_str());
      break;
  }
}

Image::Image(size_t width, size_t height, size_t channel) {
  width_ = width;
  height_ = height;
  channel_ = channel;
  format_ = image::kChannelToDefaultFormat[channel_];
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  memset(buffer_, 0, sizeof(char) * width_ * height_ * channel_);
}

Image::Image(int *buffer, size_t width, size_t height) {
  width_ = width;
  height_ = height;
  channel_ = 3;
  format_ = image::kChannelToDefaultFormat[channel_];
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  for (int i = 0; i < width * height; i++) {
    unsigned char *byte_ptr = ToByte(buffer[i]);
    memcpy(buffer_ + i * channel_ * sizeof(char), byte_ptr,
           sizeof(char) * channel_);
    delete byte_ptr;
  }
}

Image::Image(unsigned char *red_buffer, unsigned char *green_buffer,
             unsigned char *blue_buffer, size_t width, size_t height) {
  width_ = width;
  height_ = height;
  channel_ = 3;
  format_ = image::ImageFormat::kRgb;
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  size_t size = width_ * height_;
  size_t cursor = 0;
  memcpy(buffer_, red_buffer, sizeof(char) * size);
  cursor += sizeof(char) * size;
  memcpy(buffer_ + (int)cursor, green_buffer, sizeof(char) * size);
  cursor += sizeof(char) * size;
  memcpy(buffer_ + (int)cursor, blue_buffer, sizeof(char) * size);
}

Image::Image(unsigned char *buffer, size_t width, size_t height,
             image::ImageFormat format) {
  width_ = width;
  height_ = height;
  channel_ = image::kFormatToChannel[static_cast<int>(format)];
  switch (format) {
    case image::ImageFormat::kGray:
      format_ = image::ImageFormat::kGray;
      buffer_ = static_cast<unsigned char *>(
          malloc(sizeof(char) * width_ * height_ * channel_));
      memcpy(buffer_, buffer, sizeof(char) * width_ * height_ * channel_);
      break;
    case image::ImageFormat::kRgb:
    case image::ImageFormat::kRgbParallel:
      format_ = image::ImageFormat::kRgb;
      buffer_ = static_cast<unsigned char *>(
          malloc(sizeof(char) * width_ * height_ * channel_));
      memcpy(buffer_, buffer, sizeof(char) * width_ * height_ * channel_);
      break;
    case image::ImageFormat::kRgbMixed:  // Separate the pixel to Red, Green,
                                         // Blue buffer
      format_ = image::ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer);
      break;
    case image::ImageFormat::kBgr:
    case image::ImageFormat::kBgrParallel:
      format_ = image::ImageFormat::kRgb;
      buffer_ = static_cast<unsigned char *>(
          malloc(sizeof(char) * width_ * height_ * channel_));
      for (size_t c = 0; c < channel_; c++) {
        size_t start = (channel_ - c - 1) * width_ * height_;
        memcpy(buffer_ + sizeof(char) * start, buffer + sizeof(char) * start,
               sizeof(char) * width_ * height_);
      }
      break;
    case image::ImageFormat::kBgrMixed:
      format_ = image::ImageFormat::kRgb;
      buffer_ = ToParallelColorBuffer(buffer, true);
      break;
    default:
      std::printf("[Image] Abnormal Image Format\n");
      format_ = image::ImageFormat::kGray;
      buffer_ = static_cast<unsigned char *>(
          malloc(sizeof(char) * width_ * height_ * channel_));
      memset(buffer_, 0, sizeof(char) * width_ * height_ * channel_);
      break;
  }
}

Image::~Image() {
  if (!buffer_) {
    free(buffer_);
    buffer_ = nullptr;
  }
}

unsigned char *Image::ToParallelColorBuffer(const unsigned char *buffer,
                                            bool reverse_order) const {
  auto *result = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = (reverse_order) ? channel_ - c - 1 : c;  // BRG or RGB
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
        result[start + y * width_ + x] =
            buffer[y * width_ * channel_ + x * channel_ + color];
      }
    }
  }
  return result;
}

unsigned char *Image::ToMixedColorBuffer(const unsigned char *buffer,
                                         bool reverse_order) const {
  auto *result = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));

  for (size_t c = 0; c < channel_; c++) {
    size_t start = c * width_ * height_;
    size_t color = (reverse_order) ? channel_ - c - 1 : c;
    for (int y = 0; y < height_; y++) {
      for (int x = 0; x < width_; x++) {
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

unsigned char *Image::GetBuffer() { return buffer_; }

unsigned char *Image::CopyBuffer() {
  auto *buffer = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  memcpy(buffer, buffer_, sizeof(char) * width_ * height_ * channel_);
  return buffer;
}

image::ImageFormat Image::ImageFormat() { return format_; }

unsigned char *Image::GetMixedColorBuffer() {
  unsigned char *buffer = nullptr;
  switch (format_) {
    case image::ImageFormat::kGray: {
      size_t size = width_ * height_;
      buffer = static_cast<unsigned char *>(malloc(sizeof(char) * size * 3));
      for (int c = 0; c < 3; c++) {
        size_t offset = size * c;
        memcpy(buffer + sizeof(char) * offset, buffer_ + sizeof(char) * offset,
               sizeof(char) * size);
      }
      break;
    }
    case image::ImageFormat::kRgb:
      buffer = ToMixedColorBuffer(buffer_, true);
      break;
    case image::ImageFormat::kBgr:
      buffer = ToMixedColorBuffer(buffer_, false);
      break;
    default:
      std::printf("[Image][GetMixedColorBuffer] Abnormal Image Format\n");
      break;
  }
  return buffer;
}

void Image::CopyFrom(const Image &image) {
  width_ = image.width_;
  height_ = image.height_;
  channel_ = image.channel_;
  format_ = image.format_;
  if (buffer_ != nullptr) {
    free(buffer_);
    buffer_ = nullptr;
  }
  buffer_ = static_cast<unsigned char *>(
      malloc(sizeof(char) * width_ * height_ * channel_));
  memcpy(buffer_, image.buffer_, sizeof(char) * width_ * height_ * channel_);
}

Image *Image::Clone() { return new Image(buffer_, width_, height_, format_); }

bool Image::Equals(const Image &image) {
  bool equal = (width_ == image.width_) && (height_ == image.height_) &&
               (channel_ == image.channel_) && (format_ == image.format_);
  if (equal) {
    size_t size = width_ * height_ * channel_;
    for (size_t i = 0; i < size; i++) {
      if (buffer_[i] != image.buffer_[i]) {
        equal = false;
        break;
      }
    }
  }
  return equal;
}

unsigned char *Image::ToGrayBuffer() {
  size_t size = width_ * height_;
  auto *result = new unsigned char[sizeof(char) * size];
  switch (format_) {
    case image::ImageFormat::kGray:
      memcpy(result, buffer_, sizeof(char) * size);
      break;
    case image::ImageFormat::kRgb:
      for (size_t h = 0; h < height_; h++) {
        for (size_t w = 0; w < width_; w++) {
          size_t pos = h * width_ + w;
          unsigned char red = buffer_[pos];
          unsigned char green = buffer_[size + pos];
          unsigned char blue = buffer_[2 * size + pos];
          // ITU-RBT.709, YPrPb
          result[pos] =
              (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);
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
          // ITU-RBT.709, YPrPb
          result[pos] =
              (unsigned char)(0.299 * red + 0.587 * green + 0.114 * blue);
        }
      }
      break;
    default:
      std::printf("[Image][ToGrayImage] Abnormal Image Format\n");
      memset(result, 0, sizeof(char) * size);
      break;
  }
  return result;
}

Image *Image::ToGrayImage() {
  unsigned char *result_buffer = ToGrayBuffer();
  auto *result_image =
      new Image(result_buffer, width_, height_, image::ImageFormat::kGray);
  delete[] result_buffer;
  return result_image;
}

unsigned char *Image::ToRedBuffer() {
  size_t size = width_ * height_;
  auto *result = new unsigned char[sizeof(char) * size];
  switch (format_) {
    case image::ImageFormat::kGray:
    case image::ImageFormat::kRgb:  // "R" G B
      memcpy(result, buffer_, sizeof(char) * size);
      break;
    case image::ImageFormat::kBgr:  // B G "R"
      memcpy(result, buffer_ + sizeof(char) * size * 2, sizeof(char) * size);
      break;
    default:
      std::printf("[Image][ToRedImage] Abnormal Image Format\n");
      memset(result, 0, sizeof(char) * size);
      break;
  }
  return result;
}

Image *Image::ToRedImage() {
  unsigned char *result_buffer = ToRedBuffer();
  auto *result_image =
      new Image(result_buffer, width_, height_, image::ImageFormat::kGray);
  delete[] result_buffer;
  return result_image;
}

unsigned char *Image::ToGreenBuffer() {
  size_t size = width_ * height_;
  auto *result_buffer = new unsigned char[sizeof(char) * size];
  switch (format_) {
    case image::ImageFormat::kGray:
      memcpy(result_buffer, buffer_, sizeof(char) * size);
      break;
    case image::ImageFormat::kRgb:  // R "G" B
    case image::ImageFormat::kBgr:  // B "G" R
      memcpy(result_buffer, buffer_ + sizeof(char) * size, sizeof(char) * size);
      break;
    default:
      std::printf("[Image][ToGreenImage] Abnormal Image Format\n");
      memset(result_buffer, 0, sizeof(char) * size);
      break;
  }
  return result_buffer;
}

Image *Image::ToGreenImage() {
  unsigned char *result_buffer = ToGreenBuffer();
  auto *result_image =
      new Image(result_buffer, width_, height_, image::ImageFormat::kGray);
  delete[] result_buffer;
  return result_image;
}

unsigned char *Image::ToBlueBuffer() {
  size_t size = width_ * height_;
  auto *result_buffer = new unsigned char[sizeof(char) * size];
  switch (format_) {
    case image::ImageFormat::kGray:
    case image::ImageFormat::kBgr:  // "B" G R
      memcpy(result_buffer, buffer_, sizeof(char) * size);
      break;
    case image::ImageFormat::kRgb:  // R G "B"
      memcpy(result_buffer, buffer_ + sizeof(char) * size * 2,
             sizeof(char) * size);
      break;
    default:
      std::printf("[Image][ToRedImage] Abnormal Image Format\n");
      memset(result_buffer, 0, sizeof(char) * size);
      break;
  }
  return result_buffer;
}

Image *Image::ToBlueImage() {
  unsigned char *result_buffer = ToBlueBuffer();
  auto *result_image =
      new Image(result_buffer, width_, height_, image::ImageFormat::kGray);
  delete[] result_buffer;
  return result_image;
}

bool Image::LoadBitmap(const std::string &path) {
  std::ifstream stream(path.c_str(), std::ios::binary);
  if (!stream) {
    std::printf("[Image][LoadBitmap] File Path is not correct\n");
    return false;
  }

  if (!buffer_) {
    free(buffer_);
    buffer_ = nullptr;
  }
  width_ = 0;
  height_ = 0;
  channel_ = 0;
  image::bitmap::BitmapFileHeader file_header{};
  image::bitmap::BitmapInfoHeader info_header{};
  file_header.Read(stream);
  info_header.Read(stream);
  if (info_header.size != info_header.HeaderSize()) {
    std::printf("[Image][LoadBitmap] Invalid Bitmap Size\n");
    file_header.Clear();
    info_header.Clear();
    stream.close();
    // Initialize the buffer
    width_ = image::kDefaultWidth;
    height_ = image::kDefaultHeight;
    channel_ = image::kDefaultChannel;
    format_ = image::kChannelToDefaultFormat[channel_];
    buffer_ = static_cast<unsigned char *>(
        malloc(sizeof(char) * width_ * height_ * channel_));
    memset(buffer_, 0, sizeof(char) * width_ * height_);
    return false;
  }

  width_ = info_header.width;
  height_ = info_header.height;
  channel_ = info_header.bit_count >> 3;  // 00011000 => 00000011
  format_ = image::kChannelToDefaultFormat[channel_];
  try {
    if (format_ == image::ImageFormat::kGray)
      image::bitmap::ReadBitmapPaletteTable(stream);
    unsigned char *buffer =
        image::bitmap::ReadBitmapBuffer(stream, width_, height_, channel_);
    buffer_ = ToParallelColorBuffer(buffer, true);
    delete[] buffer;
    stream.close();
  } catch (int code) {
    std::printf("[Image][LoadBitmap] Buffer Reading Error\n");
    file_header.Clear();
    info_header.Clear();
    stream.close();
    // Initialize the buffer
    width_ = image::kDefaultWidth;
    height_ = image::kDefaultHeight;
    channel_ = image::kDefaultChannel;
    format_ = image::kChannelToDefaultFormat[channel_];
    buffer_ = static_cast<unsigned char *>(
        malloc(sizeof(char) * width_ * height_ * channel_));
    memset(buffer_, 0, sizeof(char) * width_ * height_);
    return false;
  }
  return true;
}

bool Image::SaveBitmap(const std::string &path) {
  std::ofstream stream(path.c_str(), std::ios::binary);
  if (!stream) {
    std::printf("[Image][SaveBitmap] File Path is not correct\n");
    return false;
  }
  if (!buffer_) {
    std::printf("[Image][SaveBitmap] Buffer is empty\n");
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
  unsigned char *buffer;
  switch (format_) {
    case image::ImageFormat::kGray:
      // Palette Table
      image::bitmap::WriteBitmapPaletteTable(stream);
      // Pixel Buffer
      buffer = new unsigned char[sizeof(char) * size];
      memcpy(buffer, buffer_, sizeof(char) * size);
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
      buffer = new unsigned char[size * channel_];
      memcpy(buffer, buffer_, sizeof(char) * size * channel_);
      break;
    default:
      buffer = new unsigned char[size * channel_];
      memset(buffer, 0, sizeof(char) * size * channel_);
      break;
  }
  image::bitmap::WriteBitmapBuffer(stream, buffer, width_, height_, channel_);
  stream.close();
  delete[] buffer;
  return true;
}

bool Image::LoadJpeg(const std::string &path) {
  // buffer 는 mixed color buffer 로 읽어옴
  unsigned char *buffer = nullptr;
  if (image::jpeg::ReadJpegBuffer(path.c_str(), buffer, width_, height_,
                                  channel_)) {
    buffer_ = ToParallelColorBuffer(buffer);
    format_ = image::kChannelToDefaultFormat[channel_];
    return true;
  }
  std::printf("[Image][LoadJpeg] Invalid jpeg path\n");
  return false;
}

bool Image::SaveJpeg(const std::string &path) {
  // jpeg 로 save 할 때는 mixed color buffer 사용해야함
  unsigned char *buffer = (format_ != image::ImageFormat::kRgbMixed)
                              ? ToMixedColorBuffer(buffer_)
                              : buffer_;
  if (!image::jpeg::WriteJpegBuffer(path.c_str(), buffer, width_, height_,
                                    (int)channel_)) {
    std::printf("[Image][SaveJpeg] Jpeg saved failed\n");
    return false;
  } else {
    return true;
  }
}

Image *Image::GrayPaletteBar(int width, int height, int step) {
  width *= step;
  auto *buffer = new unsigned char[width * height];
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      buffer[y * width + x] = (unsigned char)(x / step);
    }
  }
  auto *result_image =
      new Image(buffer, width, height, image::ImageFormat::kGray);
  delete[] buffer;
  return result_image;
}

Image *Image::ColorPaletteBar(int width, int height, int step) {
  width *= step;
  int channel = 3;
  auto *red_buffer = new unsigned char[width * height * channel];
  auto *green_buffer = new unsigned char[width * height * channel];
  auto *blue_buffer = new unsigned char[width * height * channel];
  for (int y = 0; y < height; y++) {
    for (int page = 0; page < channel; page++) {
      for (int x = 0; x < width; x++) {
        int i = y * width * channel + page * width + x;
        switch (page) {
          case 0:  // RED Area
            red_buffer[i] = 255 - (unsigned char)(x / step);
            green_buffer[i] = (unsigned char)(x / step);
            blue_buffer[i] = 0;
            break;
          case 1:  // GREEN Area
            red_buffer[i] = 0;
            green_buffer[i] = 255 - (unsigned char)(x / step);
            blue_buffer[i] = (unsigned char)(x / step);
            break;
          case 2:  // BLUE Area
            red_buffer[i] = (unsigned char)(x / step);
            green_buffer[i] = 0;
            blue_buffer[i] = 255 - (unsigned char)(x / step);
            break;
          default:
            break;
        }
      }
    }
  }
  auto *result_image =
      new Image(red_buffer, green_buffer, blue_buffer, width * channel, height);
  delete[] red_buffer;
  delete[] green_buffer;
  delete[] blue_buffer;
  return result_image;
}
}  // namespace yoonvision
