#ifndef SOCKCP_SOCKCP_SOCKET_BUFFER_H_
#define SOCKCP_SOCKCP_SOCKET_BUFFER_H_

#include <memory>
#include <vector>

#include "socket.h"

namespace sockcp {
  template <typename ProtocolFamily>
  class basic_socket_buffer final {
   public:
    basic_socket_buffer(basic_socket<ProtocolFamily>&& socket, std::size_t buffer_size = 512u) 
      : sock_(std::move(socket)),
        buf_size_(buffer_size),
        buf_(new char[buf_size_]),
        end_(buf_),
        pos_(buf_) {
      ;
      fill_buffer();
    }

    basic_socket_buffer(const basic_socket_buffer&) = delete;
    basic_socket_buffer(basic_socket_buffer&& other)
      noexcept : sock_(std::move(other.sock_)),
        buf_size_(other.buf_size_),
        buf_(other.buf_),
        end_(other.end_),
        pos_(other.pos_) {
      other.pos_ = other.end_ = other.buf_ = nullptr;
      fill_buffer();
    }
    
    basic_socket_buffer& operator=(const basic_socket_buffer&) = delete;
    basic_socket_buffer& operator=(basic_socket_buffer&& other) noexcept {
      sock_ = std::move(other.sock_);
      buf_size_ = other.buf_size_;
      buf_ = other.buf_;
      end_ = other.end_;
      pos_ = other.pos_;
      other.pos_ = other.end_ = other.buf_ = nullptr;
      fill_buffer();
    }

    ~basic_socket_buffer() noexcept {
      delete buf_;
      flush();
    }

    const basic_socket<ProtocolFamily>& bound_socket() const noexcept {
      return sock_;
    }

    std::size_t read(char* mem, std::size_t count) {
      std::memcpy(mem, pos_, end_ - pos_);
      pos_ = end_;
      return sock_.read(mem, count);
    }

    std::vector<char> read(std::size_t count = std::size_t(-1)) {
      std::vector<char> data(pos_, end_);
      pos_ = end_;
      std::vector<char> avail = sock_.read(count);
      data.insert(data.end(), avail.begin(), avail.end());
      return data;
    }

    std::vector<char> read_until(char c) {
      std::vector<char> res;
      char* begin = pos_;
      do {
        for (;pos_ != end_ && *pos_ != c; ++pos_);
        res.insert(res.end(), begin, pos_);
        if (pos_ == end_) {
          fill_buffer();
          begin = pos_;
        }
      } while (*pos_ != c);
      res.push_back(*pos_++);
      if (pos_ == end_) {
        fill_buffer();
      }
      return res;
    }

    void flush() {
      sock_.read();
      pos_ = end_ = buf_;
    }

    basic_socket_buffer& operator>>(int& i) {
      // TODO handle narrowing
    } 

    basic_socket_buffer& operator>>(float& i) {

    }

    basic_socket_buffer& operator>>(double& i) {

    } 

    basic_socket_buffer& operator>>(std::string& i) {
      std::string res;
      char* begin = pos_;
      do {
        for (;pos_ != end_ && *pos_; ++pos_);
        res.append(begin, pos_);
        if (pos_ == end_) {
          fill_buffer();
          begin = pos_;
        }
      } while (*pos_);
      ++pos_;
      if (pos_ == end_) {
        fill_buffer();
      }
      return *this;
    } 

   private:
    void fill_buffer() {
      if (sock_.peek() > 0) {
        std::size_t rd = sock_.read(buf_, buf_size_);
        end_ = buf_ + rd;
        pos_ = buf_;
      }
    }

    basic_socket<ProtocolFamily> sock_;
    std::size_t buf_size_;
    char* buf_;
    char* end_;
    char* pos_;
  };

  using ipv4socket_buffer = basic_socket_buffer<ipv4>;  
  using socket_buffer = basic_socket_buffer<ipv4>;  
}  // namespace sound

#endif  // SOCKCP_SOCKCP_SOCKET_BUFFER_H_
