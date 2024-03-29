/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/protocols/json_rpc/json_rpc_types.h>

#include <string>

namespace common {
namespace protocols {
namespace json_rpc {

typedef std::string json_rpc_method;
extern const json_rpc_method invalid_json_rpc_method;

typedef Optional<std::string> json_rpc_request_params;

struct JsonRPCRequest {
  JsonRPCRequest();

  static JsonRPCRequest MakeNotification(json_rpc_method method,
                                         json_rpc_request_params params = json_rpc_request_params());

  bool IsValid() const;
  bool IsRequest() const;
  bool IsNotification() const;
  bool Equals(const JsonRPCRequest& req) const;

  std::string ToString() const;

  json_rpc_id id;
  json_rpc_method method;
  json_rpc_request_params params;
};

inline bool operator==(const JsonRPCRequest& left, const JsonRPCRequest& right) {
  return left.Equals(right);
}

inline bool operator!=(const JsonRPCRequest& left, const JsonRPCRequest& right) {
  return !(left == right);
}

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
