#include "yoonvision/image_draw.hpp"

#include <algorithm>
#include <cstdint>

namespace yoonvision {

constexpr int kCharCellWidth = 6;

namespace {

int ClampInt(int value, int min_value, int max_value) {
  return std::max(min_value, std::min(value, max_value));
}

void PutPixel(Image& image, int x, int y, Rgb8 color) {
  const int width = static_cast<int>(image.GetWidth());
  const int height = static_cast<int>(image.GetHeight());

  if (x < 0 || y < 0 || x >= width || y >= height) {
    return;
  }

  if (image.GetChannel() == 1) {
    image(static_cast<size_t>(y), static_cast<size_t>(x), 0) =
        static_cast<byte>((static_cast<int>(color.r) + color.g + color.b) / 3);
    return;
  }

  image(static_cast<size_t>(y), static_cast<size_t>(x), 0) = color.r;
  image(static_cast<size_t>(y), static_cast<size_t>(x), 1) = color.g;
  image(static_cast<size_t>(y), static_cast<size_t>(x), 2) = color.b;
}

namespace font_data {
#include "image_draw_font_data.inc"
}

void DrawGlyph5x7(Image& image,
                  int x,
                  int y,
                  char ch,
                  const Rgb8& fg,
                  int scale) {
  unsigned char c = static_cast<unsigned char>(ch);
  if (c < 32 || c > 127) {
    c = static_cast<unsigned char>(' ');
  }
  const int idx = static_cast<int>(c) - 32;
  const unsigned char* glyph = font_data::kFont5x7Ascii + idx * 5;
  const int s = std::max(1, scale);

  for (int col = 0; col < 5; ++col) {
    std::uint8_t line = glyph[col];
    for (int row = 0; row < 7; ++row, line >>= 1) {
      if ((line & 1) == 0) {
        continue;
      }
      for (int sy = 0; sy < s; ++sy) {
        for (int sx = 0; sx < s; ++sx) {
          PutPixel(image, x + col * s + sx, y + row * s + sy, fg);
        }
      }
    }
  }
}

}  // namespace

int ImageDrawContext::MeasureText5x7(const std::string& text, int scale) {
  const int s = std::max(1, scale);
  return static_cast<int>(text.size()) * kCharCellWidth * s;
}

Rgb8 ImageDrawContext::StableRgbFromInt(int value) {
  const std::uint32_t seed =
      static_cast<std::uint32_t>(value < 0 ? -value : value);
  return Rgb8{
      static_cast<byte>(64u + ((seed * 97u) % 192u)),
      static_cast<byte>(64u + ((seed * 57u + 83u) % 192u)),
      static_cast<byte>(64u + ((seed * 131u + 191u) % 192u))};
}

ImageDrawContext::ImageDrawContext(Image& image) : image_(image) {}

ImageDrawContext& ImageDrawContext::SetPixel(int x, int y, Rgb8 color) {
  PutPixel(image_, x, y, color);
  return *this;
}

ImageDrawContext& ImageDrawContext::DrawHorizontalLine(int x0,
                                                       int x1,
                                                       int y,
                                                       Rgb8 color) {
  const int start = std::min(x0, x1);
  const int end = std::max(x0, x1);
  for (int x = start; x <= end; ++x) {
    PutPixel(image_, x, y, color);
  }
  return *this;
}

ImageDrawContext& ImageDrawContext::DrawVerticalLine(int y0,
                                                     int y1,
                                                     int x,
                                                     Rgb8 color) {
  const int start = std::min(y0, y1);
  const int end = std::max(y0, y1);
  for (int y = start; y <= end; ++y) {
    PutPixel(image_, x, y, color);
  }
  return *this;
}

ImageDrawContext& ImageDrawContext::FillRectangle(int left,
                                                  int top,
                                                  int right,
                                                  int bottom,
                                                  Rgb8 color) {
  if (image_.GetWidth() == 0 || image_.GetHeight() == 0) {
    return *this;
  }

  const int width = static_cast<int>(image_.GetWidth());
  const int height = static_cast<int>(image_.GetHeight());

  left = ClampInt(left, 0, width - 1);
  right = ClampInt(right, 0, width - 1);
  top = ClampInt(top, 0, height - 1);
  bottom = ClampInt(bottom, 0, height - 1);

  if (right < left) {
    std::swap(left, right);
  }
  if (bottom < top) {
    std::swap(top, bottom);
  }

  for (int y = top; y <= bottom; ++y) {
    for (int x = left; x <= right; ++x) {
      PutPixel(image_, x, y, color);
    }
  }
  return *this;
}

ImageDrawContext& ImageDrawContext::OutlineRectangle(int left,
                                                     int top,
                                                     int right,
                                                     int bottom,
                                                     Rgb8 color,
                                                     int thickness) {
  if (image_.GetWidth() == 0 || image_.GetHeight() == 0 || thickness <= 0) {
    return *this;
  }

  const int width = static_cast<int>(image_.GetWidth());
  const int height = static_cast<int>(image_.GetHeight());

  left = ClampInt(left, 0, width - 1);
  right = ClampInt(right, 0, width - 1);
  top = ClampInt(top, 0, height - 1);
  bottom = ClampInt(bottom, 0, height - 1);

  if (right <= left || bottom <= top) {
    return *this;
  }

  for (int t = 0; t < thickness; ++t) {
    DrawHorizontalLine(left, right, top + t, color);
    DrawHorizontalLine(left, right, bottom - t, color);
    DrawVerticalLine(top, bottom, left + t, color);
    DrawVerticalLine(top, bottom, right - t, color);
  }
  return *this;
}

ImageDrawContext& ImageDrawContext::DrawText5x7(int x,
                                                int y,
                                                const std::string& text,
                                                Rgb8 fg,
                                                int scale) {
  const int s = std::max(1, scale);
  int cx = x;
  for (char ch : text) {
    DrawGlyph5x7(image_, cx, y, ch, fg, s);
    cx += kCharCellWidth * s;
  }
  return *this;
}

}  // namespace yoonvision
