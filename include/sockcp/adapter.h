#ifndef SOCKCP_SOCKCP_ADAPTER_H_
#define SOCKCP_SOCKCP_ADAPTER_H_

#include <string>

#include "inet_address.h"

#if defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined(__CYGWIN__)

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#elif defined(_WIN32)
#error Adapter settings are not supported outside linux environment
#include <winsock.h>

#endif

namespace sockcp {

template <typename ProtocolFamily>
struct adapter final {
  std::string name;
  ProtocolFamily addr;
};

class adapter_provider final {
 public:

 private:
  ifaddrs* adapters = nullptr;
}

}  // namespace sockcp

#endif  // SOCKCP_SOCKCP_ADAPTER_H_

