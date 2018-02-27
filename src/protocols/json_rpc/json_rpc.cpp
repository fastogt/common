#include <common/protocols/json_rpc/json_rpc.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <common/sprintf.h>
#include <common/system_info/system_info.h>  // for SystemInfo, etc

#define JSONRPC_FIELD "jsonrpc"
#define JSONRPC_METHOD_FIELD "method"
#define JSONRPC_PARAMS_FIELD "params"
#define JSONRPC_ID_FIELD "id"

#define JSONRPC_ERROR_FIELD "error"
#define JSONRPC_ERROR_CODE_FIELD "code"
#define JSONRPC_ERROR_MESSAGE_FIELD "message"
#define JSONRPC_RESULT_FIELD "result"

namespace common {
namespace protocols {
namespace json_rpc {

namespace {
Error GetJsonRpcResult(json_object* rpc, JsonRPCResult* result) {
  CHECK(rpc && result);

  json_object* jrpc = NULL;
  json_bool jrpc_exists = json_object_object_get_ex(rpc, JSONRPC_FIELD, &jrpc);
  if (!jrpc_exists) {
    return make_error_inval();
  }

  json_object* jid = NULL;
  json_bool jid_exists = json_object_object_get_ex(rpc, JSONRPC_ID_FIELD, &jid);
  if (!jid_exists) {
    return make_error_inval();
  }

  JsonRPCResult res;
  if (json_object_get_type(jid) == json_type_null) {
    res.id = "null";
  } else {
    res.id = json_object_get_string(jid);
  }

  json_object* jerror = NULL;
  json_bool jerror_exists = json_object_object_get_ex(rpc, JSONRPC_ERROR_FIELD, &jerror);
  if (jerror_exists && json_object_get_type(jerror) != json_type_null) {
    json_object* jerror_code = NULL;
    json_bool jerror_code_exists = json_object_object_get_ex(jerror, JSONRPC_ERROR_CODE_FIELD, &jerror_code);

    json_object* jerror_message = NULL;
    json_bool jerror_message_exists = json_object_object_get_ex(jerror, JSONRPC_ERROR_MESSAGE_FIELD, &jerror_message);
    if (jerror_message_exists && jerror_code_exists) {
      std::string error_str = json_object_get_string(jerror_message);
      JsonRPCErrorCode err_code = static_cast<JsonRPCErrorCode>(json_object_get_int(jerror_code));
      JsonRPCError jerr = {error_str, err_code};
      res.error = jerr;
      *result = res;
      return Error();
    }

    JsonRPCError jerr = {json_object_get_string(jerror), JSON_RPC_NOT_RFC_ERROR};
    res.error = jerr;
    *result = res;
    return Error();
  }

  json_object* jresult = NULL;
  json_bool jresult_exists = json_object_object_get_ex(rpc, JSONRPC_RESULT_FIELD, &jresult);
  if (!jresult_exists) {
    return make_error_inval();
  }

  JsonRPCMessage msg;
  msg.result = json_object_get_string(jresult);
  res.message = msg;
  *result = res;
  return Error();
}

}  // namespace

Error MakeJsonRPC(const std::string& method, struct json_object* param, struct json_object** out_json) {
  if (method.empty() || !out_json || *out_json) {
    return make_error_inval();
  }

  const char* method_ptr = method.c_str();
  json_object* command_json = json_object_new_object();
  json_object_object_add(command_json, JSONRPC_FIELD, json_object_new_string(JSONRPC_VERSION));
  json_object_object_add(command_json, JSONRPC_METHOD_FIELD, json_object_new_string(method_ptr));
  json_object_object_add(command_json, JSONRPC_ID_FIELD, NULL);
  if (param) {
    json_object_object_add(command_json, JSONRPC_PARAMS_FIELD, param);
  }

  *out_json = command_json;
  return Error();
}

Error ParseJsonRPC(const std::string& data, JsonRPCResult* result) {
  if (data.empty() || !result) {
    return common::make_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return common::make_error_inval();
  }

  Error err = GetJsonRpcResult(jdata, result);
  json_object_put(jdata);
  return err;
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
