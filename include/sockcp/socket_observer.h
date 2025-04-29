#ifndef SOCKCP_SOCKCP_SOCKET_OBSERVER_H_
#define SOCKCP_SOCKCP_SOCKET_OBSERVER_H_

#include <chrono>
#include <vector>
#include <unordered_map>
#include <algorithm>

#if defined(PROJ_OS_LINUX) || defined(PROJ_OS_OSX) || defined(PROJ_OS_CYGWIN)

#include <poll.h>

#elif defined(PROJ_OS_WINDOWS)

#include <winsock.h>

#endif

#include "socket.h"

namespace sockcp {
  enum class event {
    in = POLLIN,
    pri = POLLPRI,
    out = POLLOUT,
    rdhup, POLLRDHUP,
    err = POLLERR,
    hup = POLLHUP,
    nval = POLLNVAL,
    no_event = 0x0
  };

  constexpr event operator&(event lhs, event rhs) {
    return static_cast<event>
      (static_cast<int>(lhs) & static_cast<int>(rhs));
  }

  constexpr event operator|(event lhs, event rhs) {
    return static_cast<event>
      (static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  constexpr event operator^(event lhs, event rhs) {
    return static_cast<event>
      (static_cast<int>(lhs) ^ static_cast<int>(rhs));
  }

  constexpr event operator~(event x) {
    return static_cast<event>(~static_cast<int>(x));
  }

  constexpr event& operator&=(event& lhs, event  rhs) {
    lhs = lhs & rhs;
    return lhs;
  }

  constexpr event& operator|=(event& lhs, event  rhs) {
    lhs = lhs | rhs;
    return lhs;
  }

  constexpr event& operator^=(event& lhs, event  rhs) {
    lhs = lhs ^ rhs;
    return lhs;
  }

  class socket_observer final {
    struct pollfd_comp {
      bool operator()(const ::pollfd& other) {
        return fd == other.fd;
      }

      int fd;
    };
   public: 
    template <typename ProtocolFamily>
    void attach_socket(const basic_socket<ProtocolFamily>& sock, event events) {
      auto pos = std::find_if(socket_fds_.begin(), socket_fds_.end(), pollfd_comp{sock.fd()});
      auto st = socket_fds_.emplace(pos, ::pollfd{});
      st->fd = sock.fd();
      st->events = static_cast<short>(events);
    }

    template <typename ProtocolFamily>
    void detach_socket(const basic_socket<ProtocolFamily>& sock) {
      pollfd_comp kek{sock.fd()};
      auto pos = std::find_if(socket_fds_.begin(), socket_fds_.end(), pollfd_comp{sock.fd()});
      SOCKCP_ASSERT(pos != socket_fds_.end(), std::runtime_error("No such socket"));
      socket_fds_.erase(pos);
    }
    
    std::unordered_map<int, event> poll(std::chrono::milliseconds timeout) {
      int r = ::poll(socket_fds_.data(), socket_fds_.size(), timeout.count());
      std::unordered_map<int, event> events;
      if (r < 0) {
        throw socket_error();
      } else if (r > 0) {
        for (auto& pfd : socket_fds_) {
          if (pfd.revents) {
            events.emplace(pfd.fd, static_cast<event>(pfd.revents));
            pfd.revents = 0;
          } 
        }        
      }
      return events;
    }

    template <typename ProtocolFamily>
    event poll(const basic_socket<ProtocolFamily>& sock, std::chrono::milliseconds timeout) {
      auto pos = std::find_if(socket_fds_.begin(), socket_fds_.end(), pollfd_comp{sock.fd()});
      SOCKCP_ASSERT(pos != socket_fds_.end(), std::runtime_error("No such socket"));
      std::size_t i = std::distance(socket_fds_.begin(), pos);
      int r = ::poll(&socket_fds_[i], 1, timeout.count());
      SOCKCP_ASSERT(r >= 0, socket_error());
      event res = static_cast<event>(socket_fds_[i].revents);
      socket_fds_[i].revents = 0;
      return res;
    }
   private:
    std::vector<::pollfd> socket_fds_;
  };
}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_SOCKET_OBSERVER_H_
