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

#include <common/net/net.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#if defined(OS_POSIX)
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/uio.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#if defined(OS_MACOSX) || defined(OS_FREEBSD)
#include <sys/types.h>
#include <sys/uio.h>
#elif defined(OS_LINUX)
#include <sys/sendfile.h>
#endif

#if defined(COMPILER_MSVC)
#include <io.h>
#endif

#include <common/eintr_wrapper.h>
#include <common/sprintf.h>

#if defined(OS_WIN) || defined(OS_ANDROID) || defined(OS_MACOSX) || defined(OS_FREEBSD) || defined(OS_IOS)

#define BUF_SIZE 8192

namespace {

ssize_t sendfile(common::net::socket_descr_t out_fd, descriptor_t in_fd, off_t* offset, size_t count) {
  off_t orig = 0;
  char buf[BUF_SIZE] = {0};

  if (offset) {
    /* Save current file offset and set offset to value in '*offset' */

    orig = lseek(in_fd, 0, SEEK_CUR);
    if (orig == -1) {
      return ERROR_RESULT_VALUE;
    }
    if (lseek(in_fd, *offset, SEEK_SET) == -1) {
      return ERROR_RESULT_VALUE;
    }
  }

  ssize_t totSent = 0;

  while (count > 0) {
    ssize_t num_read = read(in_fd, buf, BUF_SIZE);
    if (num_read == ERROR_RESULT_VALUE) {
      return ERROR_RESULT_VALUE;
    }
    if (num_read == 0) {
      break; /* EOF */
    }

    size_t numSent = 0;
    common::ErrnoError err = common::net::write_to_tcp_socket(out_fd, buf, static_cast<size_t>(num_read), &numSent);
    if (err) {
      return ERROR_RESULT_VALUE;
    }

    if (numSent == 0) {
      return ERROR_RESULT_VALUE;
    }

    count -= numSent;
    totSent += numSent;
  }

  if (offset) {
    /* Return updated file offset in '*offset', and reset the file offset
       to the value it had when we were called. */

    *offset = lseek(in_fd, 0, SEEK_CUR);
    if (*offset == -1) {
      return ERROR_RESULT_VALUE;
    }
    if (lseek(in_fd, orig, SEEK_SET) == -1) {
      return ERROR_RESULT_VALUE;
    }
  }

  return totSent;
}

}  // namespace
#endif

namespace {
int compare_addr(const struct addrinfo* a, const struct addrinfo* b) {
  if (a->ai_family != b->ai_family) {
    return 1;
  }

  if (a->ai_family == AF_INET) {
    return (((const struct sockaddr_in*)a)->sin_addr.s_addr != ((const struct sockaddr_in*)b)->sin_addr.s_addr);
  }

  if (a->ai_family == AF_INET6) {
    const uint8_t* s6_addr_a = ((const struct sockaddr_in6*)a)->sin6_addr.s6_addr;
    const uint8_t* s6_addr_b = ((const struct sockaddr_in6*)b)->sin6_addr.s6_addr;
    return memcmp(s6_addr_a, s6_addr_b, 16);
  }
  return 1;
}
}  // namespace

namespace common {
namespace net {

namespace {

template <typename CHAR>
ErrnoError do_sendto(socket_descr_t fd,
                     const CHAR* data,
                     uint16_t len,
                     struct sockaddr* addr,
                     socklen_t addr_len,
                     ssize_t* nwritten_out) {
  if (!data) {
    return make_error_perror("sendto", EINVAL);
  }

  if (!addr) {
    return make_error_perror("sendto", EINVAL);
  }

  if (fd == INVALID_SOCKET_VALUE) {
    return make_error_perror("sendto", EINVAL);
  }

#if defined(OS_WIN)
  ssize_t res = ::sendto(fd, reinterpret_cast<const char*>(data), len, 0, addr, addr_len);
#else
  ssize_t res = ::sendto(fd, data, len, 0, addr, addr_len);
#endif

  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("sendto", errno);
  }
  if (*nwritten_out) {
    *nwritten_out = res;
  }
  return ErrnoError();
}

template <typename CHAR>
ErrnoError do_recvfrom(socket_descr_t fd,
                       CHAR* out_data,
                       uint16_t max_size,
                       sockaddr* addr,
                       socklen_t* addr_len,
                       ssize_t* nread_out) {
  if (!out_data) {
    return make_error_perror("recvfrom", EINVAL);
  }

  if (!addr) {
    return make_error_perror("recvfrom", EINVAL);
  }

  if (fd == INVALID_SOCKET_VALUE) {
    return make_error_perror("recvfrom", EINVAL);
  }

#if defined(OS_WIN)
  ssize_t res = ::recvfrom(fd, reinterpret_cast<char*>(out_data), max_size, 0, addr, addr_len);
#else
  ssize_t res = ::recvfrom(fd, out_data, max_size, 0, addr, addr_len);
#endif

  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("recvfrom", errno);
  }
  if (*nread_out) {
    *nread_out = res;
  }
  return ErrnoError();
}

struct UnBlockAndBlockSocket {
  explicit UnBlockAndBlockSocket(socket_descr_t sock) : sock_(sock) {
    ErrnoError err = set_blocking_socket(sock, false);
    DCHECK(!err) << err->GetDescription();
  }
  ~UnBlockAndBlockSocket() {
    ErrnoError err = set_blocking_socket(sock_, true);
    DCHECK(!err) << err->GetDescription();
  }

