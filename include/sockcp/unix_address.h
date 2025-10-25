#if defined(PROJ_OS_LINUX) || defined(PROJ_OS_OSX) || defined(PROJ_OS_CYGWIN)
#ifndef SOCKCP_SOCKCP_UNIX_ADDRESS_H_
#define SOCKCP_SOCKCP_UNIX_ADDRESS_H_

#include <array>
#include <string>
#include <limits>
#include <charconv>

#include <sys/types.h>
#include <sys/un.h>

#include "error.h"

namespace sockcp {
  struct unix_addr final {
    static constexpr int family = AF_LOCAL;

    unix_addr() noexcept : addr(::sockaddr_un{}) {
      addr.sun_family = family;
    }

    unix_addr(const ::sockaddr_un& addr_data) noexcept : addr(addr_data) {}

    template <typename InputIt>
    unix_addr(InputIt first, InputIt last) : unix_addr() {
      set_address(first, last);
    }

    unix_addr(const char* address) : unix_addr() {
      set_address(address);
    }

    unix_addr(const std::string& address) : unix_addr(address.c_str()) {}

    unix_addr(const std::string_view& address) : unix_addr(address.begin(), address.end()) {}

    std::string to_string() const {
      return std::string(addr.sun_path);
    }

    constexpr int size() const noexcept {
      return sizeof(::sockaddr_un);
    }

    std::string address() const noexcept {
      return to_string();
    }

    const char* binary() const noexcept {
      return addr.sun_path;
    }

    template <typename InputIt>
    void set_address(InputIt first, InputIt last)  {
      for (std::size_t i = 0; first != last && *first && i < sizeof(addr.sun_path); ++i) {
        addr.sun_path[i] = *first++;
      }
    }

    void set_address(const char* address)  {
      for (std::size_t i = 0; *address && i < sizeof(addr.sun_path); ++i) {
        addr.sun_path[i] = address[i];
      }
    }

    void set_address(const std::string& address) {
      set_address(address.c_str());
    }

    void set_address(const std::string_view& address) {
      set_address(address.begin(), address.end());
    }

    const ::sockaddr* data() const noexcept {
      return reinterpret_cast<const sockaddr*>(&addr);
    }
    
    ::sockaddr* data() noexcept {
      return reinterpret_cast<sockaddr*>(&addr);
    }

    ::sockaddr_un addr;
  };
}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_UNIX_ADDRESS_H_
#endif  // LINUX
