#ifndef SOCKCP_SOCKCP_SOCKET_H_
#define SOCKCP_SOCKCP_SOCKET_H_

#include <vector>
#include <string>
#include <string_view>

#if defined(PROJ_OS_LINUX) || defined(PROJ_OS_OSX) || defined(PROJ_OS_CYGWIN)

#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#elif defined(PROJ_OS_WINDOWS)

#include <winsock.h>

#endif

#include "address.h"

namespace sockcp {

  enum class socktype {
    stream = SOCK_STREAM,
    datagram = SOCK_DGRAM,
    seqpacket = SOCK_SEQPACKET,
    raw = SOCK_RAW,
    rdm = SOCK_RDM,
    nonblock = SOCK_NONBLOCK,
    cloexec = SOCK_CLOEXEC
  };

  constexpr socktype operator&(socktype lhs, socktype rhs) {
    return static_cast<socktype>
      (static_cast<int>(lhs) & static_cast<int>(rhs));
  }

  constexpr socktype operator|(socktype lhs, socktype rhs) {
    return static_cast<socktype>
      (static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  constexpr socktype operator^(socktype lhs, socktype rhs) {
    return static_cast<socktype>
      (static_cast<int>(lhs) ^ static_cast<int>(rhs));
  }

  constexpr socktype operator~(socktype x) {
    return static_cast<socktype>(~static_cast<int>(x));
  }

  constexpr socktype& operator&=(socktype& lhs, socktype  rhs) {
    lhs = lhs & rhs;
    return lhs;
  }

  constexpr socktype& operator|=(socktype& lhs, socktype  rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  constexpr socktype& operator^=(socktype& lhs, socktype  rhs) {
    lhs = lhs ^ rhs;
    return lhs;
  }

  enum class closeway {
    read = SHUT_RD,
    write = SHUT_WR,
    readwrite = SHUT_RDWR,
    rd = SHUT_RD,
    wr = SHUT_WR,
    rdwr = SHUT_RDWR
  };

  template <typename ProtocolFamily>
  class basic_socket final {
   public:
    static constexpr int protocol_family = ProtocolFamily::family;

    basic_socket(socktype type, int protocol = 0) 
        : type_(static_cast<int>(type)) {
      fd_ = ::socket(protocol_family, type_, protocol);
      SOCKCP_ASSERT(fd_ >= 0, socket_error("basic_socket"));
    }

    basic_socket(basic_socket&) = delete;
    basic_socket(basic_socket&& other) noexcept {
      *this = std::move(other);
    }

    basic_socket& operator=(basic_socket&) = delete;
    basic_socket& operator=(basic_socket&& other) noexcept {
      fd_ = other.fd_;
      type_ = other.type_;
      other.fd_ = -1;
      other.type_ = 0;
      name_ = std::move(other.name_);
      return *this;
    }

    ~basic_socket() noexcept {
      close();
    }

    int fd() const noexcept { return fd_; }

    int type() const noexcept { return type_; }

    bool is_blocking() const noexcept { return type_ & SOCK_NONBLOCK;}

    void set_blocking(bool val) { 
      ::fcntl(fd_, F_SETFL, O_NONBLOCK*static_cast<int>(val));
      type_ &= ~SOCK_NONBLOCK;
      type_ |= static_cast<int>(val)*SOCK_NONBLOCK;
    }

    void bind(ProtocolFamily addr) {
      SOCKCP_ASSERT(
        ::bind(fd_, addr.data(), addr.size()) == 0, 
        socket_error("bind")
      );
      name_ = addr;
    }

    const ProtocolFamily bound_address() const noexcept {
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
      ::recv(fd_, &c, 1, MSG_DONTWAIT | MSG_PEEK);
      SOCKCP_ASSERT(
        !errno || errno == EAGAIN || errno == EWOULDBLOCK,
        socket_error("peek")
      );
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
        SOCKCP_ASSERT(
          !errno || errno == EAGAIN || errno == EWOULDBLOCK,
          socket_error("read")
        );
        res.insert(res.end(), buf.begin(), buf.begin() + rdbytes);
        count -= rdbytes;
      }
      return res;
    }

    void write(const char* data, std::size_t count, std::size_t chunk = 0) {
      if (!chunk) {
        chunk = count;
      }
      for (;count;) {
        errno = 0;
        std::size_t wrbytes = ::send(fd_, data, chunk, 0);
        SOCKCP_ASSERT(
          !errno || errno == EAGAIN || errno == EWOULDBLOCK,
          socket_error("write")
        );
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
      SOCKCP_WRAP_NOEXCEPT(::close(fd_);)
    }

    
   private:
    basic_socket(int fd, ProtocolFamily addr) noexcept
        : fd_(fd), name_(addr) {}

    int fd_;
    int type_;
    ProtocolFamily name_;
  };

  using ipv4socket = basic_socket<ipv4>;
  using socket = basic_socket<ipv4>;

}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_SOCKET_H_
