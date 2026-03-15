//
// Created by maroomir on 2026-03-16.
//

#include "yoonvision/image_processor.hpp"

#include <algorithm>

#include "log.hpp"

namespace yoonvision {

namespace {

enum class ChannelKind { kRed, kGreen, kBlue };

std::vector<byte> ComputeGrayBuffer(const Image& image) {
  const size_t width = image.GetWidth();
  const size_t height = image.GetHeight();
  const size_t size = width * height;
  const auto format = image.GetImageFormat();
  const std::vector<byte>& src = image.GetBuffer();

  std::vector<byte> result(size);

  switch (format) {
    case Image::ImageFormat::kGray:
      result = src;
      break;
    case Image::ImageFormat::kRgb: {
      for (size_t h = 0; h < height; h++) {
        for (size_t w = 0; w < width; w++) {
          size_t pos = h * width + w;
          byte red = src[pos];
          byte green = src[size + pos];
          byte blue = src[2 * size + pos];
          result[pos] = static_cast<byte>(
              0.299 * red + 0.587 * green + 0.114 * blue);
        }
      }
      break;
    }
    case Image::ImageFormat::kBgr: {
      for (size_t h = 0; h < height; h++) {
        for (size_t w = 0; w < width; w++) {
          size_t pos = h * width + w;
          byte blue = src[pos];
          byte green = src[size + pos];
          byte red = src[2 * size + pos];
          result[pos] = static_cast<byte>(
              0.299 * red + 0.587 * green + 0.114 * blue);
        }
      }
      break;
    }
    default:
      LOG_WARN("ComputeGrayBuffer: unsupported format: %d",
               static_cast<int>(format));
      std::fill(result.begin(), result.end(), 0);
      break;
  }

  return result;
}

std::vector<byte> ExtractChannelBuffer(const Image& image, ChannelKind kind) {
  const size_t width = image.GetWidth();
  const size_t height = image.GetHeight();
  const size_t size = width * height;
  const auto format = image.GetImageFormat();
  const std::vector<byte>& src = image.GetBuffer();

  std::vector<byte> result(size);

  switch (kind) {
    case ChannelKind::kRed:
      switch (format) {
        case Image::ImageFormat::kGray:
        case Image::ImageFormat::kRgb:
          std::copy(src.begin(), src.begin() + size, result.begin());
          break;
        case Image::ImageFormat::kBgr:
          std::copy(src.begin() + size * 2, src.begin() + size * 3,
                    result.begin());
          break;
        default:
          LOG_WARN("ExtractChannelBuffer(kRed): unsupported format: %d",
                   static_cast<int>(format));
          std::fill(result.begin(), result.end(), 0);
          break;
      }
      break;

    case ChannelKind::kGreen:
      switch (format) {
        case Image::ImageFormat::kGray:
          std::copy(src.begin(), src.begin() + size, result.begin());
          break;
        case Image::ImageFormat::kRgb:
        case Image::ImageFormat::kBgr:
          std::copy(src.begin() + size, src.begin() + size * 2, result.begin());
          break;
        default:
          LOG_WARN("ExtractChannelBuffer(kGreen): unsupported format: %d",
                   static_cast<int>(format));
          std::fill(result.begin(), result.end(), 0);
          break;
      }
      break;

    case ChannelKind::kBlue:
      switch (format) {
        case Image::ImageFormat::kGray:
        case Image::ImageFormat::kBgr:
          std::copy(src.begin(), src.begin() + size, result.begin());
          break;
        case Image::ImageFormat::kRgb:
          std::copy(src.begin() + size * 2, src.begin() + size * 3,
                    result.begin());
          break;
        default:
          LOG_WARN("ExtractChannelBuffer(kBlue): unsupported format: %d",
                   static_cast<int>(format));
          std::fill(result.begin(), result.end(), 0);
          break;
      }
      break;
  }

  return result;
}

}  // namespace

ImageProcessor ImageProcessor::FromImage(const Image& image) {
  return ImageProcessor(image);
}

ImageProcessor ImageProcessor::FromShared(Image::Ptr image) {
  if (!image) {
    LOG_WARN("ImageProcessor::FromShared: null image pointer");
    return ImageProcessor(Image());
  }
  return ImageProcessor(*image);
}

ImageProcessor::ImageProcessor(const Image& image) : current_(image.Clone()) {}

ImageProcessor::ImageProcessor(Image::Ptr image)
    : current_(image ? image->Clone() : Image()) {}

Image ImageProcessor::MakeGrayImageFromBuffer(const std::vector<byte>& buffer,
                                              size_t width,
                                              size_t height) {
  return Image(buffer, width, height, Image::ImageFormat::kGray);
}

Image ImageProcessor::ResizeNearest(const Image& src,
                                    int target_width,
                                    int target_height) {
  const size_t src_width = src.GetWidth();
  const size_t src_height = src.GetHeight();
  const size_t channels = src.GetChannel();

  Image dst(static_cast<size_t>(target_width),
            static_cast<size_t>(target_height), channels);
  const std::vector<byte>& src_buf = src.GetBuffer();
  std::vector<byte>& dst_buf = dst.GetBuffer();

  const float scale_x =
      static_cast<float>(src_width) / static_cast<float>(target_width);
  const float scale_y =
      static_cast<float>(src_height) / static_cast<float>(target_height);

  for (int y = 0; y < target_height; ++y) {
    int src_y = static_cast<int>(y * scale_y);
    if (src_y >= static_cast<int>(src_height))
      src_y = static_cast<int>(src_height) - 1;
    for (int x = 0; x < target_width; ++x) {
      int src_x = static_cast<int>(x * scale_x);
      if (src_x >= static_cast<int>(src_width))
        src_x = static_cast<int>(src_width) - 1;
      for (size_t c = 0; c < channels; ++c) {
        size_t dst_idx =
            (static_cast<size_t>(y) * target_width +
             static_cast<size_t>(x)) *
                channels +
            c;
        size_t src_idx =
            (static_cast<size_t>(src_y) * src_width +
             static_cast<size_t>(src_x)) *
                channels +
            c;
        dst_buf[dst_idx] = src_buf[src_idx];
      }
    }
  }

  return dst;
}

Image ImageProcessor::LetterboxNearest(const Image& src,
                                       int target_width,
                                       int target_height,
                                       byte pad_value) {
  const size_t src_width = src.GetWidth();
  const size_t src_height = src.GetHeight();
  const size_t channels = src.GetChannel();

  float scale =
      std::min(static_cast<float>(target_width) / static_cast<float>(src_width),
               static_cast<float>(target_height) /
                   static_cast<float>(src_height));

  int new_width = static_cast<int>(src_width * scale);
  int new_height = static_cast<int>(src_height * scale);

  int pad_w = (target_width - new_width) / 2;
  int pad_h = (target_height - new_height) / 2;

  Image dst(static_cast<size_t>(target_width),
            static_cast<size_t>(target_height), channels);
  std::vector<byte>& dst_buf = dst.GetBuffer();
  std::fill(dst_buf.begin(), dst_buf.end(), pad_value);

  const std::vector<byte>& src_buf = src.GetBuffer();

  const float scale_x =
      static_cast<float>(src_width) / static_cast<float>(new_width);
  const float scale_y =
      static_cast<float>(src_height) / static_cast<float>(new_height);

  for (int y = 0; y < new_height; ++y) {
    int src_y = static_cast<int>(y * scale_y);
    if (src_y >= static_cast<int>(src_height))
      src_y = static_cast<int>(src_height) - 1;
    for (int x = 0; x < new_width; ++x) {
      int src_x = static_cast<int>(x * scale_x);
      if (src_x >= static_cast<int>(src_width))
        src_x = static_cast<int>(src_width) - 1;
      for (size_t c = 0; c < channels; ++c) {
        size_t dst_idx =
            ((static_cast<size_t>(y) + pad_h) * target_width +
             (static_cast<size_t>(x) + pad_w)) *
                channels +
            c;
        size_t src_idx =
            (static_cast<size_t>(src_y) * src_width +
             static_cast<size_t>(src_x)) *
                channels +
            c;
        dst_buf[dst_idx] = src_buf[src_idx];
      }
    }
  }

  return dst;
}

ImageProcessor& ImageProcessor::ToGray() {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  std::vector<byte> result = ComputeGrayBuffer(current_);
  current_ = MakeGrayImageFromBuffer(result, width, height);
  return *this;
}

ImageProcessor& ImageProcessor::ToRed() {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  std::vector<byte> result = ExtractChannelBuffer(current_, ChannelKind::kRed);
  current_ = MakeGrayImageFromBuffer(result, width, height);
  return *this;
}

ImageProcessor& ImageProcessor::ToGreen() {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  std::vector<byte> result = ExtractChannelBuffer(current_, ChannelKind::kGreen);
  current_ = MakeGrayImageFromBuffer(result, width, height);
  return *this;
}

ImageProcessor& ImageProcessor::ToBlue() {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  std::vector<byte> result = ExtractChannelBuffer(current_, ChannelKind::kBlue);
  current_ = MakeGrayImageFromBuffer(result, width, height);
  return *this;
}

ImageProcessor& ImageProcessor::ToRgb() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kRgb);
  return *this;
}

