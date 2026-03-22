//
// Created by maroomir on 2026-03-22.
//

#ifndef YOONVISION_IMAGE_DRAW_HPP_
#define YOONVISION_IMAGE_DRAW_HPP_

#include <string>

#include "byte.hpp"
#include "image.hpp"

namespace yoonvision {

struct Rgb8 {
  byte r;
  byte g;
  byte b;
};

class ImageDrawContext {
 public:
  explicit ImageDrawContext(Image& image);

  [[nodiscard]] static int MeasureText5x7(const std::string& text, int scale);
  [[nodiscard]] static Rgb8 StableRgbFromInt(int value);

  [[nodiscard]] Image& Target() { return image_; }
  [[nodiscard]] const Image& Target() const { return image_; }

  ImageDrawContext& SetPixel(int x, int y, Rgb8 color);
  ImageDrawContext& DrawHorizontalLine(int x0, int x1, int y, Rgb8 color);
  ImageDrawContext& DrawVerticalLine(int y0, int y1, int x, Rgb8 color);
  ImageDrawContext& FillRectangle(int left, int top, int right, int bottom,
                                  Rgb8 color);
  ImageDrawContext& OutlineRectangle(int left, int top, int right, int bottom,
                                     Rgb8 color, int thickness);
  ImageDrawContext& DrawText5x7(int x, int y, const std::string& text, Rgb8 fg,
                                int scale);

 private:
  Image& image_;
};

}  // namespace yoonvision

#endif  // YOONVISION_IMAGE_DRAW_HPP_
