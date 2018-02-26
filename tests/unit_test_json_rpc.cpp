#include <gtest/gtest.h>

#include <json-c/json_object.h>

#include <common/protocols/json_rpc/json_rpc.h>

#define METHOD "test"

#define RPC_ID 1
#define STR_19 "19"
#define RESULT_19 "{\"jsonrpc\": \"" JSONRPC_VERSION "\", \"result\": " STR_19 ", \"id\": " STRINGIZE(RPC_ID) "}"

#define RPC_ID_STR "2"
#define NOT_FOUND_STR "Method not found"
#define NOT_FOUND_ERROR_CODE -32601
#define METHOD_NON_EXISTS                                                                        \
  "{\"jsonrpc\": \"" JSONRPC_VERSION                                                             \
  "\", \"error\": {\"code\": " STRINGIZE(NOT_FOUND_ERROR_CODE) ", \"message\": \"" NOT_FOUND_STR \
                                                               "\"}, \"id\": \"" RPC_ID_STR "\" }"

#define INVALID_REQUEST_STR "Invalid Request"
#define INVALID_REQUEST_ERROR_CODE -32600
#define NULL_ID "null"
#define INVALID_REQUEST                                                                                      \
  "{\"jsonrpc\": \"" JSONRPC_VERSION                                                                         \
  "\", \"error\": {\"code\": " STRINGIZE(INVALID_REQUEST_ERROR_CODE) ", \"message\": \"" INVALID_REQUEST_STR \
                                                                     "\"}, \"id\": " NULL_ID "}"

TEST(json_rpc, make_command) {
  using namespace common::protocols::json_rpc;
  common::Error err = MakeJsonRPC(METHOD, NULL, NULL);
  ASSERT_TRUE(err);

  json_object* res = NULL;
  err = MakeJsonRPC(METHOD, NULL, &res);
  ASSERT_FALSE(err);
  json_object_put(res);
}

TEST(json_rpc, parse_result_19) {
  using namespace common::protocols::json_rpc;
  JsonRPCResult result;
  common::Error err = ParseJsonRPC(RESULT_19, &result);
  ASSERT_FALSE(err);
  ASSERT_TRUE(result.IsMessage());
  ASSERT_FALSE(result.IsError());
  ASSERT_EQ(STRINGIZE(RPC_ID), result.id);
  auto mess = result.message;
  ASSERT_EQ(STR_19, mess->result);
  ASSERT_FALSE(result.error);
}

TEST(json_rpc, parse_method_non_exist) {
  using namespace common::protocols::json_rpc;
  JsonRPCResult result;
  common::Error err = ParseJsonRPC(METHOD_NON_EXISTS, &result);
  ASSERT_FALSE(err);
  ASSERT_FALSE(result.IsMessage());
  ASSERT_TRUE(result.IsError());
  ASSERT_EQ(RPC_ID_STR, result.id);
  auto jerr = result.error;
  ASSERT_EQ(NOT_FOUND_STR, jerr->message);
  ASSERT_EQ(NOT_FOUND_ERROR_CODE, jerr->code);
  ASSERT_FALSE(result.message);
}

TEST(json_rpc, parse_method_invalid_request) {
  using namespace common::protocols::json_rpc;
  JsonRPCResult result;
  common::Error err = ParseJsonRPC(INVALID_REQUEST, &result);
  ASSERT_FALSE(err);
  ASSERT_FALSE(result.IsMessage());
  ASSERT_TRUE(result.IsError());
  ASSERT_EQ(NULL_ID, result.id);
  auto jerr = result.error;
  ASSERT_EQ(INVALID_REQUEST_STR, jerr->message);
  ASSERT_EQ(INVALID_REQUEST_ERROR_CODE, jerr->code);
  ASSERT_FALSE(result.message);
}
