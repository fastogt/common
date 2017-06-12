#include "common/file_system.h"

#import <Cocoa/Cocoa.h>

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
