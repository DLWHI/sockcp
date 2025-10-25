#ifndef SOCKCP_SOCKCP_ADDRESS_H_
#define SOCKCP_SOCKCP_ADDRESS_H_

#include <array>
#include <string>
#include <limits>
#include <charconv>

#if defined(PROJ_OS_LINUX) || defined(PROJ_OS_OSX) || defined(PROJ_OS_CYGWIN)

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#elif defined(PROJ_OS_WINDOWS)

#include <winsock.h>

#endif

#include "error.h"

namespace sockcp {
  struct ipv4 final {
    static constexpr int family = AF_INET;

    ipv4() noexcept : addr(::sockaddr_in{}) {
      addr.sin_family = family;
    }

    ipv4(const ::sockaddr_in& addr_data) noexcept : addr(addr_data) {}
    
    ipv4(uint8_t address[4], uint16_t p = 0) noexcept : ipv4() {
      set_address(address);
      set_port(p);
    }

    ipv4(uint32_t binary, uint16_t p = 0) noexcept : ipv4() {
      set_address(binary);
      set_port(p);
    }

    ipv4(const std::string& address, uint16_t p = 0) : ipv4(address.c_str(), p) {}
    
    ipv4(const char* address, uint16_t p = 0) : ipv4() {
      set_address(address);
      set_port(p);
    }

    std::string to_string() const {
      std::string res('a', 22);
      ::inet_ntop(family, &addr.sin_addr, res.data(), 22);
      std::size_t p = res.find('\0');
      res[p] = ':';
      res.resize(++p);
      res += std::to_string(::ntohs(addr.sin_port));
      return res;
    }

    constexpr int size() const noexcept {
      return sizeof(::sockaddr_in);
    }

    std::array<uint8_t, 4> address() const noexcept {
      std::array<uint8_t, 4> res{};
      uint32_t bn = binary();
      res[0] = bn & 0xff000000;
      res[1] = bn & 0x00ff0000;
      res[2] = bn & 0x0000ff00;
      res[3] = bn & 0x000000ff;
      return res;
    }

    uint32_t binary() const noexcept {
      SOCKCP_WRAP_NOEXCEPT(return ::ntohl(addr.sin_addr.s_addr););
      return 0;  // std::unreachable
    }

    uint16_t port() noexcept {
      SOCKCP_WRAP_NOEXCEPT(return ::ntohs(addr.sin_port););
      return 0;  // std::unreachable
    }

    void set_port(uint16_t val) noexcept {
      SOCKCP_WRAP_NOEXCEPT(addr.sin_port = ::htons(val););
    }

    void set_address(uint32_t binary) noexcept {
      SOCKCP_WRAP_NOEXCEPT(addr.sin_addr.s_addr = ::htonl(binary););
    }

    void set_address(uint8_t bytes[4]) noexcept {
      SOCKCP_WRAP_NOEXCEPT(
        addr.sin_addr.s_addr = ::htonl((bytes[0] << 24) | (bytes[1] << 16) | (bytes[2] << 8) | (bytes[3]));
      );
    }

    void set_address(const char* address)  {
      SOCKCP_ASSERT(
        ::inet_pton(family, address, &addr.sin_addr) > 0, 
        protocol_error("Invalid address provided", typeid(ipv4))
      );
    }

    void set_address(const std::string& address) {
      set_address(address.c_str());
    }

    const ::sockaddr* data() const noexcept {
      return reinterpret_cast<const sockaddr*>(&addr);
    }

    ::sockaddr* data() noexcept {
      return reinterpret_cast<sockaddr*>(&addr);
    }

    ::sockaddr_in addr;
  };

  struct ipv6 final {
    static constexpr int family = AF_INET6;

    ipv6() noexcept : addr(::sockaddr_in6{}) {
      addr.sin6_family = family;
    }

    ipv6(const ::sockaddr_in6& addr_data) noexcept : addr(addr_data) {}

    ipv6(uint8_t address[16], uint16_t p = 0) noexcept : ipv6() {
      SOCKCP_WRAP_NOEXCEPT(
        std::copy(address, address + 16, addr.sin6_addr.s6_addr);
        addr.sin6_port = ::htons(p);
      );
    }

    ipv6(const std::string& address, uint16_t port = 0) : ipv6(address.c_str(), port) {}
    
    ipv6(const char* address, uint16_t port = 0) : ipv6() {
      SOCKCP_ASSERT(
        ::inet_pton(family, address, &addr.sin6_addr) > 0, 
        protocol_error("Invalid address provided", typeid(ipv4))
      )
      addr.sin6_port = ::htons(port);
    }

    std::string to_string() const {
      std::string res('a', 47);
      res[0] = '[';
      ::inet_ntop(AF_INET6, &addr.sin6_addr, res.data() + 1, 22);
      std::size_t p = res.find('\0');
      res[p] = ']';
      res[++p] = ':';
      res.resize(++p);
      res += std::to_string(::ntohs(addr.sin6_port));
      return res;
    }

    constexpr int size() const noexcept {
      return sizeof(::sockaddr_in6);
    }

    std::array<uint8_t, 16> address() const noexcept {
      std::array<uint8_t, 16> res{};
      SOCKCP_WRAP_NOEXCEPT(std::copy(addr.sin6_addr.s6_addr, addr.sin6_addr.s6_addr + 16, res.data()););
      return res;
    }

    uint16_t port() noexcept {
      SOCKCP_WRAP_NOEXCEPT(return ::ntohs(addr.sin6_port););
      return 0;  // std::unreachable
    }

    void set_port(uint16_t val) noexcept {
      SOCKCP_WRAP_NOEXCEPT(addr.sin6_port = ::htons(val););
    }

    void set_address(uint8_t bytes[16]) noexcept {
      SOCKCP_WRAP_NOEXCEPT(
        std::copy(bytes, bytes + 16, addr.sin6_addr.s6_addr);
      );
    }

    void set_address(const char* address)  {
      SOCKCP_ASSERT(
        ::inet_pton(family, address, &addr.sin6_addr) > 0,
        protocol_error("Invalid address provided", typeid(ipv4))
      );
    }

    void set_address(const std::string& address) {
      set_address(address.c_str());
    }

    const ::sockaddr* data() const noexcept {
      return reinterpret_cast<const sockaddr*>(&addr);
    }

    ::sockaddr* data() noexcept {
      return reinterpret_cast<sockaddr*>(&addr);
    }

    ::sockaddr_in6 addr;
  };
}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_ADDRESS_H_

