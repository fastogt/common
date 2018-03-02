#include <common/protocols/json_rpc/json_rpc_responce.h>

namespace common {
namespace protocols {
namespace json_rpc {

JsonRPCResponce::JsonRPCResponce() : id(invalid_json_rpc_id), message(), error() {}

JsonRPCResponce MakeError(json_rpc_id jid, json_rpc_error error) {
  JsonRPCResponce resp;
  resp.id = jid;
  resp.error = error;
  CHECK(resp.IsValid() && resp.IsError()) << "JsonRPCResponce should be valid.";
  return resp;
}

JsonRPCResponce MakeMessage(json_rpc_id jid, json_rpc_message msg) {
  JsonRPCResponce resp;
  resp.id = jid;
  resp.message = msg;
  CHECK(resp.IsValid() && resp.IsMessage()) << "JsonRPCResponce should be valid.";
  return resp;
}

bool JsonRPCResponce::IsError() const {
  if (id == invalid_json_rpc_id) {
    return false;
  }

  if (message) {
    return false;
  }

  if (error) {
    return true;
  }

  return false;
}

bool JsonRPCResponce::IsMessage() const {
  if (id == invalid_json_rpc_id) {
    return false;
  }

  if (error) {
    return false;
  }

  if (message) {
    return true;
  }

  return false;
}

bool JsonRPCResponce::IsValid() const {
  if (id == invalid_json_rpc_id) {
    return false;
  }

  if (error && message) {
    return false;
  }

  return true;
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
