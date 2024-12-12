/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/libev/io_loop_observer.h>
#include <common/libev/websocket/websocket_client.h>
#include <common/libev/websocket/websocket_server.h>

#include <common/threads/thread_manager.h>

#include <common/convert2string.h>
#include <common/net/net.h>

#define BUF_SIZE 4096
#define HOST "127.0.0.1"
#define ROUTE "/updates"
#define KEY "0019c93c6b0000beea359300003e9f947b01904e583aba03009cc80bd909d0b13ead040ab04d61cd824703e9eafd3d470"
#define API_KEY_PARAM "API-KEY"
namespace {
const common::uri::GURL ws_url("ws://" HOST ":8011" ROUTE "?" API_KEY_PARAM "=" KEY);
const common::net::HostAndPort g_hs(HOST, 8011);
const common::libev::http::HttpServerInfo kHinf(PROJECT_NAME_TITLE, PROJECT_DOMAIN);
}  // namespace

class ServerWebsockHandler : public common::libev::IoLoopObserver {
  const common::libev::http::HttpServerInfo info_;

 public:
  explicit ServerWebsockHandler(const common::libev::http::HttpServerInfo& info) : info_(info) {}

  ~ServerWebsockHandler() {}

  const common::libev::http::HttpServerInfo& info() const { return info_; }

  void PreLooped(common::libev::IoLoop* server) override {
    /*common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(server);
    common::net::socket_info sc;
    common::ErrnoError errn = common::net::connect(kHostAndPort, common::net::ST_SOCK_STREAM, nullptr, &sc);
    ASSERT_FALSE(errn);

    common::libev::websocket::WebSocketServerClient* cl = new common::libev::websocket::WebSocketServerClient(sserver,
    sc); ignore_result(cl->SetBlocking(false)); ignore_result(sserver->RegisterClient(cl)); errn =
    cl->StartHandshake(ws_url, kHinf); ASSERT_FALSE(errn);*/
  }

  void Accepted(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_TRUE(cl.empty());
  }

  void Moved(common::libev::IoLoop* server, common::libev::IoClient* client) override {
    UNUSED(server);
    UNUSED(client);
  }

  void Closed(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_EQ(cl.size(), 1);
  }

  void TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) override {
    UNUSED(server);
    UNUSED(id);
  }

  void Accepted(common::libev::IoChild* child) override { UNUSED(child); }
  void Moved(common::libev::IoLoop* server, common::libev::IoChild* child) override {
    UNUSED(server);
    UNUSED(child);
  }

  void ChildStatusChanged(common::libev::IoChild* child, int status, int signal) override {
    UNUSED(child);
    UNUSED(status);
    UNUSED(signal);
  }

  void DataReceived(common::libev::IoClient* client) override {
    common::libev::websocket::WebSocketServerClient* wclient =
        dynamic_cast<common::libev::websocket::WebSocketServerClient*>(client);
    if (wclient) {
      auto step = wclient->Step();
      if (step == common::libev::websocket::ZERO) {
        char buff[BUF_SIZE] = {0};
        size_t nread = 0;
        common::ErrnoError errn = client->SingleRead(buff, BUF_SIZE, &nread);
        if (errn || nread == 0) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        auto data = std::string(buff, nread);
        common::http::HttpRequest req;
        auto status_and_err = common::http::parse_http_request(data, &req);
        if (status_and_err.second) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        common::http::header_t head;
        if (!req.FindHeaderByKey("Sec-WebSocket-Key", false, &head)) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        common::http::header_t found_key_in_headers;
        if (!req.FindHeaderByKey(API_KEY_PARAM, false, &found_key_in_headers)) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        if (found_key_in_headers.value != KEY) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        auto url = req.GetURL();
        auto route = url.path();
        if (route != ROUTE) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        if (!url.has_query()) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        auto query_str = url.query();
        common::uri::Component key, value;
        common::uri::Component query(0, query_str.length());
        common::Optional<std::string> found_key_in_params;
        while (common::uri::ExtractQueryKeyValue(query_str.c_str(), &query, &key, &value)) {
          std::string key_string(query_str.substr(key.begin, key.len));
          std::string param_text(query_str.substr(value.begin, value.len));
          if (common::EqualsASCII(key_string, API_KEY_PARAM, false)) {
            found_key_in_params = param_text;
            break;
          }
        }

        if (!found_key_in_params) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        if (*found_key_in_params != KEY) {
          ignore_result(client->Close());
          delete client;
          return;
        }

        common::http::headers_t extra_headers = {{"Access-Control-Allow-Origin", "*"}};
        common::ErrnoError err = wclient->SendSwitchProtocolsResponse(head.value, extra_headers, info());
        ASSERT_FALSE(err);
        return;
      }

      auto print = [wclient](char* data, size_t len) -> void { wclient->SendFrame(data, len); };

      common::ErrnoError errn = wclient->ProcessFrame(print);
      if (errn) {
        ignore_result(client->Close());
        delete client;
        return;
      }
    }
  }

  void DataReadyToWrite(common::libev::IoClient* client) override { UNUSED(client); }

  void PostLooped(common::libev::IoLoop* server) override {
    std::vector<common::libev::IoClient*> cl = server->GetClients();
    ASSERT_TRUE(cl.empty());
  }
};

void DanceAndExitWebServer(common::libev::tcp::TcpServer* ser) {
  common::threads::PlatformThread::Sleep(1000);
  common::net::socket_info sc;
  common::ErrnoError errn = common::net::connect(g_hs, common::net::ST_SOCK_STREAM, nullptr, &sc);
  ASSERT_FALSE(errn);

  common::libev::websocket::WebSocketClient* cl = new common::libev::websocket::WebSocketClient(ser, sc);
  common::http::headers_t extra_headers = {{API_KEY_PARAM, KEY}};
  errn = cl->StartHandshake(ws_url, extra_headers, kHinf);
  ASSERT_FALSE(errn);
  common::threads::PlatformThread::Sleep(1000);
  errn = cl->SendFrame("hello", SIZEOFMASS("hello") - 1);
  ASSERT_FALSE(errn);
  common::threads::PlatformThread::Sleep(1000);
  ser->Stop();
}

TEST(Libev, Websoсket) {
  ServerWebsockHandler hand(kHinf);
  auto sock = new common::net::ServerSocketEvTcp(g_hs);
  common::libev::websocket::WebSocketServer* serv = new common::libev::websocket::WebSocketServer(sock, false, &hand);
  common::ErrnoError err = serv->Bind(true);
  ASSERT_FALSE(err);

  err = serv->Listen(5);
  ASSERT_FALSE(err);

  auto tp = THREAD_MANAGER()->CreateThread(&DanceAndExitWebServer, serv);
  GTEST_ASSERT_EQ(tp->GetHandle(), common::threads::invalid_thread_handle());
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  tp->Join();
  delete serv;
}
