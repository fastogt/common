#pragma once

#include <common/protocols/json_rpc/json_rpc_types.h>

namespace common {
namespace protocols {
namespace json_rpc {

typedef std::string json_rpc_method;
extern const json_rpc_method invalid_json_rpc_method;

struct JsonRPCRequest {
  JsonRPCRequest();
  bool IsValid() const;

  json_rpc_id id;
  std::string method;
};

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