 private:
  socket_descr_t sock_;
};

int socket_type_to_native(socket_t socktype) {
  return static_cast<int>(socktype);
}

socket_t native_to_socket_type(int socktype) {
  return static_cast<socket_t>(socktype);
}

ErrnoError do_connect(socket_descr_t sock, const struct sockaddr* addr, socklen_t len, struct timeval* tv) {
  if (!addr || sock == INVALID_SOCKET_VALUE) {
    DNOTREACHED();
    return make_errno_error_inval();
  }

  if (!tv) {
    int res = ::connect(sock, addr, len);
    if (res == ERROR_RESULT_VALUE) {
      return make_error_perror("connect", errno);
    }

    return ErrnoError();
  }

  // async connect
  UnBlockAndBlockSocket blocker(sock);
  int res = ::connect(sock, addr, len);
  if (res < 0) {
    if (errno == EINPROGRESS) {
#if defined(OS_POSIX)
      struct pollfd fds[1];
      fds[0].fd = sock;
      fds[0].events = POLLOUT;
      int msec = (tv->tv_sec * 1000) + ((tv->tv_usec + 999) / 1000);
      res = poll(fds, 1, msec);
#else
      fd_set master_set;
      FD_ZERO(&master_set);
      FD_SET(sock, &master_set);
      res = select(sock + 1, &master_set, nullptr, nullptr, tv);
#endif
      if (res == -1) {
        return make_error_perror("async_connect poll", errno);
      } else if (res == 0) {
        return make_error_perror("async_connect timeout", ETIMEDOUT);
      }

      // Socket selected for write
      int so_error = 0;
      socklen_t errlen = sizeof(so_error);
      if (getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&so_error), &errlen) == ERROR_RESULT_VALUE) {
        return make_error_perror("async_connect getsockopt", errno);
      }

      // Check the value returned...
      if (so_error) {
        return make_error_perror("async_connect", so_error);
      }
    }
    return make_error_perror("async_connect", errno);
  }

  return ErrnoError();
}

ErrnoError resolve_raw(const char* host, uint16_t port, int socktype, socket_info* out_info, int family = AF_UNSPEC) {
  if (!host || !out_info) {
    return make_error_perror("connect", EINVAL);
  }

  socket_descr_t sfd = INVALID_SOCKET_VALUE;

  struct addrinfo hints, *rp = nullptr;
  struct addrinfo* result = nullptr;
  char _port[6];
  snprintf(_port, sizeof(_port), "%u", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = family;
  hints.ai_socktype = socktype;
  hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
  hints.ai_protocol = 0;       /* Any protocol */
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;

  int rv = getaddrinfo(host, _port, &hints, &result);
  if (rv != 0) {
    return make_error_perror("getaddrinfo", rv);
  }

  for (rp = result; rp != nullptr; rp = rp->ai_next) {
    if (rp->ai_socktype != socktype) {
      continue;
    }

    sfd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == INVALID_SOCKET_VALUE) {
      continue;
    }

    break;
  }

  if (!rp) { /* No address succeeded */
    int err = errno;
    DCHECK(err) << "Errno(" << err << ") should be not zero!";
    freeaddrinfo(result); /* No longer needed */
    return make_error_perror("getaddrinfo", err);
  }

  out_info->set_addrinfo(rp);
  out_info->set_fd(sfd);
  out_info->set_host(host);
  out_info->set_port(port);

  freeaddrinfo(result); /* No longer needed */
  return ErrnoError();
}

