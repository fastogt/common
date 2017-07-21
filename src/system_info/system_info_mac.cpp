#include "common/system_info/system_info.h"

//#include <ApplicationServices/ApplicationServices.h>
#include <CoreServices/CoreServices.h>

#include <mach/mach_host.h>
#include <mach/mach_init.h>

#include "common/sprintf.h"

namespace common {
namespace system_info {

std::string OperatingSystemName() {
  return "Mac OS X";
}

std::string OperatingSystemVersion() {
  int32_t major, minor, bugfix;
  Gestalt(gestaltSystemVersionMajor, reinterpret_cast<SInt32*>(&major));
  Gestalt(gestaltSystemVersionMinor, reinterpret_cast<SInt32*>(&minor));
  Gestalt(gestaltSystemVersionBugFix, reinterpret_cast<SInt32*>(&bugfix));

  return MemSPrintf("%d.%d.%d", major, minor, bugfix);
}

int64_t AmountOfPhysicalMemory() {
  struct host_basic_info hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  mach_port_t host = mach_host_self();
  int result = host_info(host, HOST_BASIC_INFO, reinterpret_cast<host_info_t>(&hostinfo), &count);
  if (result != KERN_SUCCESS) {
    NOTREACHED();
    return 0;
  }

  DCHECK_EQ(HOST_BASIC_INFO_COUNT, count);
  return static_cast<int64_t>(hostinfo.max_mem);
}

int64_t AmountOfAvailablePhysicalMemory() {
  mach_port_t host = mach_host_self();
  vm_statistics_data_t vm_info;
  mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
  if (host_statistics(host, HOST_VM_INFO, reinterpret_cast<host_info_t>(&vm_info), &count) != KERN_SUCCESS) {
    NOTREACHED();
    return 0;
  }

  return static_cast<int64_t>(vm_info.free_count - vm_info.speculative_count) * PAGE_SIZE;
}
}  // namespace system_info
}  // namespace common
