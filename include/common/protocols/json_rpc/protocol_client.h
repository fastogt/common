/*  Copyright (C) 2014-2019 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <common/libev/io_client.h>
#include <common/protocols/json_rpc/json_rpc.h>
#include <common/text_decoders/iedcoder.h>

#include <limits>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace common {
namespace protocols {
namespace json_rpc {

typedef uint32_t protocoled_size_t;  // sizeof 4 byte
enum : protocoled_size_t { MAX_COMMAND_LENGTH = 64 * 1024 * 1024 };
COMPILE_ASSERT(std::numeric_limits<protocoled_size_t>::max() >= MAX_COMMAND_LENGTH,
               "Protocoled packet size should be greater MAX_COMMAND_LENGTH");

namespace detail {
ErrnoError WriteRequest(libev::IoClient* client,
                        IEDcoder* compressor,
                        const JsonRPCRequest& request) WARN_UNUSED_RESULT;
ErrnoError WriteResponse(libev::IoClient* client,
                         IEDcoder* compressor,
                         const JsonRPCResponse& response) WARN_UNUSED_RESULT;
ErrnoError ReadCommand(libev::IoClient* client, IEDcoder* compressor, std::string* out) WARN_UNUSED_RESULT;
}  // namespace detail

template <typename Client>
class ProtocolClient : public Client {
  COMPILE_ASSERT((std::is_base_of<libev::IoClient, Client>::value), "Client must derive from libev::IoClient");

 public:
  typedef Client base_class;
  typedef std::function<void(const JsonRPCRequest* request, const JsonRPCResponse* response)> callback_t;
  typedef std::pair<JsonRPCRequest, callback_t> request_save_entry_t;
  typedef std::shared_ptr<IEDcoder> compressor_t;

  template <typename... Args>
  explicit ProtocolClient(compressor_t compressor, Args... args)
      : base_class(args...), compressor_(compressor), id_(0) {}

  ErrnoError WriteRequest(const JsonRPCRequest& request, callback_t cb = callback_t()) WARN_UNUSED_RESULT {
    // INFO_LOG() << "WriteRequest: " << request.ToString();
    ErrnoError err = detail::WriteRequest(this, compressor_.get(), request);
    if (!err && !request.IsNotification()) {
      requests_queue_[request.id] = std::make_pair(request, cb);
    }
    return err;
  }

  ErrnoError WriteResponse(const JsonRPCResponse& response) WARN_UNUSED_RESULT {
    // INFO_LOG() << "WriteResponse: " << response.ToString();
    return detail::WriteResponse(this, compressor_.get(), response);
  }

  ErrnoError ReadCommand(std::string* out) WARN_UNUSED_RESULT {
    return detail::ReadCommand(this, compressor_.get(), out);
  }

  bool PopRequestByID(json_rpc_id sid, JsonRPCRequest* req, callback_t* cb = nullptr) {
    if (!req || !sid) {
      return false;
    }

    auto found_it = requests_queue_.find(sid);
    if (found_it == requests_queue_.end()) {
      return false;
    }

    request_save_entry_t it = found_it->second;
    *req = it.first;
    if (cb) {
      *cb = it.second;
    }
    requests_queue_.erase(found_it);
    return true;
  }

  size_t GetRequstSizeQueue() const { return requests_queue_.size(); }

 protected:
  json_rpc_id NextRequestID() {
    const seq_id_t next_id = id_++;
    return MakeRequestID(next_id);
  }

 private:
  const compressor_t compressor_;
  std::map<json_rpc_id, request_save_entry_t> requests_queue_;
  seq_id_t id_;
  using Client::Read;
  using Client::Write;
};

}  // namespace json_rpc
}  // namespace protocols
}  // namespace common
