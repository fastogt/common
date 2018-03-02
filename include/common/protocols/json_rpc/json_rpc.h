#pragma once

#include <common/error.h>

#include <common/protocols/json_rpc/json_rpc_request.h>
#include <common/protocols/json_rpc/json_rpc_responce.h>

struct json_object;

#define JSONRPC_VERSION "2.0"

namespace common {
namespace protocols {
namespace json_rpc {

Error MakeJsonRPCRequest(const JsonRPCRequest& request,
                         struct json_object* param,
                         struct json_object** out_json) WARN_UNUSED_RESULT;

Error ParseJsonRPCResponce(const std::string& data, JsonRPCResponce* result) WARN_UNUSED_RESULT;

Error MakeJsonRPCResponce(const std::string& method,
                          const JsonRPCResponce& responce,
                          struct json_object** out_json) WARN_UNUSED_RESULT;

Error ParseJsonRPCRequest(const std::string& data, JsonRPCRequest* result) WARN_UNUSED_RESULT;

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
