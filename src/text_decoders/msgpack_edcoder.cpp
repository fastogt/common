/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <common/text_decoders/msgpack_edcoder.h>

#include <inttypes.h>  // for PRId64, PRIu64
#include <stddef.h>    // for size_t
#include <stdint.h>    // for uint8_t, uint16_t
#include <stdlib.h>    // for free, calloc
#include <string.h>    // for memcpy, memmove, memset, etc

#include <string>  // for string

#include <cmp/cmp.h>  // for cmp_object_t, etc

#include <common/convert2string.h>  // for ConvertToString

namespace {

bool read_bytes(void* data, size_t sz, char* ch) {
  size_t len = strlen(ch);
  bool is_comp = len >= sz;
  if (is_comp) {
    memcpy(data, ch, sz);
    memmove(ch, ch + sz, len - sz);
    ch[len - sz] = 0;
  } else {
    memcpy(data, ch, len);
    ch[0] = 0;
  }

  return is_comp;
}

bool stream_reader(cmp_ctx_t* ctx, void* data, size_t limit) {
  return read_bytes(data, limit, reinterpret_cast<char*>(ctx->buf));
}

size_t stream_writer(cmp_ctx_t* ctx, const void* data, size_t count) {
  std::string* out = static_cast<std::string*>(ctx->buf);
  out->append(static_cast<const char*>(data), count);
  return count;
}

}  // namespace

namespace common {
MsgPackEDcoder::MsgPackEDcoder() : IEDcoder(MsgPack) {}

Error MsgPackEDcoder::EncodeImpl(const std::string& data, std::string* out) {
  if (!out || data.empty()) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  cmp_ctx_t cmp;
  std::string lout;
  cmp_init(&cmp, &lout, NULL, stream_writer);
  bool res = cmp_write_str(&cmp, data.c_str(), data.size());
  if (!res) {
    return make_error_value("MsgPackEDcoder internal error!", ErrorValue::E_ERROR);
  }
  *out = lout;
  return Error();
}

Error MsgPackEDcoder::DecodeImpl(const std::string& data, std::string* out) {
  if (!out || data.empty()) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }
  cmp_ctx_t cmp;
  char* copy = reinterpret_cast<char*>(calloc(data.size() + 1, sizeof(char)));
  if (!copy) {
    return make_error_value("Memory allocation failed!", ErrorValue::E_ERROR);
  }

  memcpy(copy, data.c_str(), data.size());

  cmp_init(&cmp, copy, stream_reader, NULL);

  std::string lout;

  while (1) {
    cmp_object_t obj;

    if (!cmp_read_object(&cmp, &obj)) {
      if (copy[0] == 0) {
        break;
      }

      return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
    }

    char sbuf[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    switch (obj.type) {
      case CMP_TYPE_POSITIVE_FIXNUM:
      case CMP_TYPE_UINT8:
        lout += ConvertToString(obj.as.u8);
        break;
      case CMP_TYPE_FIXMAP:
      case CMP_TYPE_MAP16:
      case CMP_TYPE_MAP32:
        break;
      case CMP_TYPE_FIXARRAY:
      case CMP_TYPE_ARRAY16:
      case CMP_TYPE_ARRAY32:
        break;
      case CMP_TYPE_FIXSTR:
      case CMP_TYPE_STR8:
      case CMP_TYPE_STR16:
      case CMP_TYPE_STR32:
        if (!read_bytes(sbuf, obj.as.str_size, copy)) {
          free(copy);
          return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
        }
        sbuf[obj.as.str_size] = 0;
        lout += sbuf;
        break;
      case CMP_TYPE_BIN8:
      case CMP_TYPE_BIN16:
      case CMP_TYPE_BIN32:
        memset(sbuf, 0, sizeof(sbuf));
        if (!read_bytes(sbuf, obj.as.bin_size, copy)) {
          free(copy);
          return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
        }
        lout.append(sbuf, obj.as.bin_size);
        break;
      case CMP_TYPE_NIL:
        lout += "NULL";
        break;
      case CMP_TYPE_BOOLEAN:
        if (obj.as.boolean) {
          lout += "true";
        } else {
          lout += "false";
        }
        break;
      case CMP_TYPE_EXT8:
      case CMP_TYPE_EXT16:
      case CMP_TYPE_EXT32:
      case CMP_TYPE_FIXEXT1:
      case CMP_TYPE_FIXEXT2:
      case CMP_TYPE_FIXEXT4:
      case CMP_TYPE_FIXEXT8:
      case CMP_TYPE_FIXEXT16:
        if (obj.as.ext.type == 1) { /* Date object */
          uint16_t year = 0;
          uint8_t month = 0;
          uint8_t day = 0;
          if (!read_bytes(&year, sizeof(uint16_t), copy)) {
            free(copy);
            return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
          }

          if (!read_bytes(&month, sizeof(uint8_t), copy)) {
            free(copy);
            return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
          }

          if (!read_bytes(&day, sizeof(uint8_t), copy)) {
            free(copy);
            return make_error_value(cmp_strerror(&cmp), ErrorValue::E_ERROR);
          }

          char buff[32] = {0};
          SNPrintf(buff, sizeof(buff), "%u/%u/%u", year, month, day);
          lout += buff;
        } else {
          while (obj.as.ext.size--) {
            read_bytes(sbuf, sizeof(uint8_t), copy);
            char buff[32] = {0};
            SNPrintf(buff, sizeof(buff), "%02x ", sbuf[0]);
            lout += buff;
          }
        }
        break;
      case CMP_TYPE_FLOAT:
        lout += ConvertToString(obj.as.flt);
        break;
      case CMP_TYPE_DOUBLE:
        lout += ConvertToString(obj.as.dbl);
        break;
      case CMP_TYPE_UINT16:
        lout += ConvertToString(obj.as.u16);
        break;
      case CMP_TYPE_UINT32:
        lout += ConvertToString(obj.as.u32);
        break;
      case CMP_TYPE_UINT64: {
        char buff[32] = {0};
        SNPrintf(buff, sizeof(buff), "%" PRIu64 "", obj.as.u64);
        lout += buff;
        break;
      }
      case CMP_TYPE_NEGATIVE_FIXNUM:
      case CMP_TYPE_SINT8:
        lout += ConvertToString(obj.as.s8);
        break;
      case CMP_TYPE_SINT16:
        lout += ConvertToString(obj.as.s16);
        break;
      case CMP_TYPE_SINT32:
        lout += ConvertToString(obj.as.s32);
        break;
      case CMP_TYPE_SINT64: {
        char buff[32] = {0};
        SNPrintf(buff, sizeof(buff), "%" PRId64 "", obj.as.s64);
        lout += buff;
        break;
      }
      default: { return make_error_value(MemSPrintf("Unrecognized object type %u\n", obj.type), ErrorValue::E_ERROR); }
    }
  }

  *out = lout;
  free(copy);
  return Error();
}

}  // namespace common
