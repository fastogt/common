#pragma once

#include <common/optional.h>
#include <common/protocols/json_rpc/json_rpc_types.h>

namespace common {
namespace protocols {
namespace json_rpc {

typedef std::string json_rpc_method;
extern const json_rpc_method invalid_json_rpc_method;

typedef Optional<std::string> json_rpc_request_params;

struct JsonRPCRequest {
  JsonRPCRequest();

  bool IsValid() const;
  bool Equals(const JsonRPCRequest& req) const;

  json_rpc_id id;
  json_rpc_method method;
  json_rpc_request_params params;
};

inline bool operator==(const JsonRPCRequest& left, const JsonRPCRequest& right) {
  return left.Equals(right);
}

inline bool operator!=(const JsonRPCRequest& left, const JsonRPCRequest& right) {
  return !(left == right);
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
