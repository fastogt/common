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

#include <common/media/bandwidth_estimation.h>

#include <common/macros.h>  // for DNOTREACHED

#include <common/convert2string.h>

#define KBITS_TO_BYTES(X) (X * 1024 / 8)

#define PROFILE_H264_CONSTRAINED (1 << 9)
#define PROFILE_H264_BASELINE 66                                           // cbr
#define PROFILE_H264_CONSTRAINED_BASELINE (66 | PROFILE_H264_CONSTRAINED)  // cbr
#define PROFILE_H264_MAIN 77
#define PROFILE_H264_EXTENDED 88
#define PROFILE_H264_HIGH 100

#define PROFILE_AAC_MAIN 0  // cbr
#define PROFILE_AAC_LOW 1
#define PROFILE_AAC_SSR 2
#define PROFILE_AAC_LTP 3
#define PROFILE_AAC_HE 4

#define KOEF_BASE_TO_HIGHT 4 / 3

namespace common {
namespace media {

DesireBytesPerSec::DesireBytesPerSec() : min(0), max(0) {}
DesireBytesPerSec::DesireBytesPerSec(bandwidth_t min, bandwidth_t max) : min(min), max(max) {}

bool DesireBytesPerSec::IsValid() const {
  if (min == 0 && max == 0) {
    return false;
  }
  return min <= max;
}

bool DesireBytesPerSec::InRange(bandwidth_t val) const {
  if (!IsValid()) {
    return false;
  }

  return min <= val && val <= max;
}

bool DesireBytesPerSec::Equals(const DesireBytesPerSec& des) const {
  return min == des.min && max == des.max;
}

DesireBytesPerSec& DesireBytesPerSec::operator+=(const DesireBytesPerSec& other) {
  min += other.min;
  max += other.max;
  return *this;
}

DesireBytesPerSec CalculateDesireAudioBandwidthBytesPerSec(int rate, int channels) {
  if (rate < 0 || channels < 0) {
    DNOTREACHED() << "CalculateDesireAudioBandwidthBytesPerSec invalid input, rate: " << rate
                  << ", channels: " << channels;
    return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
  }

  unsigned min_bitrate_kbps = static_cast<unsigned>(rate) * static_cast<unsigned>(channels) / 1000;
  return DesireBytesPerSec(KBITS_TO_BYTES(min_bitrate_kbps), KBITS_TO_BYTES(min_bitrate_kbps * 2));
}

DesireBytesPerSec CalculateDesireMP2BandwidthBytesPerSec(int channels) {
  if (channels < 0) {
    DNOTREACHED() << "CalculateDesireMP2BandwidthBytesPerSec invalid input, channels: " << channels;
    return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
  }

  if (channels == 1) {
    return DesireBytesPerSec(KBITS_TO_BYTES(72), KBITS_TO_BYTES(128));
  } else if (channels == 2) {
    return DesireBytesPerSec(KBITS_TO_BYTES(144), KBITS_TO_BYTES(256));
  } else if (channels == 4) {
    return DesireBytesPerSec(KBITS_TO_BYTES(288), KBITS_TO_BYTES(512));
  } else if (channels == 6) {
    return DesireBytesPerSec(KBITS_TO_BYTES(432), KBITS_TO_BYTES(768));
  }

  DNOTREACHED() << "CalculateDesireMP2BandwidthBytesPerSec failed, channels:" << channels;
  return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
}

DesireBytesPerSec AudioBitrateAverage(bandwidth_t bytes_per_sec) {
  return DesireBytesPerSec(bytes_per_sec * 3 / 4, bytes_per_sec * 4 / 3);
}

DesireBytesPerSec CalculateDesireAACBandwidthBytesPerSec(int channels) {
  // https://toolstud.io/video/audiosize.php
  if (channels == 1) {
    return DesireBytesPerSec(KBITS_TO_BYTES(32), KBITS_TO_BYTES(96));
  } else if (channels == 2) {
    return DesireBytesPerSec(KBITS_TO_BYTES(64), KBITS_TO_BYTES(192));
  } else if (channels == 4) {
    return DesireBytesPerSec(KBITS_TO_BYTES(128), KBITS_TO_BYTES(384));
  } else if (channels == 6) {
    return DesireBytesPerSec(KBITS_TO_BYTES(256), KBITS_TO_BYTES(512));
  }

  DNOTREACHED() "CalculateDesireAACBandwidthBytesPerSec failed, channels:" << channels;
  return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
}

DesireBytesPerSec VideoBitrateAverage(bandwidth_t bytes_per_sec) {
  return DesireBytesPerSec(bytes_per_sec / 2, bytes_per_sec * 3 / 2);
}

DesireBytesPerSec CalculateDesireH264BandwidthBytesPerSec(int width, int height, double framerate, int profile) {
  // https://support.google.com/youtube/answer/2853702?hl=en
  // https://support.ustream.tv/hc/en-us/articles/207852117-Internet-connection-and-recommended-encoding-settings
  // https://support.google.com/youtube/answer/1722171?hl=en
  const int stabled_profile = stable_value_in_range(profile, PROFILE_H264_BASELINE, PROFILE_H264_HIGH);
  if (stabled_profile < PROFILE_H264_MAIN) {
    if (framerate <= 30.0) {
      if (width >= 3840 && height >= 2160) {  // 2160p (4k) 40000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(13000), KBITS_TO_BYTES(40000));
      } else if (width >= 2560 && height >= 1440) {  // 1440p (2k) 16000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(6000), KBITS_TO_BYTES(16000));
      } else if (width >= 1920 && height >= 1080) {  // 1080p 8000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(3000), KBITS_TO_BYTES(8000));
      } else if (width >= 1280 && height >= 720) {  // 720p 5000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(1500), KBITS_TO_BYTES(5000));
      } else if (width >= 854 && height >= 480) {  // 480p 2500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(500), KBITS_TO_BYTES(2500));
      } else if (width >= 640 && height >= 360) {  // 360p 1000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(400), KBITS_TO_BYTES(1000));
      } else if (width >= 426 && height >= 240) {  // 240p 600 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(300), KBITS_TO_BYTES(700));
      } else {
        return DesireBytesPerSec(KBITS_TO_BYTES(300), KBITS_TO_BYTES(700));
      }
    } else {
      if (width >= 3840 && height >= 2160) {  // 2160p 60000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(20000), KBITS_TO_BYTES(60000));
      } else if (width >= 2560 && height >= 1440) {  // 1440p 24000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(9000), KBITS_TO_BYTES(24000));
      } else if (width >= 1920 && height >= 1080) {  // 1080p 12000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(4500), KBITS_TO_BYTES(12000));
      } else if (width >= 1280 && height >= 720) {  // 720p 7500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(2250), KBITS_TO_BYTES(7500));
      } else if (width >= 854 && height >= 480) {  // 480p 4000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(1000), KBITS_TO_BYTES(4000));
      } else if (width >= 640 && height >= 360) {  // 360p 1500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(600), KBITS_TO_BYTES(1500));
      } else if (width >= 426 && height >= 240) {  // 240p 900 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(400), KBITS_TO_BYTES(900));
      } else {
        return DesireBytesPerSec(KBITS_TO_BYTES(400), KBITS_TO_BYTES(900));
      }
    }
  } else if (stabled_profile <= PROFILE_H264_HIGH) {
    if (framerate <= 30.0) {
      if (width >= 3840 && height >= 2160) {  // 2160p (4k) 40000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(13000) * KOEF_BASE_TO_HIGHT,
                                 KBITS_TO_BYTES(40000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 2560 && height >= 1440) {  // 1440p (2k) 16000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(6000) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(16000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 1920 && height >= 1080) {  // 1080p 12000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(3000), KBITS_TO_BYTES(12000));
      } else if (width >= 1280 && height >= 720) {  // 720p 6000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(1000), KBITS_TO_BYTES(6000));
      } else if (width >= 854 && height >= 480) {  // 480p 2500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(500) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(2500) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 640 && height >= 360) {  // 360p 2500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(600), KBITS_TO_BYTES(2500));
      } else if (width >= 426 && height >= 240) {  // 240p 600 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(300) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(700) * KOEF_BASE_TO_HIGHT);
      } else {
        return DesireBytesPerSec(KBITS_TO_BYTES(300) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(700) * KOEF_BASE_TO_HIGHT);
      }
    } else {
      if (width >= 3840 && height >= 2160) {  // 2160p 60000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(20000) * KOEF_BASE_TO_HIGHT,
                                 KBITS_TO_BYTES(60000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 2560 && height >= 1440) {  // 1440p 24000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(9000) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(24000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 1920 && height >= 1080) {  // 1080p 12000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(4500) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(12000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 1280 && height >= 720) {  // 720p 7500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(2250) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(7500) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 854 && height >= 480) {  // 480p 4000 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(1000) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(4000) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 640 && height >= 360) {  // 360p 1500 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(600) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(1500) * KOEF_BASE_TO_HIGHT);
      } else if (width >= 426 && height >= 240) {  // 240p 900 Kbps
        return DesireBytesPerSec(KBITS_TO_BYTES(400) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(900) * KOEF_BASE_TO_HIGHT);
      } else {
        return DesireBytesPerSec(KBITS_TO_BYTES(400) * KOEF_BASE_TO_HIGHT, KBITS_TO_BYTES(900) * KOEF_BASE_TO_HIGHT);
      }
    }
  }

  DNOTREACHED() << "CalculateDesireH264BandwidthBytesPerSec failed, size: " << width << "x" << height
                << ", framerate: " << framerate << ", profile: " << profile;
  return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
}

DesireBytesPerSec CalculateDesireH264BandwidthBytesPerSec(int width,
                                                          int height,
                                                          double framerate,
                                                          const std::string& profile) {
  int prof = PROFILE_H264_MAIN;
  if (profile == "main") {
    prof = PROFILE_H264_MAIN;
  } else if (profile == "high") {
    prof = PROFILE_H264_HIGH;
  } else if (profile == "baseline") {
    prof = PROFILE_H264_BASELINE;
  } else if (profile == "constrained-baseline") {
    prof = PROFILE_H264_CONSTRAINED_BASELINE;
  }

  return CalculateDesireH264BandwidthBytesPerSec(width, height, framerate, prof);
}

DesireBytesPerSec CalculateDesireH265BandwidthBytesPerSec(int width,
                                                          int height,
                                                          double framerate,
                                                          const std::string& profile) {
  int prof = PROFILE_H264_MAIN;
  if (profile == "main") {
    prof = PROFILE_H264_MAIN;
  } else if (profile == "high") {
    prof = PROFILE_H264_HIGH;
  } else if (profile == "baseline") {
    prof = PROFILE_H264_BASELINE;
  } else if (profile == "constrained-baseline") {
    prof = PROFILE_H264_CONSTRAINED_BASELINE;
  }

  return CalculateDesireH264BandwidthBytesPerSec(width, height, framerate, prof);
}

DesireBytesPerSec CalculateDesireH265BandwidthBytesPerSec(int width, int height, double framerate, int profile) {
  return CalculateDesireH264BandwidthBytesPerSec(width, height, framerate, profile);
}

DesireBytesPerSec CalculateDesireMPEGBandwidthBytesPerSec(int width, int height) {
  // https://en.wikipedia.org/wiki/H.262/MPEG-2_Part_2#Video_profiles_and_levels
  // http://www.iem.thm.de/telekom-labor/zinke/mk/mpeg2beg/beginnzi.htm#Profiles
  // and Levels
  if (width <= 352 && height <= 288) {
    return DesireBytesPerSec(KBITS_TO_BYTES(2000), KBITS_TO_BYTES(4000));
  } else if (width <= 720 && height <= 576) {
    return DesireBytesPerSec(KBITS_TO_BYTES(7500), KBITS_TO_BYTES(15000));
  } else if (width <= 1440 && height <= 1152) {
    return DesireBytesPerSec(KBITS_TO_BYTES(30000), KBITS_TO_BYTES(60000));
  } else if (width <= 1920 && height <= 1152) {
    return DesireBytesPerSec(KBITS_TO_BYTES(40000), KBITS_TO_BYTES(80000));
  }

  DNOTREACHED() << "CalculateDesireMPEGBandwidthBytesPerSec failed, size: " << width << "x" << height;
  return DesireBytesPerSec(KBITS_TO_BYTES(0), KBITS_TO_BYTES(0));
}

}  // namespace media

std::string ConvertToString(const media::DesireBytesPerSec& from) {
  if (!from.IsValid()) {
    return std::string();
  }

  std::ostringstream bw;
  bw << from.min << ":" << from.max;
  return bw.str();
}

bool ConvertFromString(const std::string& from, media::DesireBytesPerSec* out) {
  if (!out) {
    return false;
  }

  media::DesireBytesPerSec res;
  size_t del = from.find_first_of(':');
  if (del != std::string::npos) {
    media::bandwidth_t lmin;
    bool ok = ConvertFromString(from.substr(0, del), &lmin);
    if (ok) {
      res.min = lmin;
    }

    media::bandwidth_t lmax;
    ok = ConvertFromString(from.substr(del + 1), &lmax);
    if (ok) {
      res.max = lmax;
    }
  }

  *out = res;
  return true;
}
}  // namespace common
