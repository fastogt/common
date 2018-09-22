#include <gtest/gtest.h>

#include <common/libev/io_loop_observer.h>
#include <common/libev/websocket/websocket_client.h>
#include <common/libev/websocket/websocket_server.h>

#include <common/threads/thread_manager.h>

#include <common/convert2string.h>
#include <common/net/net.h>

#define BUF_SIZE 4096

namespace {
const char kHost[] = "demos.kaazing.com";
const common::net::HostAndPort g_hs("localhost", 8011);
const char kRequest[] = "/echo";
const common::net::HostAndPort kHostAndPort(kHost, 80);
const common::libev::http::HttpServerInfo kHinf(PROJECT_NAME_TITLE, PROJECT_DOMAIN);
}  // namespace

class ServerWebHandler : public common::libev::IoLoopObserver {
  const common::libev::http::HttpServerInfo info_;

 public:
  explicit ServerWebHandler(const common::libev::http::HttpServerInfo& info) : info_(info) {}

  ~ServerWebHandler() {}

  const common::libev::http::HttpServerInfo& info() const { return info_; }

  virtual void PreLooped(common::libev::IoLoop* server) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(server);
    common::net::socket_info sc;
    common::ErrnoError errn = common::net::connect(kHostAndPort, common::net::ST_SOCK_STREAM, nullptr, &sc);
    ASSERT_FALSE(errn);

    common::libev::websocket::WebSocketClient* cl = new common::libev::websocket::WebSocketClient(sserver, sc);
    cl->SetBlocking(false);
    sserver->RegisterClient(cl);
    const std::string request = common::MemSPrintf(
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        kRequest, kHost);
    size_t nout;
    errn = cl->Write(request.c_str(), request.size(), &nout);
    ASSERT_FALSE(errn);
  }

  virtual void Accepted(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_TRUE(cl.empty());
  }

  virtual void Moved(common::libev::IoLoop* server, common::libev::IoClient* client) override {
    UNUSED(server);
    UNUSED(client);
  }

  virtual void Closed(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_EQ(cl.size(), 1);
  }

  virtual void TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) override {
    UNUSED(server);
    UNUSED(id);
  }

#if LIBEV_CHILD_ENABLE
  virtual void Accepted(common::libev::IoChild* child) override { UNUSED(child); }
  virtual void Moved(common::libev::IoLoop* server, common::libev::IoChild* child) override {
    UNUSED(server);
    UNUSED(child);
  }
  virtual void ChildStatusChanged(common::libev::IoChild* child, int status) override {
    UNUSED(child);
    UNUSED(status);
  }
#endif

  virtual void DataReceived(common::libev::IoClient* client) override {
    char buff[BUF_SIZE] = {0};
    size_t nread = 0;
    common::ErrnoError errn = client->Read(buff, BUF_SIZE, &nread);
    if ((errn && errn->GetErrorCode() != EAGAIN) || nread == 0) {
      client->Close();
      delete client;
      return;
    }

    common::libev::http::HttpClient* hclient = dynamic_cast<common::libev::http::HttpClient*>(client);
    CHECK(hclient);
    common::http::HttpResponse resp;
    common::Error err = common::http::parse_http_responce(std::string(buff, nread), &resp);
    ASSERT_FALSE(err);
  }

  virtual void DataReadyToWrite(common::libev::IoClient* client) override { UNUSED(client); }

  virtual void PostLooped(common::libev::IoLoop* server) override {
    std::vector<common::libev::IoClient*> cl = server->GetClients();
    ASSERT_TRUE(cl.empty());
  }
};

void ExitWebServer(common::libev::tcp::TcpServer* ser) {
  common::threads::PlatformThread::Sleep(10000);
  ser->Stop();
}

TEST(Libev, Webscoket) {
  ServerWebHandler hand(kHinf);
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(g_hs, false, &hand);
  common::ErrnoError err = serv->Bind(true);
  ASSERT_FALSE(err);

  err = serv->Listen(5);
  ASSERT_FALSE(err);

  auto tp = THREAD_MANAGER()->CreateThread(&ExitWebServer, serv);
  GTEST_ASSERT_EQ(tp->GetHandle(), common::threads::invalid_thread_handle());
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  tp->Join();
  delete serv;
}
