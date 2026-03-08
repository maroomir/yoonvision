//
// Created by 윤철중 on 2023/05/01.
// https://github.com/aumuell/libjpeg-turbo/blob/master/example.c
//

#ifndef YOONVISION_JPEG_HPP_
#define YOONVISION_JPEG_HPP_

#include <jpeglib.h>
#include <csetjmp>
#include <cstdlib>
#include <vector>

#include "byte.hpp"

namespace yoonvision::image::jpeg {
namespace {
struct ErrorManager {
  struct jpeg_error_mgr publisher {};
  jmp_buf buffer{};
};

void JpegErrorExit(j_common_ptr info) {
  auto *err = (ErrorManager *)info->err;
  (info->err->output_message)(info);
  longjmp(err->buffer, 1);
}

bool SetupCompressParams(j_compress_ptr cinfo, size_t width, size_t height,
                         int channel, int quality) {
  cinfo->image_width = static_cast<JDIMENSION>(width);
  cinfo->image_height = static_cast<JDIMENSION>(height);
  switch (channel) {
    case 1:
      cinfo->input_components = 1;
      cinfo->in_color_space = JCS_GRAYSCALE;
      break;
    case 3:
      cinfo->input_components = 3;
      cinfo->in_color_space = JCS_RGB;
      break;
    default:
      return false;
  }
  jpeg_set_defaults(cinfo);
  jpeg_set_quality(cinfo, quality, TRUE);
  jpeg_start_compress(cinfo, TRUE);
  return true;
}

void WriteScanlinesFromBuffer(j_compress_ptr cinfo,
                              const std::vector<byte>& buffer,
                              size_t stride) {
  JSAMPROW row[1];
  while (cinfo->next_scanline < cinfo->image_height) {
    row[0] = const_cast<byte*>(&buffer[cinfo->next_scanline * stride]);
    (void)jpeg_write_scanlines(cinfo, row, 1);
  }
}

bool PrepareDecompressFromFile(j_decompress_ptr cinfo, FILE* file,
                               ErrorManager* manager) {
  cinfo->err = jpeg_std_error(&manager->publisher);
  manager->publisher.error_exit = JpegErrorExit;
  if (setjmp(manager->buffer)) {
    jpeg_destroy_decompress(cinfo);
    fclose(file);
    return false;
  }
  jpeg_create_decompress(cinfo);
  jpeg_stdio_src(cinfo, file);
  (void)jpeg_read_header(cinfo, TRUE);
  (void)jpeg_start_decompress(cinfo);
  return true;
}

void ReadAllScanlinesToBuffer(j_decompress_ptr cinfo, std::vector<byte>& result) {
  const size_t stride = cinfo->output_width * cinfo->output_components;
  size_t pixel = 0;
  std::vector<byte> row_buffer(stride);
  JSAMPROW row_ptr[1] = {row_buffer.data()};
  while (cinfo->output_scanline < cinfo->output_height) {
    (void)jpeg_read_scanlines(cinfo, row_ptr, 1);
    for (size_t i = 0; i < stride; i++) {
      result[pixel++] = row_buffer[i];
    }
  }
}
}  // namespace

static bool WriteJpegBuffer(const char* path,
                            const std::vector<byte>& buffer,
                            size_t width, size_t height, int channel,
                            int quality = 100) {
  struct jpeg_compress_struct info {};
  struct jpeg_error_mgr manager {};
  info.err = jpeg_std_error(&manager);
  jpeg_create_compress(&info);

  FILE* file = fopen(path, "wb");
  if (!file) {
    jpeg_destroy_compress(&info);
    return false;
  }
  jpeg_stdio_dest(&info, file);

  if (!SetupCompressParams(&info, width, height, channel, quality)) {
    fclose(file);
    jpeg_destroy_compress(&info);
    return false;
  }

  const size_t stride = width * static_cast<size_t>(channel);
  WriteScanlinesFromBuffer(&info, buffer, stride);

  jpeg_finish_compress(&info);
  fclose(file);
  jpeg_destroy_compress(&info);
  return true;
}

static bool ReadJpegBuffer(const char* path, std::vector<byte>& result,
                           size_t& width, size_t& height, size_t& channel) {
  struct jpeg_decompress_struct info {};
  ErrorManager manager;

  FILE* file = fopen(path, "rb");
  if (!file) {
    return false;
  }

  if (!PrepareDecompressFromFile(&info, file, &manager)) {
    return false;
  }

  width = info.output_width;
  height = info.output_height;
  channel = static_cast<size_t>(info.output_components);
  result.resize(width * height * channel);

  ReadAllScanlinesToBuffer(&info, result);

  (void)jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  fclose(file);
  return true;
}

inline bool EncodeJpegMemory(const std::vector<byte>& buffer,
                             size_t width, size_t height, int channel,
                             int quality, std::vector<byte>& out) {
  if (buffer.size() < width * height * static_cast<size_t>(channel)) {
    return false;
  }
  out.clear();

  struct jpeg_compress_struct info {};
  struct jpeg_error_mgr err {};
  info.err = jpeg_std_error(&err);
  jpeg_create_compress(&info);

  unsigned char* encoded = nullptr;
  unsigned long encoded_size = 0;
  jpeg_mem_dest(&info, &encoded, &encoded_size);

  if (!SetupCompressParams(&info, width, height, channel, quality)) {
    if (encoded) {
      free(encoded);
    }
    jpeg_destroy_compress(&info);
    return false;
  }

  const size_t stride = width * static_cast<size_t>(channel);
  WriteScanlinesFromBuffer(&info, buffer, stride);

  jpeg_finish_compress(&info);

  if (encoded && encoded_size > 0) {
    out.assign(encoded, encoded + encoded_size);
  }

  if (encoded) {
    free(encoded);
  }

  jpeg_destroy_compress(&info);
  return !out.empty();
}

}  // namespace yoonvision::image::jpeg

#endif  // YOONVISION_JPEG_HPP_
