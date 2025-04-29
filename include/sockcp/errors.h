#ifndef SOCKCP_SOCKCP_ERRORS_H_
#define SOCKCP_SOCKCP_ERRORS_H_

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>

#define SOCKCP_WRAP_NOEXCEPT(expr) try{expr}catch(...){}

#define SOCKCP_ASSERT(assertion, error)                \
  if (!(assertion)) {                                   \
    throw error;                                         \
  }

namespace sockcp {

class protocol_error: public std::runtime_error {
 public:
  protocol_error(const char* msg) : std::runtime_error(msg) {}
  protocol_error(std::string msg) : std::runtime_error(msg.c_str()) {}
  protocol_error(std::string_view msg) : std::runtime_error(msg.data()) {}
};

class socket_error: public std::runtime_error {
 public:
  socket_error() : errno_(errno), std::runtime_error(std::strerror(errno)) {}

  int code() { return errno_;}
 private:
  int errno_;
};

}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_ERRORS_H_
