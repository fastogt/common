/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <string>

#include <common/macros.h>
#include <common/media/types.h>  // for bandwidth_t

namespace common {
namespace media {

struct DesireBytesPerSec {
  DesireBytesPerSec();
  DesireBytesPerSec(bandwidth_t min, bandwidth_t max);

  bool IsValid() const;
  bool InRange(bandwidth_t val) const;
  bool Equals(const DesireBytesPerSec& des) const;

  DesireBytesPerSec& operator+=(const DesireBytesPerSec& other);

  bandwidth_t min;
  bandwidth_t max;
};

inline DesireBytesPerSec operator+(const DesireBytesPerSec& left, const DesireBytesPerSec& right) {
  DesireBytesPerSec tmp = left;
  tmp += right;
  return tmp;
}

inline bool operator==(const DesireBytesPerSec& left, const DesireBytesPerSec& right) {
  return left.Equals(right);
}

inline bool operator!=(const DesireBytesPerSec& left, const DesireBytesPerSec& right) {
  return !(left == right);
}

DesireBytesPerSec CalculateDesireAudioBandwidthBytesPerSec(int rate, int channels);  // raw
DesireBytesPerSec AudioBitrateAverage(bandwidth_t bytes_per_sec);
DesireBytesPerSec CalculateDesireAACBandwidthBytesPerSec(int channels);
DesireBytesPerSec CalculateDesireMP2BandwidthBytesPerSec(int channels);

DesireBytesPerSec VideoBitrateAverage(bandwidth_t bytes_per_sec);
DesireBytesPerSec CalculateDesireH264BandwidthBytesPerSec(int width, int height, double framerate, int profile);
DesireBytesPerSec CalculateDesireH264BandwidthBytesPerSec(int width,
                                                          int height,
                                                          double framerate,
                                                          const std::string& profile);
DesireBytesPerSec CalculateDesireH265BandwidthBytesPerSec(int width, int height, double framerate, int profile);
DesireBytesPerSec CalculateDesireH265BandwidthBytesPerSec(int width,
                                                          int height,
                                                          double framerate,
                                                          const std::string& profile);
DesireBytesPerSec CalculateDesireMPEGBandwidthBytesPerSec(int width, int height);

}  // namespace media

std::string ConvertToString(const media::DesireBytesPerSec& from);
bool ConvertFromString(const std::string& from, media::DesireBytesPerSec* out) WARN_UNUSED_RESULT;
}  // namespace common
