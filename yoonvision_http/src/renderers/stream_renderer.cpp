#include "yoonvision_http/page_renderer.hpp"
#include "log.hpp"
#include "yoonvision/image.hpp"
#include "yoonvision/jpeg.hpp"
#include <chrono>
#include <sstream>
#include <thread>

namespace yoonvision {
namespace http {

namespace {

std::vector<byte> ImageToRgbInterleaved(const Image& img) {
  const size_t w = img.GetWidth();
  const size_t h = img.GetHeight();
  const size_t ch = img.GetChannel();
  const std::vector<byte>& buf = img.GetBuffer();

  if (ch == 1) {
    return buf;
  }

  const size_t plane = w * h;
  std::vector<byte> out(w * h * 3);

  switch (img.GetImageFormat()) {
    case Image::ImageFormat::kGray:
      return buf;
    case Image::ImageFormat::kRgb:
    case Image::ImageFormat::kRgbParallel:
      for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
          size_t i = y * w + x;
          out[i * 3 + 0] = buf[i];
          out[i * 3 + 1] = buf[plane + i];
          out[i * 3 + 2] = buf[2 * plane + i];
        }
      }
      break;
    case Image::ImageFormat::kBgr:
    case Image::ImageFormat::kBgrParallel:
      for (size_t y = 0; y < h; y++) {
        for (size_t x = 0; x < w; x++) {
          size_t i = y * w + x;
          out[i * 3 + 0] = buf[2 * plane + i];
          out[i * 3 + 1] = buf[plane + i];
          out[i * 3 + 2] = buf[i];
        }
      }
      break;
    case Image::ImageFormat::kRgbMixed:
      return buf;
    case Image::ImageFormat::kBgrMixed: {
      for (size_t i = 0; i < w * h; i++) {
        out[i * 3 + 0] = buf[i * 3 + 2];
        out[i * 3 + 1] = buf[i * 3 + 1];
        out[i * 3 + 2] = buf[i * 3 + 0];
      }
    } break;
    default:
      return out;
  }
  return out;
}

}  // namespace

StreamRenderer::StreamRenderer(GetStreamCallback get_stream,
                               const std::atomic<bool>* running)
    : get_stream_(std::move(get_stream)), running_(running) {}

void StreamRenderer::SetResourcePath(const std::string& path) {
  (void)path;
}

void StreamRenderer::Render(const httplib::Request& req,
                            httplib::Response& res) {
  std::string stream_id(req.matches[1].str());
  LOG_INFO("Stream request received: %s", stream_id.c_str());

  res.set_content_provider(
      "multipart/x-mixed-replace; boundary=frame",
      [this, stream_id](size_t /*offset*/, httplib::DataSink& sink) {
        while (*running_) {
          Image::Ptr frame = get_stream_(stream_id);

          if (frame && frame->GetWidth() > 0 && frame->GetHeight() > 0 &&
              !frame->GetBuffer().empty()) {
            std::vector<byte> rgb = ImageToRgbInterleaved(*frame);
            size_t w = frame->GetWidth();
            size_t h = frame->GetHeight();
            int ch = frame->GetChannel() == 1 ? 1 : 3;

            std::vector<byte> jpeg_data;
            if (image::jpeg::EncodeJpegMemory(rgb, w, h, ch, 80, jpeg_data) &&
                !jpeg_data.empty()) {
              std::ostringstream oss;
              oss << "--frame\r\n";
              oss << "Content-Type: image/jpeg\r\n";
              oss << "Content-Length: " << jpeg_data.size() << "\r\n";
              oss << "Cache-Control: no-store, no-cache, must-revalidate, "
                     "max-age=0\r\n";
              oss << "Pragma: no-cache\r\n";
              oss << "\r\n";
              std::string header = oss.str();

              sink.write(header.c_str(), header.size());
              sink.write(reinterpret_cast<const char*>(jpeg_data.data()),
                        jpeg_data.size());
              sink.write("\r\n", 2);
            }
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        return false;
      });
}

}  // namespace http
}  // namespace yoonvision
