#ifndef YOONVISION_HTTP_HTTP_SERVER_HPP_
#define YOONVISION_HTTP_HTTP_SERVER_HPP_

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "yoonvision/image.hpp"
#include "httplib.h"

namespace yoonvision {
namespace http {

class IPageRenderer;

class HTTPServer {
 public:
  HTTPServer();
  ~HTTPServer();

  void Initialize(int port, const std::string& resources_path);
  void Start();
  void Stop();
  bool IsRunning() const;

  void RegisterStream(const std::string& stream_id, Image::Ptr frame);
  void UnregisterStream(const std::string& stream_id);
  std::vector<std::string> GetStreamIds() const;

 private:
  Image::Ptr provide_stream(const std::string& stream_id) const;
  std::map<std::string, Image::Ptr> provide_streams() const;
  void handle_streams_api(httplib::Response& res);
  void server_task();

  int port_;
  std::string resources_path_;
  std::unique_ptr<httplib::Server> server_;
  std::thread server_thread_;
  std::atomic<bool> running_;
  std::map<std::string, Image::Ptr> streams_;
  mutable std::mutex streams_mutex_;
  std::vector<std::unique_ptr<IPageRenderer>> renderers_;
};

}  // namespace http
}  // namespace yoonvision

#endif  // YOONVISION_HTTP_HTTP_SERVER_HPP_
