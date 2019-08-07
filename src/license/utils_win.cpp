/*  Copyright (C) 2014-2018 FastoGT. All right reserved.
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

#include <fileapi.h>
#include <sysinfoapi.h>

#define UUID_SIZE 16
#define UUID_STRING_SIZE 36
typedef unsigned char uuid_t[UUID_SIZE];
typedef char uuid_string_t[UUID_STRING_SIZE];

#pragma pack(1)
struct smbios_prologue {
  BYTE calling_method;
  BYTE major_version;
  BYTE minor_version;
  BYTE revision;
  DWORD length;
};

struct smbios_header {
  BYTE type;
  BYTE length;
  WORD handle;
};

struct smbios_system {
  struct smbios_header hdr;
  BYTE vendor;
  BYTE product;
  BYTE version;
  BYTE serial;
  BYTE uuid[16];
};

#pragma pack()

#define RSMB (('R' << 24) | ('S' << 16) | ('M' << 8) | 'B')

namespace {

void uuid_unparse_lower(const uuid_t uu, char* out) {
  for (unsigned i = 0; i != UUID_SIZE; ++i) {
    unsigned char ch = uu[i];
    *out++ = "0123456789abcdef"[ch >> 4];
    *out++ = "0123456789abcdef"[ch & 0x0f];
    if ((0x2a8 >> i) & 1) {
      *out++ = '-';
    }
  }
  *out = '\0';
}
}  // namespace

namespace common {
namespace license {

bool GetHddID(std::string* serial) {
  if (!serial) {
    return false;
  }

  DWORD disk_serialINT;
  if (!GetVolumeInformationW(nullptr, nullptr, NULL, &disk_serialINT, nullptr, nullptr, nullptr, NULL)) {
    return false;
  }

  BYTE bytes[UUID_SIZE];
  memcpy(&bytes, &disk_serialINT, sizeof(bytes));

  uuid_string_t uuid_string;
  uuid_unparse_lower(bytes, uuid_string);

  *serial = uuid_string;
  return true;
}

bool GetMachineID(std::string* serial) {
  if (!serial) {
    return false;
  }

  static const BYTE none[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  const struct smbios_prologue* prologue;
  const struct smbios_header* hdr;
  const struct smbios_system* system;
  const BYTE* uuid = nullptr;

  size_t len = GetSystemFirmwareTable(RSMB, 0, nullptr, 0);
  if (len < sizeof(*prologue)) {
    return false;
  }

  char* buf = (char*)malloc(len);
  if (!buf) {
    return false;
  }

  GetSystemFirmwareTable(RSMB, 0, buf, len);
  prologue = (const struct smbios_prologue*)(buf);
  if (prologue->length < sizeof(*hdr)) {
    free(buf);
    return false;
  }

  const char* ptr;
  const char* start = (const char*)(prologue + 1);
  hdr = (const struct smbios_header*)start;

  for (;;) {
    if (uuid || (const char*)hdr - start >= prologue->length - sizeof(*hdr)) {
      break;
    }

    if (!hdr->length) {
      break;
    }

    switch (hdr->type) {
      case 1: /* system entry */
        if (hdr->length < sizeof(*system) || (const char*)hdr - start + hdr->length > prologue->length)
          break;
        system = (const struct smbios_system*)hdr;
        uuid = system->uuid;
        break;

      default:                                                                     /* skip other entries */
        for (ptr = (const char*)hdr + hdr->length; *ptr; ptr += strlen(ptr) + 1) { /* nothing */
        }
        if (ptr == (const char*)hdr + hdr->length) {
          ptr++;
        }

        hdr = (const struct smbios_header*)(ptr + 1);
        break;
    }
  }

  if (!uuid || !memcmp(uuid, none, sizeof(none))) {
    free(buf);
    return false;
  }

  uuid_string_t uuid_string;
  uuid_unparse_lower(uuid, uuid_string);

  *serial = uuid_string;
  free(buf);
  return true;
}

}  // namespace license
}  // namespace common
