#ifndef SOCKCP_SOCKCP_ADAPTER_H_
#define SOCKCP_SOCKCP_ADAPTER_H_

#include <string>

#include "address.h"

#if defined(PROJ_OS_LINUX) || defined(PROJ_OS_OSX) || defined(PROJ_OS_CYGWIN)

#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#elif defined(PROJ_OS_WINDOWS)

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

