#ifndef SOCKCP_SOCKCP_SOCKET_OBSERVER_H_
#define SOCKCP_SOCKCP_SOCKET_OBSERVER_H_

#include <chrono>
#include <vector>
#include <unordered_map>
#include <algorithm>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__CYGWIN__)

#include <poll.h>
#define sockcp_poll(x, y, z) ::poll(x, y, z)

#elif defined(_WIN32)

#include <winsock2.h>
#define sockcp_poll(x, y, z) ::WSAPoll(x, y, z)

#else
#error Unknown socket API.
#endif
#include "socket.h"

namespace sockcp
{
#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__)) || defined(__CYGWIN__)

  using pollfd_t = ::pollfd;
  s
#elif defined(_WIN32)

  using pollfd_t = WSAPOLLFD;

#endif

      enum class event {
        in = POLLIN,
        pri = POLLPRI,
        out = POLLOUT,
        err = POLLERR,
        hup = POLLHUP,
        nval = POLLNVAL,
        all = POLLIN | POLLPRI | POLLOUT | POLLERR | POLLHUP | POLLNVAL,
        no_event = 0x0
      };

  constexpr event operator&(event lhs, event rhs)
  {
    return static_cast<event>(static_cast<int>(lhs) & static_cast<int>(rhs));
  }

  constexpr event operator|(event lhs, event rhs)
  {
    return static_cast<event>(static_cast<int>(lhs) | static_cast<int>(rhs));
  }

  constexpr event operator^(event lhs, event rhs)
  {
    return static_cast<event>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
  }

  constexpr event operator~(event x)
  {
    return static_cast<event>(~static_cast<int>(x));
  }

  constexpr event &operator&=(event &lhs, event rhs)
  {
    lhs = lhs & rhs;
    return lhs;
  }

  constexpr event &operator|=(event &lhs, event rhs)
  {
    lhs = lhs | rhs;
    return lhs;
  }

  constexpr event &operator^=(event &lhs, event rhs)
  {
    lhs = lhs ^ rhs;
    return lhs;
  }

  class socket_observer final
  {
    struct pollfd_comp
    {
      bool operator()(const pollfd_t &other)
      {
        return fd == other.fd;
      }

      fd_type fd;
    };

  public:
    template <typename ProtocolFamily>
    void attach_socket(const basic_socket<ProtocolFamily> &sock, event events)
    {
      auto pos = std::find_if(socket_fds_.begin(), socket_fds_.end(), pollfd_comp{sock.fd()});
      auto st = socket_fds_.emplace(pos, pollfd{});
      st->fd = sock.fd();
      st->events = static_cast<short>(events);
    }

    template <typename ProtocolFamily>
    void detach_socket(const basic_socket<ProtocolFamily> &sock)
    {
      pollfd_comp kek{sock.fd()};
      auto pos = std::find_if(socket_fds_.begin(), socket_fds_.end(), pollfd_comp{sock.fd()});
      SOCKCP_ASSERT(pos != socket_fds_.end(), std::logic_error("No such socket"));
      socket_fds_.erase(pos);
    }

    std::unordered_map<fd_type, event> poll(std::chrono::milliseconds timeout)
    {
      int r = sockcp_poll(socket_fds_.data(), socket_fds_.size(), timeout.count());
      std::unordered_map<fd_type, event> events;
      if (r < 0)
      {
        throw socket_error("poll");
      }
      else if (r > 0)
      {
        for (auto &pfd : socket_fds_)
        {
          if (pfd.revents)
          {
            events.emplace(pfd.fd, static_cast<event>(pfd.revents));
            pfd.revents = 0;
          }
        }
      }
      return events;
    }

  private:
    std::vector<pollfd_t> socket_fds_;
  };

  template <typename ProtocolFamily>
  event poll(const basic_socket<ProtocolFamily> &sock, std::chrono::milliseconds timeout, event subs = event::all)
  {
    pollfd_t pfd{};
    pfd.fd = sock.fd();
    pfd.events = static_cast<short>(subs);
    int r = sockcp_poll(&pfd, 1, timeout.count());
    SOCKCP_ASSERT(r >= 0, socket_error("poll"));
    event res = static_cast<event>(pfd.revents);
    return res;
  }
} // namespace sockcp

#endif // SOCKCP_SOCKCP_SOCKET_OBSERVER_H_
