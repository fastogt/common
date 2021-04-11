/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/license/utils.h>

#include <sys/stat.h>

#if defined(HAVE_UDEV)
#include <libudev.h>
#endif

#define MACHINE_ID_BUFF_SIZE 32

namespace common {
namespace license {

bool GetHddID(std::string* serial) {
  if (!serial) {
    return false;
  }

#if defined(HAVE_UDEV)
  struct stat stats;
  int err = stat("/", &stats);
  if (err < 0) {
    return false;
  }

  /* Create the udev object */
  struct udev* udev = udev_new();
  if (!udev) {
    return false;
  }

  char type = S_ISCHR(stats.st_mode) ? 'c' : 'b';

  /* Create a list of the devices in the 'hidraw' subsystem. */
  struct udev_device* device = udev_device_new_from_devnum(udev, type, stats.st_dev);
  if (!device) {
    udev_unref(udev);
    return false;
  }

#if 0
  struct udev_list_entry* list_entry = udev_device_get_properties_list_entry(device);
  for (struct udev_list_entry* it = list_entry; it != NULL; it = udev_list_entry_get_next(it)) {
    const char* name = udev_list_entry_get_name(it);
    const char* value = udev_list_entry_get_value(it);
    printf("%s:%s\n", name, value);
  }
#endif

  const char* serial_id = udev_device_get_property_value(device, "ID_SERIAL_SHORT");
  if (!serial_id) {
    serial_id = udev_device_get_property_value(device, "ID_FS_UUID");
  }

  if (serial_id) {
    *serial = serial_id;
  }

  udev_device_unref(device);
  udev_unref(udev);
  return serial_id != NULL;
#else
  return false;
#endif
}

bool GetMachineID(std::string* serial) {
  if (!serial) {
    return false;
  }

  FILE* machine_id_file = fopen("/etc/machine-id", "r");
  if (machine_id_file == NULL) {
    machine_id_file = fopen("/var/lib/dbus/machine-id", "r");
    if (machine_id_file == NULL) {
      return false;
    }
  }

  char* ptr = static_cast<char*>(calloc(MACHINE_ID_BUFF_SIZE, sizeof(char)));
  if (!ptr) {
    fclose(machine_id_file);
    return false;
  }

  size_t res = fread(ptr, sizeof(char), MACHINE_ID_BUFF_SIZE, machine_id_file);
  if (res == 0) {
    free(ptr);
    fclose(machine_id_file);
    return false;
  }
  *serial = std::string(ptr, res);
  free(ptr);
  fclose(machine_id_file);
  return true;
}

}  // namespace license
}  // namespace common
