//
// Created by maroomir on 2021-07-05.
//

#ifndef YOONVISION_BYTE_HPP_
#define YOONVISION_BYTE_HPP_

#include <vector>

namespace yoonvision {

using byte = unsigned char;

namespace byte_util {

inline std::vector<byte> ToByte(const int &number) {
  std::vector<byte> bytes(4);
  bytes[0] = number & 0xFF;
  bytes[1] = (number >> 8) & 0xFF;
  bytes[2] = (number >> 16) & 0xFF;
  bytes[3] = (number >> 24) & 0xFF;
  return bytes;
}

inline int ToInteger(const std::vector<byte> &bytes,
                     int invalid_num = -65536) {
  if (bytes.size() != 4) {
    return invalid_num;
  }
  int number = 0;
  number |= bytes[0];
  number |= (bytes[1] << 8);
  number |= (bytes[2] << 16);
  number |= (bytes[3] << 24);
  return number;
}

}  // namespace byte_util

}  // namespace yoonvision

#endif  // YOONVISION_BYTE_HPP_
