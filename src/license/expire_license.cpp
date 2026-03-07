/*  Copyright (C) 2014-2022 FastoGT. All right reserved.
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

#include <common/license/expire_license.h>
#include <common/license/hardware_hash.h>

#include <openssl/evp.h>

namespace common {
namespace license {

namespace {

// Ed25519 public key (32 bytes) for license verification
const unsigned char kPublicKey[32] = {
    0x27, 0x87, 0xb8, 0x28, 0x06, 0xfd, 0x12, 0x40, 0x88, 0xf4, 0x4f, 0x99, 0x14, 0x94, 0xf8, 0xae,
    0x2c, 0x96, 0x70, 0x7d, 0x48, 0xe1, 0xbc, 0xb8, 0x4a, 0x02, 0x22, 0x83, 0x02, 0x5b, 0x57, 0x19};

const size_t kPayloadSize = 73;  // algo(1) + expiry(8) + signature(64)
const size_t kSignatureSize = 64;

// base64url decode (no padding)
bool Base64UrlDecode(const char* input, size_t input_len, unsigned char* output, size_t* output_len) {
  // Convert base64url to standard base64
  std::string b64;
  b64.reserve(input_len + 4);
  for (size_t i = 0; i < input_len; ++i) {
    char c = input[i];
    if (c == '-') {
      b64 += '+';
    } else if (c == '_') {
      b64 += '/';
    } else {
      b64 += c;
    }
  }
  // Add padding
  while (b64.size() % 4 != 0) {
    b64 += '=';
  }

  EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
  if (!ctx) {
    return false;
  }

  EVP_DecodeInit(ctx);
  int out_len = 0;
  int tmp_len = 0;
  int rc = EVP_DecodeUpdate(ctx, output, &out_len, reinterpret_cast<const unsigned char*>(b64.data()),
                            static_cast<int>(b64.size()));
  if (rc < 0) {
    EVP_ENCODE_CTX_free(ctx);
    return false;
  }

  rc = EVP_DecodeFinal(ctx, output + out_len, &tmp_len);
  EVP_ENCODE_CTX_free(ctx);
  if (rc < 0) {
    return false;
  }

  *output_len = static_cast<size_t>(out_len + tmp_len);
  return true;
}

bool VerifyEd25519(const unsigned char* message, size_t msg_len, const unsigned char* signature, size_t sig_len) {
  EVP_PKEY* pkey = EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519, nullptr, kPublicKey, sizeof(kPublicKey));
  if (!pkey) {
    return false;
  }

  EVP_MD_CTX* md_ctx = EVP_MD_CTX_new();
  if (!md_ctx) {
    EVP_PKEY_free(pkey);
    return false;
  }

  bool ok = false;
  if (EVP_DigestVerifyInit(md_ctx, nullptr, nullptr, nullptr, pkey) == 1) {
    if (EVP_DigestVerify(md_ctx, signature, sig_len, message, msg_len) == 1) {
      ok = true;
    }
  }

  EVP_MD_CTX_free(md_ctx);
  EVP_PKEY_free(pkey);
  return ok;
}

uint64_t ReadBigEndianUint64(const unsigned char* data) {
  uint64_t val = 0;
  for (int i = 0; i < 8; ++i) {
    val = (val << 8) | data[i];
  }
  return val;
}

}  // namespace

bool IsValidExpireKey(const std::string& project, const expire_key_t& expire_key) {
  time64_t res;
  if (!GetExpireTimeFromKey(project, expire_key, &res)) {
    return false;
  }

  time64_t utc_msec = time::current_utc_mstime();
  if (utc_msec > res) {
    return false;
  }
  return true;
}

bool GetExpireTimeFromKey(const std::string& project, const expire_key_t& expire_key, time64_t* time) {
  if (project.empty() || !time) {
    return false;
  }

  // Base64url decode the key
  unsigned char payload[kPayloadSize + 16];  // extra space for safety
  size_t payload_len = 0;
  if (!Base64UrlDecode(expire_key.data(), expire_key_t::license_size, payload, &payload_len)) {
    return false;
  }

  if (payload_len != kPayloadSize) {
    return false;
  }

  // Extract fields: algo(1) + expiry(8) + signature(64)
  ALGO_TYPE algo = static_cast<ALGO_TYPE>(payload[0]);
  if (algo != HDD && algo != MACHINE_ID) {
    return false;
  }

  const unsigned char* expiry_bytes = payload + 1;
  const unsigned char* signature = payload + 9;

  // Verify hardware hash matches this machine
  hardware_hash_t hw_hash;
  if (!GenerateHardwareHash(algo, &hw_hash)) {
    return false;
  }

  // Build signed message: project + hwHash + expiryBytes
  std::string msg;
  msg.reserve(project.size() + hardware_hash_t::license_size + 8);
  msg.append(project);
  msg.append(hw_hash.data(), hardware_hash_t::license_size);
  msg.append(reinterpret_cast<const char*>(expiry_bytes), 8);

  // Verify Ed25519 signature
  if (!VerifyEd25519(reinterpret_cast<const unsigned char*>(msg.data()), msg.size(), signature, kSignatureSize)) {
    return false;
  }

  // Extract expiry time
  *time = static_cast<time64_t>(ReadBigEndianUint64(expiry_bytes));
  return true;
}

}  // namespace license
}  // namespace common