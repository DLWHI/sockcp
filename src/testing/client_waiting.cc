#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <string>

#include <sockcp/socket.h>
#include <sockcp/socket_observer.h>

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  std::chrono::milliseconds timeout = 1000ms;
  std::string ipaddr("127.0.0.1");
  uint16_t port = 4483;
  if (argc > 2) {
    ipaddr = argv[1];
    port = std::stoi(std::string(argv[2]));
  } else if (argc > 1) {
    ipaddr = argv[1];
  }
  sockcp::socket cl_sock(sockcp::socktype::stream | sockcp::socktype::nonblock);
  sockcp::ipv4 addr(ipaddr, port);

  cl_sock.connect(addr);
  if (static_cast<bool>(sockcp::poll(cl_sock, timeout, sockcp::event::out))) {
    std::cout << "Waiting for message" << std::endl;
    for(;;) {
      sockcp::event status = sockcp::poll(cl_sock, timeout, sockcp::event::in);
      if (static_cast<bool>(status & sockcp::event::in)) {
        std::vector<char> data = cl_sock.read();
        std::cout << std::string(data.begin(), data.end())  << std::endl;
        break;
      } else {
        std::cout << "I'm still waiting" << std::endl;
      }
    }
  } else {
    std::cout << "Connection timed out" << std::endl;
  }
  return 0;
}
