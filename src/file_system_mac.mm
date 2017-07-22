#include "common/file_system.h"

#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSString.h>

namespace common {
namespace file_system {
std::string bundle_pwd() {
  NSString* bundlePath = [[NSBundle mainBundle] resourcePath];
  NSString* secondParentPath =
      [[bundlePath stringByDeletingLastPathComponent] stringByDeletingLastPathComponent];
  return [secondParentPath UTF8String];
}
}
}
