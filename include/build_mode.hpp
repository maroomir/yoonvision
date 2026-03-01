#ifndef YOONVISION_BUILD_MODE_HPP_
#define YOONVISION_BUILD_MODE_HPP_

namespace yoonvision {

class BuildMode {
 public:
  static constexpr bool IsDebugMode() {
#if defined(NDEBUG)
    return false;
#else
    return true;
#endif
  }
};

}  // namespace yoonvision

#endif  // YOONVISION_BUILD_MODE_HPP_