ErrnoError connect_raw(const char* host,
                       uint16_t port,
                       socket_t socktype,
                       struct timeval* timeout,
                       socket_info* out_info) {
  if (!host || !out_info) {
    return make_error_perror("connect", EINVAL);
  }

  socket_descr_t sfd = INVALID_SOCKET_VALUE;

  struct addrinfo hints, *rp = nullptr;
  struct addrinfo* result = nullptr;
  char _port[6];
  snprintf(_port, sizeof(_port), "%u", port);
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = socket_type_to_native(socktype);
  hints.ai_flags = AI_PASSIVE; /* For wildcard IP address */
  hints.ai_protocol = 0;       /* Any protocol */
  hints.ai_canonname = nullptr;
  hints.ai_addr = nullptr;
  hints.ai_next = nullptr;
  int rv = getaddrinfo(host, _port, &hints, &result);
  if (rv != 0) {
    return make_error_perror("getaddrinfo", rv);
  }

  for (rp = result; rp != nullptr; rp = rp->ai_next) {
    if (rp->ai_socktype != socktype) {
      continue;
    }

    sfd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sfd == INVALID_SOCKET_VALUE) {
      continue;
    }

    if (socktype == ST_SOCK_DGRAM) {
      break; /* Success */
    }

    ErrnoError err = do_connect(sfd, rp->ai_addr, rp->ai_addrlen, timeout);
    if (!err) {
      break;
    }

    ::close(sfd);
  }

  if (!rp) { /* No address succeeded */
    int err = errno;
    CHECK(err) << "Errno(" << err << ") should be not zero!";
    freeaddrinfo(result); /* No longer needed */
    return make_error_perror("getaddrinfo", err);
  }

  out_info->set_addrinfo(rp);
  out_info->set_fd(sfd);
  out_info->set_host(host);
  out_info->set_port(port);

  freeaddrinfo(result); /* No longer needed */
  return ErrnoError();
}

}  // namespace

ErrnoError socket(int domain, socket_t type, int protocol, socket_info* out_info) {
  if (!out_info) {
    return make_error_perror("socket", EINVAL);
  }

  int ntype = socket_type_to_native(type);
  socket_descr_t sd = ::socket(domain, ntype, protocol);
  if (sd == INVALID_SOCKET_VALUE) {
    return make_error_perror("socket", errno);
  }

  out_info->set_fd(sd);

  addrinfo* ainf = alloc_addrinfo();
  ainf->ai_family = domain;
  ainf->ai_socktype = ntype;
  ainf->ai_protocol = protocol;
  out_info->set_addrinfo(ainf);

  freeaddrinfo_ex(&ainf);

  return ErrnoError();
}

ErrnoError bind(socket_descr_t fd, const struct addrinfo* ainf, bool reuseaddr, socket_info* out_info) {
  if (!ainf || fd == INVALID_SOCKET_VALUE || !out_info) {
    return make_error_perror("bind", EINVAL);
  }

  if (reuseaddr) {
    const int optionval = 1;
    int res = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&optionval, sizeof(optionval));
    if (res == ERROR_RESULT_VALUE) {
      return make_error_perror("setsockopt", errno);
    }
  }

  int res = ::bind(fd, ainf->ai_addr, ainf->ai_addrlen);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("bind", errno);
  }

  out_info->set_fd(fd);
  out_info->set_addrinfo(ainf);
  return ErrnoError();
}

ErrnoError getsockname(socket_descr_t fd, const struct addrinfo* ainf, socket_info* out_info) {
  if (!ainf || fd == INVALID_SOCKET_VALUE || !out_info) {
    return make_error_perror("getsockname", EINVAL);
  }

  socklen_t len = ainf->ai_addrlen;
  int res = ::getsockname(fd, ainf->ai_addr, &len);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("getsockname", errno);
  }

  out_info->set_fd(fd);
  out_info->set_addrinfo(ainf);
  return ErrnoError();
}

ErrnoError get_in_port(const addrinfo* ainf, uint16_t* out) {
  if (!ainf || !out) {
    return make_errno_error_inval();
  }

  if (ainf->ai_family == AF_INET) {
    struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(ainf->ai_addr);
    *out = htons(sin->sin_port);
    return ErrnoError();
  }

  struct sockaddr_in6* sin = reinterpret_cast<struct sockaddr_in6*>(ainf->ai_addr);
  *out = htons(sin->sin6_port);
  return ErrnoError();
}

