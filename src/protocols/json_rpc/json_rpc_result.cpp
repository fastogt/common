#include <common/protocols/json_rpc/json_rpc_result.h>

namespace common {
namespace protocols {
namespace json_rpc {

JsonRPCResult::JsonRPCResult() : id(), message(), error() {}

bool JsonRPCResult::IsError() const {
  if (error) {
    return true;
  }

  return false;
}

bool JsonRPCResult::IsMessage() const {
  if (IsError()) {
    return false;
  }

  if (message) {
    return true;
  }

  return false;
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
