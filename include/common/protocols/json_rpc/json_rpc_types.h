#pragma once

#include <memory>
#include <string>

namespace common {
namespace protocols {
namespace json_rpc {

typedef std::string json_rpc_id;  // null or digits
extern const json_rpc_id invalid_json_rpc_id;
extern const json_rpc_id null_json_rpc_id;

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