ErrnoError get_in_addr(const addrinfo* ainf, std::string* out) {
  if (!ainf || !out) {
    return make_errno_error_inval();
  }

#if defined(OS_WIN)
  struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(ainf->ai_addr);
  char* data = inet_ntoa(sin->sin_addr);
  if (!data) {
    return make_errno_error_inval();
  }
  *out = data;
  return ErrnoError();
#else
  if (ainf->ai_family == AF_INET) {
    struct sockaddr_in* sin = reinterpret_cast<struct sockaddr_in*>(ainf->ai_addr);
    char* str = static_cast<char*>(calloc(INET_ADDRSTRLEN, sizeof(char)));
    const char* result = inet_ntop(AF_INET, &sin->sin_addr, str, INET_ADDRSTRLEN);
    if (!result) {
      free(str);
      return make_errno_error_inval();
    }
    *out = str;
    free(str);
    return ErrnoError();
  }

  struct sockaddr_in6* sin = reinterpret_cast<struct sockaddr_in6*>(ainf->ai_addr);
  char* str = static_cast<char*>(calloc(INET6_ADDRSTRLEN, sizeof(char)));
  const char* result = inet_ntop(AF_INET6, &sin->sin6_addr, str, INET6_ADDRSTRLEN);
  if (!result) {
    free(str);
    return make_errno_error_inval();
  }
  *out = str;
  free(str);
  return ErrnoError();
#endif
}

ErrnoError listen(const socket_info& info, int backlog) {
  socket_descr_t fd = info.fd();
  if (fd == INVALID_SOCKET_VALUE) {
    return make_error_perror("listen", EINVAL);
  }

  int res = ::listen(fd, backlog);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("listen", errno);
  }

  return ErrnoError();
}

ErrnoError accept(const socket_info& info, socket_info* out_info) {
  socket_descr_t fd = info.fd();
  if (fd == INVALID_SOCKET_VALUE || !out_info) {
    return make_error_perror("accept", EINVAL);
  }

  *out_info = info;
  struct addrinfo* ainf = out_info->addr_info();
#if defined(OS_POSIX)
  socklen_t* addr_len = &ainf->ai_addrlen;
#else
  int* addr_len = reinterpret_cast<int*>(&ainf->ai_addrlen);
#endif
  int res = ::accept(fd, ainf->ai_addr, addr_len);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("accept", errno);
  }

  out_info->set_fd(res);
  uint16_t port = 0;
  ErrnoError errn = get_in_port(ainf, &port);
  if (!errn) {
    out_info->set_port(port);
  }
  std::string host;
  errn = get_in_addr(ainf, &host);
  if (!errn) {
    out_info->set_host(host.c_str());
  }
  return ErrnoError();
}

ErrnoError resolve(const HostAndPort& to, socket_t socktype, socket_info* out_info) {
  if (!to.IsValid() || !out_info) {
    return make_error_perror("connect", EINVAL);
  }

  const auto host = to.GetHostNoBrackets();
  const auto port = to.GetPort();

  return resolve_raw(host.c_str(), port, socket_type_to_native(socktype), out_info);
}

ErrnoError connect(const HostAndPort& to, socket_t socktype, struct timeval* timeout, socket_info* out_info) {
  if (!to.IsValid() || !out_info) {
    return make_error_perror("connect", EINVAL);
  }

  const auto host = to.GetHostNoBrackets();
  const auto port = to.GetPort();

  return connect_raw(host.c_str(), port, socktype, timeout, out_info);
}

ErrnoError connect(const socket_info& info, struct timeval* timeout, socket_info* out_info) {
  struct addrinfo* addr = info.addr_info();
  int fd = info.fd();

  if (fd == INVALID_DESCRIPTOR || !out_info) {
    return make_error_perror("connect", EINVAL);
  }

  if (!addr) {
    return make_error_perror("connect", EINVAL);
  }

  ErrnoError err = do_connect(fd, addr->ai_addr, addr->ai_addrlen, timeout);
  if (err) {
    return err;
  }

  out_info->set_addrinfo(addr);
  return ErrnoError();
}

ErrnoError close(socket_descr_t fd) {
  if (fd == INVALID_SOCKET_VALUE) {
    return ErrnoError();
  }

#if defined(OS_WIN)
  int res = ::closesocket(fd);
#else
  int res = ::close(fd);
#endif
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("close", errno);
  }

  return ErrnoError();
}

