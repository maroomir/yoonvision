#ifndef YOONVISION_HTTP_PAGE_RENDERER_HPP_
#define YOONVISION_HTTP_PAGE_RENDERER_HPP_

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "httplib.h"
#include "yoonvision/image.hpp"

namespace yoonvision {
namespace http {

using GetStreamCallback = std::function<Image::Ptr(const std::string&)>;
using GetStreamsCallback = std::function<std::map<std::string, Image::Ptr>()>;

class IPageRenderer {
 public:
  virtual ~IPageRenderer() = default;
  virtual void SetResourcePath(const std::string& path) = 0;
  virtual void Render(const httplib::Request& req, httplib::Response& res) = 0;
  virtual std::string GetPath() const = 0;
  virtual bool IsPattern() const { return false; }
};

class StreamRenderer : public IPageRenderer {
 public:
  explicit StreamRenderer(GetStreamCallback get_stream,
                          const std::atomic<bool>* running);
  ~StreamRenderer() override = default;

  void SetResourcePath(const std::string& path) override;
  void Render(const httplib::Request& req, httplib::Response& res) override;
  std::string GetPath() const override { return "/stream/(.*)"; }
  bool IsPattern() const override { return true; }

 private:
  GetStreamCallback get_stream_;
  const std::atomic<bool>* running_;
};

class IndexRenderer : public IPageRenderer {
 public:
  explicit IndexRenderer(GetStreamsCallback get_streams);
  ~IndexRenderer() override = default;

  void SetResourcePath(const std::string& path) override;
  void Render(const httplib::Request& req, httplib::Response& res) override;
  std::string GetPath() const override { return "/"; }

 private:
  std::string resources_path_;
  std::string generate_index_html(
      const std::map<std::string, Image::Ptr>& streams) const;

  GetStreamsCallback get_streams_;
};

}  // namespace http
}  // namespace yoonvision

#endif  // YOONVISION_HTTP_PAGE_RENDERER_HPP_
