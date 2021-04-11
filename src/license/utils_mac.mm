#include <common/license/utils.h>

#import <Foundation/NSDictionary.h>
#import <Foundation/NSString.h>

#import <IOKit/IOKitLib.h>

namespace common {
namespace license {

bool GetHddID(std::string* serial) {
  if (!serial) {
    return false;
  }

  io_iterator_t io_objects;
  CFMutableDictionaryRef service_properties = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
  kern_return_t kr =
      IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceNameMatching("AppleAHCIDiskDriver"), &io_objects);

  if (kr != KERN_SUCCESS) {
    return false;
  }

  io_service_t io_service;
  NSString* hdd_serial = nil;
  while ((io_service = IOIteratorNext(io_objects))) {
    kr = IORegistryEntryCreateCFProperties(io_service, &service_properties, kCFAllocatorDefault, kNilOptions);
    if (kr == KERN_SUCCESS) {
      NSDictionary* serviceInfo = (__bridge NSDictionary*)service_properties;
      hdd_serial = [serviceInfo objectForKey:@"Serial Number"];
      CFRelease(service_properties);
    }

    IOObjectRelease(io_service);
  }
  IOObjectRelease(io_objects);

  if (!hdd_serial) {
    return false;
  }

  size_t len = [hdd_serial lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
  const char* serial_ptr = [hdd_serial UTF8String];
  if (!serial_ptr) {
    return false;
  }

  *serial = std::string(serial_ptr, len);
  return true;
}

}  // namespace license
}  // namespace common