ErrnoError set_blocking_socket(socket_descr_t sock, bool blocking) {
#if defined(OS_POSIX)
  int opts = fcntl(sock, F_GETFL);
  if (opts < 0) {
    return make_error_perror("fcntl(F_GETFL)", errno);
  }

  if (blocking) {
    opts &= ~O_NONBLOCK;
  } else {
    opts |= O_NONBLOCK;
  }

  if (fcntl(sock, F_SETFL, opts) < 0) {
    return make_error_perror("fcntl(F_SETFL)", errno);
  }

  return ErrnoError();
#else
  unsigned long flags = blocking;
  int res = ioctlsocket(sock, FIONBIO, &flags);
  if (res == SOCKET_ERROR) {
    return make_error_perror("ioctlsocket", errno);
  }

  return ErrnoError();
#endif
}

#if defined(OS_POSIX)
ErrnoError write_ev_to_socket(socket_descr_t fd, const struct iovec* iovec, int count, size_t* nwritten_out) {
  if (fd == INVALID_SOCKET_VALUE || !iovec || count <= 0 || !nwritten_out) {
    return make_error_perror("write_ev_to_socket", EINVAL);
  }

  ssize_t lnwritten = writev(fd, iovec, count);
  if (lnwritten == ERROR_RESULT_VALUE && errno != 0) {
    return make_error_perror("writev", errno);
  }

  *nwritten_out = lnwritten;
  return ErrnoError();
}

ErrnoError read_ev_to_socket(socket_descr_t fd, const struct iovec* iovec, int count, size_t* nread_out) {
  if (fd == INVALID_SOCKET_VALUE || !iovec || count <= 0 || !nread_out) {
    return make_error_perror("write_ev_to_socket", EINVAL);
  }

  ssize_t lnread = readv(fd, iovec, count);
  if (lnread == ERROR_RESULT_VALUE && errno != 0) {
    return make_error_perror("writev", errno);
  }

  if (lnread == 0) {
    return make_errno_error(ECONNRESET);
  }

  *nread_out = lnread;
  return ErrnoError();
}

ErrnoError write_to_socket(socket_descr_t fd, const void* data, size_t size, size_t* nwritten_out) {
  if (fd == INVALID_SOCKET_VALUE || !data || size == 0 || !nwritten_out) {
    return make_error_perror("write_to_socket", EINVAL);
  }

  ssize_t lnwritten = ::write(fd, data, size);

  if (lnwritten == ERROR_RESULT_VALUE && errno != 0) {
    return make_error_perror("write", errno);
  }

  *nwritten_out = lnwritten;
  return ErrnoError();
}

ErrnoError read_from_socket(socket_descr_t fd, void* buf, size_t size, size_t* nread_out) {
  if (fd == INVALID_SOCKET_VALUE || !buf || size == 0 || !nread_out) {
    return make_error_perror("read_from_socket", EINVAL);
  }

  ssize_t lnread = ::read(fd, buf, size);

  if (lnread == ERROR_RESULT_VALUE && errno != 0) {
    return make_error_perror("read", errno);
  }

  if (lnread == 0) {
    return make_errno_error(ECONNRESET);
  }

  *nread_out = lnread;
  return ErrnoError();
}
#endif

ErrnoError write_to_tcp_socket(socket_descr_t fd, const void* data, size_t size, size_t* nwritten_out) {
  if (fd == INVALID_SOCKET_VALUE || !data || size == 0 || !nwritten_out) {
    return make_error_perror("write_to_socket", EINVAL);
  }

#if defined(OS_WIN)
  ssize_t lnwritten = send(fd, reinterpret_cast<const char*>(data), size, 0);
#else
#if defined(OS_LINUX) || defined(OS_ANDROID)
  ssize_t lnwritten = HANDLE_EINTR(send(fd, data, size, MSG_NOSIGNAL));
#else
  ssize_t lnwritten = HANDLE_EINTR(write(fd, data, size));
#endif
#endif

  if (lnwritten == ERROR_RESULT_VALUE) {
    return make_error_perror("write", errno);
  }

  *nwritten_out = lnwritten;
  return ErrnoError();
}

