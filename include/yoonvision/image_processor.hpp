//
// Created by maroomir on 2026-03-16.
//

#ifndef YOONVISION_IMAGE_PROCESSOR_HPP_
#define YOONVISION_IMAGE_PROCESSOR_HPP_

#include <vector>

#include "byte.hpp"
#include "image.hpp"

namespace yoonvision {

class ImageProcessor {
 public:
  static ImageProcessor FromImage(const Image& image);
  static ImageProcessor FromShared(Image::Ptr image);

  ImageProcessor& ToGray();
  ImageProcessor& ToRed();
  ImageProcessor& ToGreen();
  ImageProcessor& ToBlue();

  ImageProcessor& ToRgb();
  ImageProcessor& ToBgr();
  ImageProcessor& ToRgbParallel();
  ImageProcessor& ToRgbMixed();
  ImageProcessor& ToBgrParallel();
  ImageProcessor& ToBgrMixed();

  ImageProcessor& Resize(int width, int height);
  ImageProcessor& Letterbox(int target_width, int target_height,
                            byte pad_value = 0);

  ImageProcessor& Normalize(float mean = 0.0f, float std = 255.0f);
  ImageProcessor& Normalize(const std::vector<float>& mean,
                            const std::vector<float>& std);

  std::vector<float> ToFloatCHW() const;
  std::vector<float> ToFloatHWC() const;

  std::vector<byte> ExtractGrayBuffer() const;
  std::vector<byte> ExtractRedBuffer() const;
  std::vector<byte> ExtractGreenBuffer() const;
  std::vector<byte> ExtractBlueBuffer() const;

  [[nodiscard]] Image Process() const;

 private:
  explicit ImageProcessor(const Image& image);
  explicit ImageProcessor(Image::Ptr image);

  static Image MakeGrayImageFromBuffer(const std::vector<byte>& buffer,
                                       size_t width,
                                       size_t height);
  static Image ResizeNearest(const Image& src,
                             int target_width,
                             int target_height);
  static Image LetterboxNearest(const Image& src,
                                int target_width,
                                int target_height,
                                byte pad_value);

  Image current_;

  float norm_mean_{0.0f};
  float norm_std_{255.0f};
  bool use_channel_wise_norm_{false};
  std::vector<float> norm_mean_vec_;
  std::vector<float> norm_std_vec_;
};

}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_PROCESSOR_HPP_

