#ifndef YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_
#define YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_

#include <cstdint>

#include "yooncamera/camera.hpp"
#include "yoondetector/detector.hpp"
#include "yoonvision_http/http_publisher.hpp"

namespace yoonvision::viewer {

class FramePublisherSubscriber : public camera::IFrameSubscriber {
 public:
  FramePublisherSubscriber(
      http::ImagePublisher& raw_publisher,
      http::ImagePublisher* overlay_publisher,
      detector::Detector* detector,
      std::uint32_t inference_interval_frames);

  void OnFrame(const Image::Ptr& frame) override;

 private:
  http::ImagePublisher& raw_publisher_;
  http::ImagePublisher* overlay_publisher_;
  detector::Detector* detector_;
  std::uint32_t inference_interval_frames_;
  std::uint64_t frame_count_;
};

}  // namespace yoonvision::viewer

#endif  // YOONVISION_VIEWER_FRAME_PUBLISHER_SUBSCRIBER_HPP_