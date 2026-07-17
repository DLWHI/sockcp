#ifndef SOCKCP_SOCKCP_SOCKET_H_
#define SOCKCP_SOCKCP_SOCKET_H_

#include <vector>
#include <string>
#include <string_view>

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined(__CYGWIN__)

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define sock_close(x) ::close(x)

#elif defined(_WIN32)

#include "wininit.h"
#define sock_close(x) ::closesocket(x)

#else
#error Unknown socket API.
#endif

#include "inet_address.h"

namespace sockcp {
#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined(__CYGWIN__)

  using socklen_t = ::socklen_t;
  using fd_type = int;

  static constexpr fd_type fd_invalid = -1;  

  enum class closeway {
    read = SHUT_RD,
    write = SHUT_WR,
    readwrite = SHUT_RDWR,
    rd = SHUT_RD,
    wr = SHUT_WR,
    rdwr = SHUT_RDWR
  };
#elif defined(_WIN32)

  using socklen_t = int;
  using fd_type = ::SOCKET;
  
  static constexpr fd_type fd_invalid = INVALID_SOCKET;  

  enum class closeway {
    read = SD_RECEIVE,
    write = SD_SEND,
    readwrite = SD_BOTH,
    rd = SD_RECEIVE,
    wr = SD_SEND,
    rdwr = SD_BOTH
  };
#endif

  enum class socktype {
    stream = SOCK_STREAM,
    datagram = SOCK_DGRAM,
    seqpacket = SOCK_SEQPACKET,
    raw = SOCK_RAW,
    rdm = SOCK_RDM
  };

  template <typename ProtocolFamily>
  class basic_socket final {
   public:
    
    static constexpr int protocol_family = ProtocolFamily::family;

    basic_socket(socktype type, int protocol = 0) 
        : type_(static_cast<int>(type)), blocking_(false) {
#if defined(_WIN32)
      wsa_ = wsadata_allocator().allocate();
#endif  // _WIN32
      fd_ = ::socket(protocol_family, type_, protocol);
      SOCKCP_ASSERT(fd_ >= 0, socket_error("basic_socket"));
    }

    basic_socket(basic_socket&) = delete;
    basic_socket(basic_socket&& other) noexcept {
#if defined(_WIN32)
      wsa_ = wsadata_allocator().allocate();
#endif  // _WIN32
      *this = std::move(other);
    }

    basic_socket& operator=(basic_socket&) = delete;
    basic_socket& operator=(basic_socket&& other) noexcept {
      fd_ = other.fd_;
      type_ = other.type_;
      blocking_ = other.blocking_;
      other.fd_ = fd_invalid;
      other.type_ = 0;
      other.blocking_ = false;
      name_ = std::move(other.name_);
      return *this;
    }

    ~basic_socket() noexcept {

#if defined(_WIN32)
      SOCKCP_WRAP_NOEXCEPT(shutdown(closeway::readwrite););
      close();
      wsadata_allocator().deallocate();
#else
      close();
#endif  // _WIN32
    }

    fd_type fd() const noexcept { return fd_; }

    int type() const noexcept { return type_; }

    bool blocking() const noexcept {
      return blocking_;
    }

    void set_block(bool val) {
      unsigned long opt = static_cast<int>(val);
      blocking_ = val;
#if defined(_WIN32)
      SOCKCP_ASSERT(!ioctlsocket(fd_, FIONBIO, &opt), socket_error("block"));      
#else
      SOCKCP_ASSERT(!::fcntl(fd_, F_SETFL, O_NONBLOCK*opt), socket_error("block"));
#endif  // _WIN32 
    }

    void bind(ProtocolFamily addr) {
      SOCKCP_ASSERT(
        ::bind(fd_, addr.data(), addr.size()) == 0, 
        socket_error("bind")
      );
      name_ = addr;
    }

    const ProtocolFamily& name() const noexcept {
      return name_;
    }

    void connect(ProtocolFamily addr) {
      errno = 0;
      ::connect(fd_, addr.data(), addr.size());
      SOCKCP_ASSERT(!errno || errno == EINPROGRESS, socket_error("connect"));
    }

    void listen(int backlog = 0) {
      SOCKCP_ASSERT(
        ::listen(fd_, backlog) == 0, 
        socket_error("listen")
      );
    }

    basic_socket accept() {
      errno = 0;
      ProtocolFamily addr{};
      socklen_t len = addr.size();
      int newfd = ::accept(fd_, addr.data(), &len);
      SOCKCP_ASSERT(
        !errno || errno == EAGAIN || errno == EWOULDBLOCK,
        socket_error("accept")
      );
      return basic_socket(newfd, addr);
    }

    char peek() {
      char c = -1;
      errno = 0;
      ::recv(fd_, &c, 1, MSG_PEEK);
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return -1;
      }
      SOCKCP_ASSERT(!errno, socket_error("peek"));
      return c;
    }

    std::size_t read(char* mem, std::size_t count) {
      std::size_t rdbytes = -1;
      errno = 0;
      rdbytes = ::recv(fd_, mem, count, 0);
      SOCKCP_ASSERT(
        !errno || errno == EAGAIN || errno == EWOULDBLOCK,
        socket_error("read")
      );
      SOCKCP_ASSERT(rdbytes > 0, disconnect_error());
      return rdbytes;
    }

    std::vector<char> read(std::size_t count = std::size_t(-1)) {
      static constexpr std::size_t kBufLen = 255;
      std::vector<char> res;
      std::vector<char> buf;
      buf.resize(kBufLen);
      std::size_t rdbytes = kBufLen;
      for (; count && rdbytes == kBufLen;) {
        errno = 0;
        rdbytes = ::recv(fd_, buf.data(), kBufLen, 0);
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
          return res;
        }
        SOCKCP_ASSERT(
          !errno,
          socket_error("read")
        );
        res.insert(res.end(), buf.begin(), buf.begin() + rdbytes);
        count -= rdbytes;
      }
      SOCKCP_ASSERT(!res.empty(), disconnect_error());
      return res;
    }

    void write(const char* data, std::size_t count, std::size_t chunk = 0) {
      if (!chunk) {
        chunk = count;
      }
      for (;count;) {
        errno = 0;
        std::size_t wrbytes = ::send(fd_, data, chunk, 0);
        SOCKCP_ASSERT(!errno, socket_error("write"));
        count -= wrbytes;
        data += wrbytes;
      }
    }

    void write(const std::string& data, std::size_t chunk = 0) {
      write(data.data(), data.size(), chunk);
    }

    void write(const std::string_view& data, std::size_t chunk = 0) {
      write(data.data(), data.size(), chunk);
    }

    void write(const std::vector<char>& data, std::size_t chunk = 0) {
      write(data.data(), data.size(), chunk);
    }

    void shutdown(closeway how) {
      ::shutdown(fd_, static_cast<int>(how));
    }

    void close() noexcept {
      SOCKCP_WRAP_NOEXCEPT(sock_close(fd_);)
    }

    
   private:
    basic_socket(fd_type fd, ProtocolFamily addr) noexcept
        : fd_(fd), name_(addr) {}

    fd_type fd_;
    int type_;
    bool blocking_;
    ProtocolFamily name_;
#if defined(_WIN32)
    const WSADATA* wsa_;
#endif  // _WIN32
  };

  using ipv4socket = basic_socket<ipv4>;
  using socket = basic_socket<ipv4>;

}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_SOCKET_H_

