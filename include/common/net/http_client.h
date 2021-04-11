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

#include <common/file_system/path.h>
#include <common/http/http.h>
#include <common/net/isocket.h>
#include <common/net/types.h>
#include <common/uri/gurl.h>

namespace common {
namespace net {

class IHttpClient {
 public:
  typedef std::string url_t;
  virtual ErrnoError Connect(struct timeval* tv = nullptr) WARN_UNUSED_RESULT = 0;
  virtual bool IsConnected() const = 0;
  virtual ErrnoError Disconnect() WARN_UNUSED_RESULT = 0;
  virtual HostAndPort GetHost() const = 0;
  virtual ErrnoError SendFile(descriptor_t file_fd, size_t file_size) WARN_UNUSED_RESULT = 0;

  Error PostFile(const url_t& path, const file_system::ascii_file_string_path& file_path) WARN_UNUSED_RESULT;
  Error Get(const url_t& path) WARN_UNUSED_RESULT;
  Error Head(const url_t& path) WARN_UNUSED_RESULT;

  Error ReadResponse(http::HttpResponse* response) WARN_UNUSED_RESULT;
  virtual ~IHttpClient();

 protected:
  explicit IHttpClient(net::ISocket* sock);
  net::ISocket* GetSocket() const;

 private:
  Error SendRequest(const http::HttpRequest& request_headers) WARN_UNUSED_RESULT;

  net::ISocket* sock_;
  Optional<http::HttpRequest> last_request_;
};

class HttpClient : public IHttpClient {
 public:
  explicit HttpClient(const HostAndPort& host);
  ErrnoError Connect(struct timeval* tv = nullptr) override;
  bool IsConnected() const override;
  ErrnoError Disconnect() override;
  ErrnoError SendFile(descriptor_t file_fd, size_t file_size) override;

  HostAndPort GetHost() const override;
};

Error PostHttpFile(const file_system::ascii_file_string_path& file_path, const uri::GURL& url);

class HttpsClient : public common::net::IHttpClient {
 public:
  typedef common::net::IHttpClient base_class;
  explicit HttpsClient(const common::net::HostAndPort& host);
  common::ErrnoError Connect(struct timeval* tv = nullptr) override;
  bool IsConnected() const override;
  common::ErrnoError Disconnect() override;
  common::net::HostAndPort GetHost() const override;
  ErrnoError SendFile(descriptor_t file_fd, size_t file_size) override;
};

Error PostHttpsFile(const file_system::ascii_file_string_path& file_path, const uri::GURL& url);

}  // namespace net
}  // namespace common
