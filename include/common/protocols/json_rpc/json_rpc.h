#pragma once

#include <common/protocols/json_rpc/json_rpc_error.h>

struct json_object;

namespace common {
namespace protocols {
namespace json_rpc {

Error MakeCommand(const std::string& method,
                  struct json_object* param,
                  struct json_object** out_json) WARN_UNUSED_RESULT;

JsonRPCError ParseResponce(const std::string& data, std::string* result) WARN_UNUSED_RESULT;

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
