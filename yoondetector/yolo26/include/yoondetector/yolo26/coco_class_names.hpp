//
// MS COCO 80-class names (YOLO / Ultralytics default order).
//

#ifndef YOONVISION_YOONDETECTOR_YOLO26_COCO_CLASS_NAMES_HPP
#define YOONVISION_YOONDETECTOR_YOLO26_COCO_CLASS_NAMES_HPP

#include <string>

namespace yoonvision {
namespace detector {
namespace yolo26 {

inline const char* CocoClassName(int class_id) {
  static const char* const kNames[80] = {
      "person",        "bicycle",      "car",           "motorcycle",
      "airplane",      "bus",          "train",         "truck",
      "boat",          "traffic light", "fire hydrant",  "stop sign",
      "parking meter", "bench",        "bird",          "cat",
      "dog",           "horse",        "sheep",         "cow",
      "elephant",      "bear",         "zebra",         "giraffe",
      "backpack",      "umbrella",     "handbag",       "tie",
      "suitcase",      "frisbee",      "skis",          "snowboard",
      "sports ball",   "kite",         "baseball bat",  "baseball glove",
      "skateboard",    "surfboard",    "tennis racket", "bottle",
      "wine glass",    "cup",          "fork",          "knife",
      "spoon",         "bowl",         "banana",        "apple",
      "sandwich",      "orange",       "broccoli",      "carrot",
      "hot dog",       "pizza",        "donut",         "cake",
      "chair",         "couch",        "potted plant",  "bed",
      "dining table",  "toilet",       "tv",            "laptop",
      "mouse",         "remote",       "keyboard",      "cell phone",
      "microwave",     "oven",         "toaster",       "sink",
      "refrigerator",  "book",         "clock",         "vase",
      "scissors",      "teddy bear",   "hair drier",    "toothbrush",
  };
  if (class_id >= 0 && class_id < 80) {
    return kNames[class_id];
  }
  return nullptr;
}

inline std::string ClassNameForYoloDetection(int class_id) {
  const char* name = CocoClassName(class_id);
  if (name != nullptr) {
    return std::string(name);
  }
  return "class_" + std::to_string(class_id);
}

}  // namespace yolo26
}  // namespace detector
}  // namespace yoonvision

#endif  // YOONVISION_YOONDETECTOR_YOLO26_COCO_CLASS_NAMES_HPP
