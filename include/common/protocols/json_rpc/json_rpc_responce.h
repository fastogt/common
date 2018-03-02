#pragma once

#include <common/optional.h>
#include <common/protocols/json_rpc/json_rpc_types.h>

namespace common {
namespace protocols {
namespace json_rpc {

// Json RPC error
enum JsonRPCErrorCode : int {
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

typedef Optional<JsonRPCMessage> json_rpc_message;
typedef Optional<JsonRPCError> json_rpc_error;

// should be message or error
struct JsonRPCResponce {
  JsonRPCResponce();

  static JsonRPCResponce MakeError(json_rpc_id jid, json_rpc_error error);
  static JsonRPCResponce MakeMessage(json_rpc_id jid, json_rpc_message msg);

  bool IsError() const;
  bool IsMessage() const;
  bool IsValid() const;

  json_rpc_id id;
  json_rpc_message message;
  json_rpc_error error;
};

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