ImageProcessor& ImageProcessor::ToBgr() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kBgr);
  return *this;
}

ImageProcessor& ImageProcessor::ToRgbParallel() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kRgbParallel);
  return *this;
}

ImageProcessor& ImageProcessor::ToRgbMixed() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kRgbMixed);
  return *this;
}

ImageProcessor& ImageProcessor::ToBgrParallel() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kBgrParallel);
  return *this;
}

ImageProcessor& ImageProcessor::ToBgrMixed() {
  current_ =
      Image(current_.CopyBuffer(), current_.GetWidth(), current_.GetHeight(),
            Image::ImageFormat::kBgrMixed);
  return *this;
}

ImageProcessor& ImageProcessor::Resize(int width, int height) {
  if (width <= 0 || height <= 0) {
    LOG_WARN("ImageProcessor::Resize: invalid target size (%d x %d)", width,
             height);
    return *this;
  }

  if (current_.GetWidth() == 0 || current_.GetHeight() == 0 ||
      current_.GetChannel() == 0) {
    LOG_WARN("ImageProcessor::Resize: invalid source dimensions");
    return *this;
  }

  current_ = ResizeNearest(current_, width, height);
  return *this;
}

ImageProcessor& ImageProcessor::Letterbox(int target_width,
                                          int target_height,
                                          byte pad_value) {
  if (target_width <= 0 || target_height <= 0) {
    LOG_WARN("ImageProcessor::Letterbox: invalid target size (%d x %d)",
             target_width, target_height);
    return *this;
  }

  if (current_.GetWidth() == 0 || current_.GetHeight() == 0 ||
      current_.GetChannel() == 0) {
    LOG_WARN("ImageProcessor::Letterbox: invalid source dimensions");
    return *this;
  }

  current_ = LetterboxNearest(current_, target_width, target_height, pad_value);
  return *this;
}

