#include <common/license/utils.h>

#import <IOKit/IOKitLib.h>

namespace common {
namespace license {

bool GetHddID(std::string* serial) {
  if (!serial) {
    return false;
  }

  io_iterator_t io_objects;
  io_service_t io_service;
  NSString* hdd_serial = nil;

  CFMutableDictionaryRef service_properties = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
  kern_return_t kr =
      IOServiceGetMatchingServices(kIOMasterPortDefault, IOServiceNameMatching("AppleAHCIDiskDriver"), &io_objects);

  if (kr != KERN_SUCCESS) {
    return false;
  }

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

  *serial = [hdd_serial UTF8String];
  return false;
}

}  // namespace license
}  // namespace common
