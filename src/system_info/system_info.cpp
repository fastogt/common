#include <common/system_info/system_info.h>

#include <common/patterns/singleton_pattern.h>

namespace common {
namespace system_info {

namespace {

struct CurrentSystemInfo {
  CurrentSystemInfo() : info(OperatingSystemName(), OperatingSystemVersion(), OperatingSystemArchitecture()) {}

  const SystemInfo info;

  static CurrentSystemInfo* instance() { return &common::patterns::LazySingleton<CurrentSystemInfo>::Instance(); }
};

}  // namespace

SystemInfo::SystemInfo(const std::string& name, const std::string& version, const std::string& arch)
    : name_(name), version_(version), arch_(arch) {}

const std::string& SystemInfo::name() const {
  return name_;
}

const std::string& SystemInfo::version() const {
  return version_;
}

const std::string& SystemInfo::arch() const {
  return arch_;
}

const SystemInfo& currentSystemInfo() {
  return CurrentSystemInfo::instance()->info;
}

}  // namespace system_info
}  // namespace common