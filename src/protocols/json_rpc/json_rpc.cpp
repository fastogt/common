#include <common/protocols/json_rpc/json_rpc.h>

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>

#include <common/convert2string.h>
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
Error GetJsonRPCRequest(json_object* rpc, JsonRPCRequest* result) {
  CHECK(rpc && result);

  json_object* jrpc = nullptr;
  json_bool jrpc_exists = json_object_object_get_ex(rpc, JSONRPC_FIELD, &jrpc);
  if (!jrpc_exists) {
    return make_error_inval();
  }

  json_object* jid = nullptr;
  json_bool jid_exists = json_object_object_get_ex(rpc, JSONRPC_ID_FIELD, &jid);
  if (!jid_exists) {
    return make_error_inval();
  }

  JsonRPCRequest res;
  if (json_object_get_type(jid) == json_type_null) {
    res.id = null_json_rpc_id;
  } else {
    res.id = json_object_get_string(jid);
  }

  json_object* jmethod = nullptr;
  json_bool jmethod_exists = json_object_object_get_ex(rpc, JSONRPC_METHOD_FIELD, &jmethod);
  if (!jmethod_exists) {
    return make_error_inval();
  }

  json_object* jparams = nullptr;
  json_bool jparams_exists = json_object_object_get_ex(rpc, JSONRPC_PARAMS_FIELD, &jparams);
  if (jparams_exists) {
    if (json_object_get_type(jparams) == json_type_null) {
      res.params = std::string();
    } else {
      res.params = std::string(json_object_get_string(jparams));
    }
  }

  res.method = json_object_get_string(jmethod);
  *result = res;
  return Error();
}

Error GetJsonRPCResponce(json_object* rpc, JsonRPCResponce* result) {
  CHECK(rpc && result);

  json_object* jrpc = nullptr;
  json_bool jrpc_exists = json_object_object_get_ex(rpc, JSONRPC_FIELD, &jrpc);
  if (!jrpc_exists) {
    return make_error_inval();
  }

  json_object* jid = nullptr;
  json_bool jid_exists = json_object_object_get_ex(rpc, JSONRPC_ID_FIELD, &jid);
  if (!jid_exists) {
    return make_error_inval();
  }

  JsonRPCResponce res;
  if (json_object_get_type(jid) == json_type_null) {
    res.id = null_json_rpc_id;
  } else {
    res.id = json_object_get_string(jid);
  }

  json_object* jerror = nullptr;
  json_bool jerror_exists = json_object_object_get_ex(rpc, JSONRPC_ERROR_FIELD, &jerror);
  if (jerror_exists && json_object_get_type(jerror) != json_type_null) {
    json_object* jerror_code = nullptr;
    json_bool jerror_code_exists = json_object_object_get_ex(jerror, JSONRPC_ERROR_CODE_FIELD, &jerror_code);

    json_object* jerror_message = nullptr;
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

  json_object* jresult = nullptr;
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

Error MakeJsonRPCRequest(const JsonRPCRequest& request, struct json_object** out_json) {
  if (!request.IsValid() || !out_json || *out_json) {
    return make_error_inval();
  }

  const char* method_ptr = request.method.c_str();
  json_object* command_json = json_object_new_object();
  json_object_object_add(command_json, JSONRPC_FIELD, json_object_new_string(JSONRPC_VERSION));
  json_object_object_add(command_json, JSONRPC_METHOD_FIELD, json_object_new_string(method_ptr));
  const char* jid_ptr = request.id.c_str();
  json_object_object_add(command_json, JSONRPC_ID_FIELD, json_object_new_string(jid_ptr));
  if (request.params) {
    std::string data = *request.params;
    const char* data_ptr = data.empty() ? nullptr : data.c_str();
    json_object* jparams = data_ptr ? json_tokener_parse(data_ptr) : nullptr;
    if (jparams) {
      json_object_object_add(command_json, JSONRPC_PARAMS_FIELD, jparams);
    } else {
      json_object_object_add(command_json, JSONRPC_PARAMS_FIELD, data_ptr ? json_object_new_string(data_ptr) : nullptr);
    }
  }

  *out_json = command_json;
  return Error();
}

Error MakeJsonRPCRequest(const JsonRPCRequest& request, std::string* out_json) {
  if (!out_json) {
    return make_error_inval();
  }

  struct json_object* jres = nullptr;
  Error err = MakeJsonRPCRequest(request, &jres);
  if (err) {
    return err;
  }

  *out_json = json_object_get_string(jres);
  json_object_put(jres);
  return Error();
}

Error ParseJsonRPCResponce(const std::string& data, JsonRPCResponce* result) {
  if (data.empty() || !result) {
    return make_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return make_error_inval();
  }

  Error err = GetJsonRPCResponce(jdata, result);
  json_object_put(jdata);
  return err;
}

Error MakeJsonRPCResponce(const JsonRPCResponce& responce, struct json_object** out_json) {
  if (!responce.IsValid() || !out_json || *out_json) {
    return make_error_inval();
  }

  json_rpc_id jid = responce.id;
  const char* jid_ptr = jid.c_str();
  json_object* command_json = json_object_new_object();
  json_object_object_add(command_json, JSONRPC_FIELD, json_object_new_string(JSONRPC_VERSION));
  json_object_object_add(command_json, JSONRPC_ID_FIELD, json_object_new_string(jid_ptr));
  if (responce.IsError()) {
    json_object* jerror = json_object_new_object();
    std::string error_str = responce.error->message;
    JsonRPCErrorCode ec = responce.error->code;
    const char* error_ptr = error_str.c_str();
    json_object_object_add(jerror, JSONRPC_ERROR_MESSAGE_FIELD, json_object_new_string(error_ptr));
    json_object_object_add(jerror, JSONRPC_ERROR_CODE_FIELD, json_object_new_int(ec));
    json_object_object_add(command_json, JSONRPC_ERROR_FIELD, jerror);
  } else if (responce.IsMessage()) {
    std::string message_str = responce.message->result;
    const char* message_ptr = message_str.c_str();
    json_object_object_add(command_json, JSONRPC_RESULT_FIELD, json_object_new_string(message_ptr));
  } else {
    NOTREACHED();
  }

  *out_json = command_json;
  return Error();
}

Error MakeJsonRPCResponce(const JsonRPCResponce& responce, std::string* out_json) {
  if (!out_json) {
    return make_error_inval();
  }

  struct json_object* jres = nullptr;
  Error err = MakeJsonRPCResponce(responce, &jres);
  if (err) {
    return err;
  }

  *out_json = json_object_get_string(jres);
  json_object_put(jres);
  return Error();
}

Error ParseJsonRPCRequest(const std::string& data, JsonRPCRequest* result) {
  if (data.empty() || !result) {
    return make_error_inval();
  }

  const char* data_ptr = data.c_str();
  json_object* jdata = json_tokener_parse(data_ptr);
  if (!jdata) {
    return make_error_inval();
  }

  Error err = GetJsonRPCRequest(jdata, result);
  json_object_put(jdata);
  return err;
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
