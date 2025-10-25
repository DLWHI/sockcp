#ifndef SOCKCP_SOCKCP_ERRORS_H_
#define SOCKCP_SOCKCP_ERRORS_H_

#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>

#define SOCKCP_WRAP_NOEXCEPT(expr) try{expr}catch(...){}

#define SOCKCP_ASSERT(assertion, error)                \
  if (!(assertion)) {                                   \
    throw error;                                         \
  }

namespace sockcp {

class protocol_error: public std::runtime_error {
 public:
  protocol_error(const char* msg, const std::type_info& source)
      : std::runtime_error(msg), source_(source)  {}
  protocol_error(const std::string& msg, const std::type_info& source)
      : std::runtime_error(msg.c_str()), source_(source) {}
  protocol_error(const std::string_view& msg, const std::type_info& source)
      : std::runtime_error(msg.data()), source_(source) {}

  const std::type_info& protocol() { return source_;}
private:
  const std::type_info& source_;
};

class socket_error: public std::runtime_error {
 public:
  socket_error(const char* source) 
    : std::runtime_error(std::strerror(errno)),
      errno_(errno),
      source_(source) {}

  int code() { return errno_;}
  const std::string& source() { return source_;}
 private:
  int errno_;
  std::string source_;
};

}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_ERRORS_H_
