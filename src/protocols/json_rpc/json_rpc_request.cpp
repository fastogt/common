#include <common/protocols/json_rpc/json_rpc_request.h>

namespace common {
namespace protocols {
namespace json_rpc {

const json_rpc_method invalid_json_rpc_method = json_rpc_method();

JsonRPCRequest::JsonRPCRequest() : id(invalid_json_rpc_id), method(invalid_json_rpc_method) {}

bool JsonRPCRequest::IsValid() const {
  if (id == invalid_json_rpc_id) {
    return false;
  }

  if (method == invalid_json_rpc_method) {
    return false;
  }

  return true;
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
