#include "common/system_info/system_info.h"

#import <Foundation/NSProcessInfo.h>
#import <Foundation/NSString.h>

namespace common {
namespace system_info {

std::string OperatingSystemVersion() {
  NSProcessInfo *pInfo = [NSProcessInfo processInfo];
  NSString *version = [pInfo operatingSystemVersionString];
  return [version UTF8String];
}

}  // namespace system_info
}  // namespace common