ImageProcessor& ImageProcessor::Normalize(float mean, float std) {
  norm_mean_ = mean;
  norm_std_ = (std == 0.0f) ? 1.0f : std;
  use_channel_wise_norm_ = false;
  norm_mean_vec_.clear();
  norm_std_vec_.clear();
  return *this;
}

ImageProcessor& ImageProcessor::Normalize(const std::vector<float>& mean,
                                          const std::vector<float>& std) {
  if (mean.empty() || std.empty() ||
      mean.size() != std.size() ||
      mean.size() != current_.GetChannel()) {
    LOG_WARN("ImageProcessor::Normalize: invalid mean/std size");
    return *this;
  }
  norm_mean_vec_ = mean;
  norm_std_vec_ = std;
  use_channel_wise_norm_ = true;
  return *this;
}

std::vector<float> ImageProcessor::ToFloatCHW() const {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  const size_t channels = current_.GetChannel();
  const std::vector<byte>& src = current_.GetBuffer();

  std::vector<float> result(width * height * channels);
  size_t idx = 0;

  for (size_t c = 0; c < channels; ++c) {
    float mean = use_channel_wise_norm_ ? norm_mean_vec_[c] : norm_mean_;
    float std = use_channel_wise_norm_ ? norm_std_vec_[c] : norm_std_;
    if (std == 0.0f) std = 1.0f;

    for (size_t y = 0; y < height; ++y) {
      for (size_t x = 0; x < width; ++x) {
        size_t src_idx = c * width * height + y * width + x;
        float value = static_cast<float>(src[src_idx]);
        result[idx++] = (value - mean) / std;
      }
    }
  }

  return result;
}

std::vector<float> ImageProcessor::ToFloatHWC() const {
  const size_t width = current_.GetWidth();
  const size_t height = current_.GetHeight();
  const size_t channels = current_.GetChannel();
  const std::vector<byte>& src = current_.GetBuffer();

  std::vector<float> result(width * height * channels);

  for (size_t y = 0; y < height; ++y) {
    for (size_t x = 0; x < width; ++x) {
      for (size_t c = 0; c < channels; ++c) {
        float mean = use_channel_wise_norm_ ? norm_mean_vec_[c] : norm_mean_;
        float std = use_channel_wise_norm_ ? norm_std_vec_[c] : norm_std_;
        if (std == 0.0f) std = 1.0f;

        size_t src_idx = c * width * height + y * width + x;
        size_t dst_idx = (y * width + x) * channels + c;
        float value = static_cast<float>(src[src_idx]);
        result[dst_idx] = (value - mean) / std;
      }
    }
  }

  return result;
}

std::vector<byte> ImageProcessor::ExtractGrayBuffer() const {
  if (current_.GetChannel() != 1) {
    ImageProcessor tmp = *this;
    tmp.ToGray();
    return tmp.current_.CopyBuffer();
  }
  return current_.CopyBuffer();
}

std::vector<byte> ImageProcessor::ExtractRedBuffer() const {
  ImageProcessor tmp = *this;
  tmp.ToRed();
  return tmp.current_.CopyBuffer();
}

std::vector<byte> ImageProcessor::ExtractGreenBuffer() const {
  ImageProcessor tmp = *this;
  tmp.ToGreen();
  return tmp.current_.CopyBuffer();
}

std::vector<byte> ImageProcessor::ExtractBlueBuffer() const {
  ImageProcessor tmp = *this;
  tmp.ToBlue();
  return tmp.current_.CopyBuffer();
}

Image ImageProcessor::Process() const {
  return current_.Clone();
}

}  // namespace yoonvision

