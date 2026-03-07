//
// Created by 윤철중 on 2021/08/17.
//

#ifndef YOONVISION_BITMAP_HPP_
#define YOONVISION_BITMAP_HPP_

#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "byte.hpp"

namespace yoonvision::image::bitmap {

// Save the MSB(Most Significant Bit) first
inline bool IsBigEndian() {
  std::uint32_t value = 0x01;
  return (1 != reinterpret_cast<char *>(&value)[0]);  // RISC CPU
}

inline std::uint16_t FlipOrder(const std::uint16_t &value) {
  return static_cast<std::uint16_t>((value >> 8) | (value << 8));
}

inline std::uint32_t FlipOrder(const std::uint32_t &value) {
  return (((value & 0xFF000000) >> 24) | ((value & 0x000000FF) << 24) |
          ((value & 0x00FF0000) >> 8) | ((value & 0x0000FF00) << 8));
}

inline size_t GetFileSize(const std::string &path) {
  std::ifstream stream(path.c_str(), std::ios::in | std::ios::binary);
  if (!stream) return 0;
  stream.seekg(0, std::ios::end);
  return static_cast<size_t>(stream.tellg());
}

template <typename T>
void ReadStream(std::ifstream &stream, T &value) {
  stream.read(reinterpret_cast<char *>(&value), sizeof(T));
}

template <typename T>
void WriteStream(std::ofstream &stream, const T &value) {
  stream.write(reinterpret_cast<const char *>(&value), sizeof(T));
}

struct BitmapFileHeader {
  std::uint16_t type;
  std::uint32_t size;
  std::uint16_t reserved1;
  std::uint16_t reserved2;
  std::uint32_t off_bits;

  [[nodiscard]] std::uint32_t HeaderSize() const {
    return static_cast<std::uint32_t>(
        sizeof(type) + sizeof(size) + sizeof(reserved1) + sizeof(reserved2) +
        sizeof(off_bits));
  }

  void Read(std::ifstream &stream) {
    ReadStream(stream, type);
    ReadStream(stream, size);
    ReadStream(stream, reserved1);
    ReadStream(stream, reserved2);
    ReadStream(stream, off_bits);
    if (IsBigEndian()) {
      type = FlipOrder(type);
      size = FlipOrder(size);
      reserved1 = FlipOrder(reserved1);
      reserved2 = FlipOrder(reserved2);
      off_bits = FlipOrder(off_bits);
    }
  }

  void Write(std::ofstream &stream) const {
    if (IsBigEndian()) {
      WriteStream(stream, FlipOrder(type));
      WriteStream(stream, FlipOrder(size));
      WriteStream(stream, FlipOrder(reserved1));
      WriteStream(stream, FlipOrder(reserved2));
      WriteStream(stream, FlipOrder(off_bits));
    } else {
      WriteStream(stream, type);
      WriteStream(stream, size);
      WriteStream(stream, reserved1);
      WriteStream(stream, reserved2);
      WriteStream(stream, off_bits);
    }
  }

  void Clear() { memset(this, 0x00, sizeof(BitmapFileHeader)); }
};

struct BitmapInfoHeader {
  std::uint32_t size;
  std::uint32_t width;
  std::uint32_t height;
  std::uint16_t planes;
  std::uint16_t bit_count;
  std::uint32_t compression;
  std::uint32_t buffer_size;
  std::uint32_t xpels_per_meter;
  std::uint32_t ypels_per_meter;
  std::uint32_t used_color;
  std::uint32_t important_color;

  [[nodiscard]] std::uint32_t HeaderSize() const {
    return static_cast<std::uint32_t>(
        sizeof(size) + sizeof(width) + sizeof(height) + sizeof(planes) +
        sizeof(bit_count) + sizeof(compression) + sizeof(buffer_size) +
        sizeof(xpels_per_meter) + sizeof(ypels_per_meter) +
        sizeof(used_color) + sizeof(important_color));
  }

