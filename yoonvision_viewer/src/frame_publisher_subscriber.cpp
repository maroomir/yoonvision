#include "yoonvision_viewer/frame_publisher_subscriber.hpp"

#include <cstdio>
#include <memory>
#include <string>

#include "yoonvision/image_draw.hpp"

namespace yoonvision::viewer {

namespace {

constexpr int kLabelScale = 4;
constexpr int kLabelPad = 4;
constexpr int kOverlayBoxThickness = 2;

void DrawDetectionOverlay(Image& image,
                          const detector::DetectionResult& detection_result,
                          int thickness) {
  if (image.GetWidth() == 0 || image.GetHeight() == 0 || thickness <= 0) {
    return;
  }

  const int img_w = static_cast<int>(image.GetWidth());
  const int img_h = static_cast<int>(image.GetHeight());

  for (const auto& detection : detection_result.detections) {
    const float cx = detection.bbox.cx;
    const float cy = detection.bbox.cy;
    const float w = detection.bbox.w;
    const float h = detection.bbox.h;

    if (w <= 0.0f || h <= 0.0f) {
      continue;
    }

    int left = static_cast<int>(cx - (w * 0.5f));
    int right = static_cast<int>(cx + (w * 0.5f));
    int top = static_cast<int>(cy - (h * 0.5f));
    int bottom = static_cast<int>(cy + (h * 0.5f));

    const yoonvision::Rgb8 color =
        yoonvision::ImageDrawContext::StableRgbFromInt(detection.class_id);

    char label_buf[192];
    const char* name = detection.class_name.empty() ? "?"
                                                      : detection.class_name.c_str();
    std::snprintf(label_buf, sizeof(label_buf), "%s %.2f", name,
                  static_cast<double>(detection.score));
    const std::string label(label_buf);

    const int s = kLabelScale;
    const int pad = kLabelPad;
    const int tw = yoonvision::ImageDrawContext::MeasureText5x7(label, s);
    const int th = 7 * s;
    const int plate_w = tw + 2 * pad;
    const int plate_h = th + 2 * pad;

    int lx = left;
    int ly = top - plate_h - 2;
    if (ly < 0) {
      ly = top + 2;
    }
    if (ly + plate_h > img_h) {
      ly = img_h - plate_h;
    }
    if (lx + plate_w > img_w) {
      lx = img_w - plate_w;
    }
    if (lx < 0) {
      lx = 0;
    }

    constexpr yoonvision::Rgb8 kLabelText{255, 255, 255};
    constexpr yoonvision::Rgb8 kLabelBg{32, 32, 32};

    yoonvision::ImageDrawContext(image)
        .OutlineRectangle(left, top, right, bottom, color, thickness)
        .FillRectangle(lx, ly, lx + plate_w - 1, ly + plate_h - 1, kLabelBg)
        .DrawText5x7(lx + pad, ly + pad, label, kLabelText, s);
  }
}

}  // namespace

FramePublisherSubscriber::FramePublisherSubscriber(
    http::ImagePublisher& raw_publisher,
    http::ImagePublisher* overlay_publisher,
    detector::Detector* detector,
    std::uint32_t inference_interval_frames)
    : raw_publisher_(raw_publisher),
      overlay_publisher_(overlay_publisher),
      detector_(detector),
      inference_interval_frames_(inference_interval_frames == 0
                                     ? 1
                                     : inference_interval_frames),
      frame_count_(0) {}

void FramePublisherSubscriber::OnFrame(const Image::Ptr& frame) {
  if (!frame) {
    return;
  }

  raw_publisher_.SetImage(frame);
  raw_publisher_.Publish();

  if (!overlay_publisher_ || !detector_ || !detector_->IsInitialized()) {
    return;
  }

  ++frame_count_;
  if ((frame_count_ % inference_interval_frames_) != 0) {
    return;
  }

  detector::DetectionResult detection_result = detector_->Detect(*frame);

  auto overlay_frame = std::make_shared<Image>(frame->Clone());
  DrawDetectionOverlay(*overlay_frame, detection_result, kOverlayBoxThickness);

  overlay_publisher_->SetImage(overlay_frame);
  overlay_publisher_->Publish();
}

}  // namespace yoonvision::viewer
