/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#pragma once

#include <string>

#include <common/error.h>

#include <common/protocols/json_rpc/json_rpc_request.h>
#include <common/protocols/json_rpc/json_rpc_response.h>

struct json_object;

#define JSONRPC_VERSION "2.0"

namespace common {
namespace protocols {
namespace json_rpc {

Error MakeJsonRPCRequest(const JsonRPCRequest& request, struct json_object** out_json) WARN_UNUSED_RESULT;
Error MakeJsonRPCRequest(const JsonRPCRequest& request, std::string* out_json) WARN_UNUSED_RESULT;

Error ParseJsonRPCResponse(struct json_object* data, JsonRPCResponse* result) WARN_UNUSED_RESULT;
Error ParseJsonRPCResponse(const std::string& data, JsonRPCResponse* result) WARN_UNUSED_RESULT;

Error MakeJsonRPCResponse(const JsonRPCResponse& response, struct json_object** out_json) WARN_UNUSED_RESULT;
Error MakeJsonRPCResponse(const JsonRPCResponse& response, std::string* out_json) WARN_UNUSED_RESULT;

Error ParseJsonRPCRequest(struct json_object* data, JsonRPCRequest* result) WARN_UNUSED_RESULT;
Error ParseJsonRPCRequest(const std::string& data, JsonRPCRequest* result) WARN_UNUSED_RESULT;

Error ParseJsonRPC(const std::string& data,
                   JsonRPCRequest** result_req,
                   JsonRPCResponse** result_resp) WARN_UNUSED_RESULT;  // allocated memory

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