  void Read(std::ifstream &stream) {
    ReadStream(stream, size);
    ReadStream(stream, width);
    ReadStream(stream, height);
    ReadStream(stream, planes);
    ReadStream(stream, bit_count);
    ReadStream(stream, compression);
    ReadStream(stream, buffer_size);
    ReadStream(stream, xpels_per_meter);
    ReadStream(stream, ypels_per_meter);
    ReadStream(stream, used_color);
    ReadStream(stream, important_color);
    if (IsBigEndian()) {
      size = FlipOrder(size);
      width = FlipOrder(width);
      height = FlipOrder(height);
      planes = FlipOrder(planes);
      bit_count = FlipOrder(bit_count);
      compression = FlipOrder(compression);
      buffer_size = FlipOrder(buffer_size);
      xpels_per_meter = FlipOrder(xpels_per_meter);
      ypels_per_meter = FlipOrder(ypels_per_meter);
      used_color = FlipOrder(used_color);
      important_color = FlipOrder(important_color);
    }
  }

  void Write(std::ofstream &stream) const {
    if (IsBigEndian()) {
      WriteStream(stream, FlipOrder(size));
      WriteStream(stream, FlipOrder(width));
      WriteStream(stream, FlipOrder(height));
      WriteStream(stream, FlipOrder(planes));
      WriteStream(stream, FlipOrder(bit_count));
      WriteStream(stream, FlipOrder(compression));
      WriteStream(stream, FlipOrder(buffer_size));
      WriteStream(stream, FlipOrder(xpels_per_meter));
      WriteStream(stream, FlipOrder(ypels_per_meter));
      WriteStream(stream, FlipOrder(used_color));
      WriteStream(stream, FlipOrder(important_color));
    } else {
      WriteStream(stream, size);
      WriteStream(stream, width);
      WriteStream(stream, height);
      WriteStream(stream, planes);
      WriteStream(stream, bit_count);
      WriteStream(stream, compression);
      WriteStream(stream, buffer_size);
      WriteStream(stream, xpels_per_meter);
      WriteStream(stream, ypels_per_meter);
      WriteStream(stream, used_color);
      WriteStream(stream, important_color);
    }
  }

  void Clear() { memset(this, 0x00, sizeof(BitmapInfoHeader)); }
};

struct RgbquadPalette {
  byte blue;
  byte green;
  byte red;
  byte reserved;
};

static void WriteBitmapPaletteTable(std::ofstream &stream) {
  std::vector<RgbquadPalette> palette(256);
  for (int i = 0; i < 256; i++) {
    palette[i].red = static_cast<byte>(i);
    palette[i].green = static_cast<byte>(i);
    palette[i].blue = static_cast<byte>(i);
    palette[i].reserved = 0;
  }
  stream.write(reinterpret_cast<const char *>(palette.data()),
               sizeof(RgbquadPalette) * 256);
}

static std::vector<RgbquadPalette> ReadBitmapPaletteTable(
    std::ifstream &stream) {
  std::vector<byte> result_buffer(1024);
  stream.read(reinterpret_cast<char *>(result_buffer.data()), 1024);
  std::vector<RgbquadPalette> palette(256);
  for (int i = 0; i < 256; i++) {
    palette[i].red = result_buffer[i * 4];
    palette[i].green = result_buffer[i * 4 + 1];
    palette[i].blue = result_buffer[i * 4 + 2];
    palette[i].reserved = 0;
  }
  return palette;
}

static void WriteBitmapBuffer(std::ofstream &stream,
                              const std::vector<byte> &buffer,
                              size_t width, size_t height, size_t channel) {
  std::uint32_t plane = width * channel;
  std::uint32_t padding = (4 - ((3 * width) % 4)) % 4;
  byte pad_buffer[4] = {0x00, 0x00, 0x00, 0x00};
  for (size_t h = 0; h < height; ++h) {
    size_t start = (height - h - 1) * plane;
    stream.write(reinterpret_cast<const char *>(buffer.data() + start), plane);
    stream.write(reinterpret_cast<const char *>(pad_buffer), padding);
  }
}

static std::vector<byte> ReadBitmapBuffer(std::ifstream &stream, size_t width,
                                          size_t height, size_t channel) {
  std::uint32_t padding = (4 - ((3 * width) % 4)) % 4;
  byte pad[4] = {0x00, 0x00, 0x00, 0x00};
  std::uint32_t plane = width * channel;
  std::vector<byte> buffer(plane * height);
  for (size_t h = 0; h < height; ++h) {
    size_t start = (height - h - 1) * plane;
    stream.read(reinterpret_cast<char *>(buffer.data() + start), plane);
    stream.read(reinterpret_cast<char *>(pad), padding);
  }
  return buffer;
}
}  // namespace yoonvision::image::bitmap

#endif  // YOONVISION_BITMAP_HPP_
