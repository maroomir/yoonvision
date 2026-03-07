#ifndef YOONVISION_HTTP_HTTP_PUBLISHER_HPP_
#define YOONVISION_HTTP_HTTP_PUBLISHER_HPP_

#include <memory>
#include <string>

#include "yoonvision/image.hpp"
#include "http_server.hpp"

namespace yoonvision {
namespace http {

class ImagePublisher {
 public:
  ImagePublisher() = default;
  ~ImagePublisher() = default;

  void Initialize(const std::shared_ptr<HTTPServer>& server,
                  const std::string& stream_id);
  void SetImage(Image::Ptr image);
  bool Publish();

 private:
  std::shared_ptr<HTTPServer> server_;
  std::string stream_id_;
  Image::Ptr image_;
};

}  // namespace http
}  // namespace yoonvision

#endif  // YOONVISION_HTTP_HTTP_PUBLISHER_HPP_
