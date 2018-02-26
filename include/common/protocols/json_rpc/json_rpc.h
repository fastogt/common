#pragma once

#include <common/protocols/json_rpc/json_rpc_error.h>

struct json_object;

#define JSONRPC_VERSION "2.0"

namespace common {
namespace protocols {
namespace json_rpc {

Error MakeJsonRPC(const std::string& method,
                  struct json_object* param,
                  struct json_object** out_json) WARN_UNUSED_RESULT;

JsonRPCError ParseJsonRPC(const std::string& data, std::string* result) WARN_UNUSED_RESULT;

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
