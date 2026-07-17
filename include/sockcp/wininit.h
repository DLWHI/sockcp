#ifndef SOCKCP_SOCKCP_WININIT_H_
#define SOCKCP_SOCKCP_WININIT_H_

#if !defined(_WIN32)
#error wininit.h is not supported outside Windows
#endif

#include <system_error>
#include <winsock2.h>
#include <windows.h>

#include "error.h"

namespace sockcp {
  class winsock_error final : public std::runtime_error {
   public: 
    winsock_error(int errcode) : std::runtime_error(std::system_category().message(errcode)), code_(errcode) {
      WSACleanup();
    }
   
    int code() const noexcept { return code_;}
   private:
    int code_;  
  };

  class wsadata_allocator final {
   public:
    using value_type = WSADATA;
    using pointer = const WSADATA*;
    using const_pointer = const WSADATA*;
    using void_pointer = void*;
    using const_void_pointer = const void*;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using is_always_equal = std::true_type;

    void set_winsock_version(int major, int minor) noexcept {
      wsa_version_ = MAKEWORD(major, minor);     
    }

    int get_winsock_version_major() const noexcept {
      return LOBYTE(wsa_version_);
    }

    int get_winsock_version_minor() const noexcept {
      return HIBYTE(wsa_version_);
    }

    size_type max_size() const noexcept {
      return 1;
    }

    pointer allocate(size_type count = 0) {
      (void) count;
      if (!refs_) {
        int r = WSAStartup(wsa_version_, &wsa_data_);
        SOCKCP_ASSERT(r == 0, winsock_error(r));    
      }
      ++refs_;      
      return &wsa_data_;
    }

    void deallocate(pointer ptr = nullptr, size_type count = 0) noexcept {
      --refs_;
      if (!refs_) {
        WSACleanup();
      }
    }

    constexpr bool operator==(const wsadata_allocator& other) const noexcept {
      return true;
    }

    constexpr bool operator!=(const wsadata_allocator& other) const noexcept {
      return false;
    }
   private:
    inline static WORD wsa_version_ = MAKEWORD(2, 2);
    inline static WSADATA wsa_data_;
    inline static size_type refs_ = 0; 
 };
}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_WININIT_H_
