//
// Created by 윤철중 on 2023/05/01.
// https://github.com/aumuell/libjpeg-turbo/blob/master/example.c
//

#ifndef YOONVISION_JPEG_HPP_
#define YOONVISION_JPEG_HPP_

#include <jpeglib.h>

#include <csetjmp>

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
}  // namespace

static bool WriteJpegBuffer(const char *path,
                            const std::vector<unsigned char> &buffer,
                            size_t width, size_t height, int channel,
                            int quality = 100) {
  // JPEG 압축 정보 생성
  struct jpeg_compress_struct info {};
  struct jpeg_error_mgr manager {};
  info.err = jpeg_std_error(&manager);
  jpeg_create_compress(&info);
  // file 을 write mode 로 열고 압축 정보 저장
  FILE *file = fopen(path, "wb");
  if (!file) {
    return false;
  }
  jpeg_stdio_dest(&info, file);
  info.image_width = width;
  info.image_height = height;
  switch (channel) {
    case 1:
      info.input_components = channel;
      info.in_color_space = JCS_GRAYSCALE;
      break;
    case 3:
      info.input_components = channel;
      info.in_color_space = JCS_RGB;
      break;
    default:
      return false;
  }
  jpeg_set_defaults(&info);
  jpeg_set_quality(&info, quality, true);
  jpeg_start_compress(&info, true);
  // JPEG buffer 정보 write
  size_t stride = width * channel;
  JSAMPROW row[1];
  while (info.next_scanline < info.image_height) {
    row[0] = const_cast<unsigned char *>(&buffer[info.next_scanline * stride]);
    (void)jpeg_write_scanlines(&info, row, 1);
  }

  jpeg_finish_compress(&info);
  fclose(file);
  jpeg_destroy_compress(&info);
  return true;
}

static bool ReadJpegBuffer(const char *path, std::vector<unsigned char> &result,
                           size_t &width, size_t &height, size_t &channel) {
  // JPEG 압축 정보 읽어오기
  struct jpeg_decompress_struct info {};
  ErrorManager manager;
  // file 에서 JPEG 정보들을 read
  FILE *file = fopen(path, "rb");
  if (!file) {
    return false;
  }
  // JPEG 압축 정보를 가져오던 중 에러가 발생하는지 체크
  info.err = jpeg_std_error(&manager.publisher);
  manager.publisher.error_exit = JpegErrorExit;
  if (setjmp(manager.buffer)) {
    jpeg_destroy_decompress(&info);
    fclose(file);
    return false;
  }
  // JPEG 압축 정보 저장
  jpeg_create_decompress(&info);
  jpeg_stdio_src(&info, file);
  (void)jpeg_read_header(&info, true);
  (void)jpeg_start_decompress(&info);
  width = info.output_width;
  height = info.output_height;
  channel = info.output_components;
  size_t stride = width * channel;
  result.resize(width * height * channel);

  // JPEG buffer 정보를 row 별로 read 해서 복사함
  size_t pixel = 0;
  std::vector<unsigned char> row_buffer(stride);
  JSAMPROW row_ptr[1] = {row_buffer.data()};
  while (info.output_scanline < info.output_height) {
    (void)jpeg_read_scanlines(&info, row_ptr, 1);
    for (size_t i = 0; i < stride; i++) {
      result[pixel++] = row_buffer[i];
    }
  }

  (void)jpeg_finish_decompress(&info);
  jpeg_destroy_decompress(&info);
  fclose(file);
  return true;
}
}  // namespace yoonvision::image::jpeg

#endif  // YOONVISION_JPEG_HPP_