ErrnoError read_from_tcp_socket(socket_descr_t fd, void* buf, size_t size, size_t* nread_out) {
  if (fd == INVALID_SOCKET_VALUE || !buf || size == 0 || !nread_out) {
    return make_error_perror("read_from_socket", EINVAL);
  }

#if defined(OS_WIN)
  ssize_t lnread = recv(fd, reinterpret_cast<char*>(buf), size, 0);
#else
  ssize_t lnread = HANDLE_EINTR(read(fd, buf, size));
#endif

  if (lnread == ERROR_RESULT_VALUE) {
    return make_error_perror("read", errno);
  }

  *nread_out = lnread;
  return ErrnoError();
}

ErrnoError sendto(socket_descr_t fd,
                  const char* data,
                  uint16_t len,
                  struct sockaddr* addr,
                  socklen_t addr_len,
                  ssize_t* nwritten_out) {
  return do_sendto(fd, data, len, addr, addr_len, nwritten_out);
}

ErrnoError sendto(socket_descr_t fd,
                  const unsigned char* data,
                  uint16_t len,
                  struct sockaddr* addr,
                  socklen_t addr_len,
                  ssize_t* nwritten_out) {
  return do_sendto(fd, data, len, addr, addr_len, nwritten_out);
}

ErrnoError recvfrom(socket_descr_t fd,
                    char* out_data,
                    uint16_t max_size,
                    sockaddr* addr,
                    socklen_t* addr_len,
                    ssize_t* nread_out) {
  return do_recvfrom(fd, out_data, max_size, addr, addr_len, nread_out);
}

ErrnoError recvfrom(socket_descr_t fd,
                    unsigned char* out_data,
                    uint16_t max_size,
                    sockaddr* addr,
                    socklen_t* addr_len,
                    ssize_t* nread_out) {
  return do_recvfrom(fd, out_data, max_size, addr, addr_len, nread_out);
}

ErrnoError send_file_to_fd(socket_descr_t sock, descriptor_t fd, off_t offset, size_t size) {
  if (sock == INVALID_SOCKET_VALUE || fd == INVALID_DESCRIPTOR) {
    return make_error_perror("send_file_to_fd", EINVAL);
  }

  for (size_t size_to_send = size; size_to_send > 0;) {
    off_t off = offset;
    ssize_t sent = sendfile(sock, fd, &off, size_to_send);
    if (sent == ERROR_RESULT_VALUE) {
      return make_error_perror("sendfile", errno);
    } else if (sent == 0) {
      return ErrnoError();
    }

    offset += sent;
    size_to_send -= sent;
  }

  return ErrnoError();
}

ErrnoError send_file(const std::string& path, const HostAndPort& to) {
  if (path.empty()) {
    return make_error_perror("send_file", EINVAL);
  }

  socket_info info;
  ErrnoError err = connect(to, ST_SOCK_STREAM, nullptr, &info);
  if (err) {
    return err;
  }

  int fd = open(path.c_str(), O_RDONLY);
  if (fd == INVALID_DESCRIPTOR) {
    return make_error_perror("open", errno);
  }

  struct stat stat_buf;
  fstat(fd, &stat_buf);

  off_t offset = 0;
  err = send_file_to_fd(info.fd(), fd, offset, stat_buf.st_size);
  ::close(info.fd());
  ::close(fd);
  return err;
}

// external
bool HostAndPort::IsSameHost(const host_t& host) const {
  if (host == host_) {
    return true;
  }

  if (net::IsLocalHost(host)) {
    return IsLocalHost();
  }

  if (net::IsDefaultRoute(host)) {
    return IsDefaultRoute();
  }

  socket_info src;
  ErrnoError err = resolve_raw(host_.c_str(), port_, 1, &src);
  if (err) {
    return false;
  }

  socket_info dst;
  err = resolve_raw(host.c_str(), port_, 1, &dst);
  if (err) {
    return false;
  }

  if (src.addr_info()->ai_family != dst.addr_info()->ai_family) {
    if (src.addr_info()->ai_family == AF_INET) {
      err = resolve_raw(host_.c_str(), port_, 1, &src, AF_INET6);
      if (err) {
        return false;
      }
    } else if (dst.addr_info()->ai_family == AF_INET) {
      err = resolve_raw(host.c_str(), port_, 1, &dst, AF_INET6);
      if (err) {
        return false;
      }
    }
  }

  return compare_addr(src.addr_info(), dst.addr_info()) == 0;
}

}  // namespace net
std::string common_gai_strerror(int err) {
  const char* error_str = gai_strerror(err);
  if (error_str) {
    return error_str;
  }

  return MemSPrintf("Unknown gai error (%d)", err);
}
}  // namespace common
