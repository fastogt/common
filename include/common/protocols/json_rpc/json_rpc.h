#pragma once

#include <common/error.h>

#include <common/optional.h>

struct json_object;

#define JSONRPC_VERSION "2.0"

namespace common {
namespace protocols {
namespace json_rpc {

Error MakeJsonRPC(const std::string& method,
                  struct json_object* param,
                  struct json_object** out_json) WARN_UNUSED_RESULT;

// Json RPC error
enum JsonRPCErrorCode {
  JSON_RPC_PARSE_ERROR = -32700,       // Parse error
  JSON_RPC_INVALID_REQUEST = -32600,   // Invalid Request
  JSON_RPC_METHOD_NOT_FOUND = -32601,  // Method not found
  JSON_RPC_INVALID_PARAMS = -32602,    // Invalid params
  JSON_RPC_INTERNAL_ERROR = -32603,    // Internal error
  JSON_RPC_SERVER_ERROR = -32000,      // to -32099 	// Server error
  JSON_RPC_NOT_RFC_ERROR = -32001
};

struct JsonRPCError {
  std::string message;
  JsonRPCErrorCode code;
};

struct JsonRPCMessage {
  std::string result;
};

struct JsonRPCResult {
  JsonRPCResult();

  bool IsError() const;
  bool IsMessage() const;

  std::string id;
  Optional<JsonRPCMessage> message;
  Optional<JsonRPCError> error;
};

Error ParseJsonRPC(const std::string& data, JsonRPCResult* result) WARN_UNUSED_RESULT;

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
