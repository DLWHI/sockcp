#include <algorithm>
#include <cctype>
#include <iostream>
#include <thread>
#include <memory>
#include <string>

#include <sockcp/socket.h>
#include <sockcp/socket_observer.h>

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
  std::string ipaddr = "127.0.0.1";
  uint16_t port = 4483;
  if (argc > 2) {
    ipaddr = argv[1];
    port = std::stoi(std::string(argv[2]));
  } else if (argc > 1) {
    ipaddr = argv[1];
  }
  sockcp::socket sv_sock(sockcp::socktype::stream);
  sockcp::ipv4 addr(ipaddr, port);
  sv_sock.bind(addr);

  sv_sock.listen();
  std::cout << "Static message server running and listening on " << ipaddr << std::endl;
  for(;;) {
    auto cl_sock = sv_sock.accept();
    std::this_thread::sleep_for(3000ms);
    cl_sock.write(std::string("I have come after 3000ms"));
    std::cout << "Sent data to " << cl_sock.name().to_string() << std::endl;
  }
  return 0;
}
