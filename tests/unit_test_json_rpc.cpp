/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

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

#define PARSE_ERROR_CODE -32700
#define PARSE_ERROR_STR "Parse error"
#define PARSE_ERROR                                                                                                  \
  "{\"jsonrpc\": " JSONRPC_VERSION                                                                                   \
  ", \"error\": {\"code\": " STRINGIZE(PARSE_ERROR_CODE) ", \"message\": \"" PARSE_ERROR_STR "\"}, \"id\": " NULL_ID \
                                                         "}"

void make_parse_commands(const std::string& method, const std::string& id, const std::string& params) {
  using namespace common::protocols::json_rpc;
  JsonRPCRequest req;
  req.method = method;
  req.id = id;
  req.params = params;
  common::Error err = MakeJsonRPCRequest(req, static_cast<json_object**>(NULL));  // inval
  ASSERT_TRUE(err);

  json_object* res = NULL;
  err = MakeJsonRPCRequest(req, &res);
  ASSERT_FALSE(err);
  std::string request_str = json_object_get_string(res);
  json_object_put(res);

  JsonRPCRequest req2;
  err = ParseJsonRPCRequest(request_str, &req2);
  ASSERT_FALSE(err);
  ASSERT_EQ(req, req2);

  ASSERT_EQ(req2.method, method);
  ASSERT_EQ(req2.id, id);
  ASSERT_EQ(req2.params, params);
}

TEST(json_rpc_request, notification) {
  using namespace common::protocols::json_rpc;
  JsonRPCRequest req = JsonRPCRequest::MakeNotification(METHOD);
  ASSERT_TRUE(req.IsNotification());
  ASSERT_TRUE(req.IsValid());
  ASSERT_FALSE(req.id);
  std::string req_str;
  common::Error err = MakeJsonRPCRequest(req, &req_str);
  ASSERT_FALSE(err);

  JsonRPCRequest req2;
  err = ParseJsonRPCRequest(req_str, &req2);
  ASSERT_FALSE(err);
  ASSERT_EQ(req, req2);
}

TEST(json_rpc_request, make_parse_commands) {
  using namespace common::protocols::json_rpc;
  make_parse_commands("test", *null_json_rpc_id, "hello");
  make_parse_commands("test1", "123", std::string());
  make_parse_commands("test2", "1235", "[ 1, 2, 3 ]");
  make_parse_commands("test3", "12355", "2");
  make_parse_commands("test4", "123556", "1.1");
  make_parse_commands("test5", "123557", "false");
  make_parse_commands("test6", "1235571", "null");
}

TEST(json_rpc_response, parse_result_19) {
  using namespace common::protocols::json_rpc;
  JsonRPCResponse result;
  common::Error err = ParseJsonRPCResponse(RESULT_19, &result);
  ASSERT_FALSE(err);
  ASSERT_TRUE(result.IsMessage());
  ASSERT_FALSE(result.IsError());
  ASSERT_EQ(STRINGIZE(RPC_ID), *result.id);
  auto mess = result.message;
  ASSERT_EQ(STR_19, mess->result);
  ASSERT_FALSE(result.error);
}

TEST(json_rpc_response, parse_method_non_exist) {
  using namespace common::protocols::json_rpc;
  JsonRPCResponse result;
  common::Error err = ParseJsonRPCResponse(METHOD_NON_EXISTS, &result);
  ASSERT_FALSE(err);
  ASSERT_FALSE(result.IsMessage());
  ASSERT_TRUE(result.IsError());
  ASSERT_EQ(RPC_ID_STR, *result.id);
  auto jerr = result.error;
  ASSERT_EQ(NOT_FOUND_STR, jerr->message);
  ASSERT_EQ(NOT_FOUND_ERROR_CODE, jerr->code);
  ASSERT_FALSE(result.message);
}

TEST(json_rpc_response, parse_method_invalid_request) {
  using namespace common::protocols::json_rpc;
  JsonRPCResponse result;
  common::Error err = ParseJsonRPCResponse(INVALID_REQUEST, &result);
  ASSERT_FALSE(err);
  ASSERT_FALSE(result.IsMessage());
  ASSERT_TRUE(result.IsError());
  ASSERT_EQ(NULL_ID, *result.id);
  auto jerr = result.error;
  ASSERT_EQ(INVALID_REQUEST_STR, jerr->message);
  ASSERT_EQ(INVALID_REQUEST_ERROR_CODE, jerr->code);
  ASSERT_FALSE(result.message);
}

TEST(json_rpc_response, parse_error) {
  using namespace common::protocols::json_rpc;
  JsonRPCResponse result;
  common::Error err = ParseJsonRPCResponse(PARSE_ERROR, &result);
  ASSERT_FALSE(err);
  ASSERT_FALSE(result.IsMessage());
  ASSERT_TRUE(result.IsError());
  ASSERT_EQ(NULL_ID, *result.id);
  auto jerr = result.error;
  ASSERT_EQ(PARSE_ERROR_STR, jerr->message);
  ASSERT_EQ(PARSE_ERROR_CODE, jerr->code);
  ASSERT_FALSE(result.message);
}
