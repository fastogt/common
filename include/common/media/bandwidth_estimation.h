#pragma once

#include <common/media/types.h>  // for bandwidth_t

namespace common {
namespace media {

struct DesireBytesPerSec {
  DesireBytesPerSec();
  DesireBytesPerSec(bandwidth_t min, bandwidth_t max);

  bool IsValid() const;
  bool InRange(bandwidth_t val) const;

  DesireBytesPerSec& operator+=(const DesireBytesPerSec& other);

  bandwidth_t min;
  bandwidth_t max;
};

inline DesireBytesPerSec operator+(const DesireBytesPerSec& left, const DesireBytesPerSec& right) {
  DesireBytesPerSec tmp = left;
  tmp += right;
  return tmp;
}

DesireBytesPerSec CalculateDesireAudioBandwidthBytesPerSec(int rate, int channels);  // raw
DesireBytesPerSec AudioBitrateAverage(bandwidth_t bytes_per_sec);
DesireBytesPerSec CalculateDesireAACBandwidthBytesPerSec(int channels);
DesireBytesPerSec CalculateDesireMP2BandwidthBytesPerSec(int channels);

DesireBytesPerSec VideoBitrateAverage(bandwidth_t bytes_per_sec);
DesireBytesPerSec CalculateDesireH264BandwidthBytesPerSec(int width, int height, double framerate, int profile);
DesireBytesPerSec CalculateDesireMPEGBandwidthBytesPerSec(int width, int height);

}  // namespace media
}  // namespace common
