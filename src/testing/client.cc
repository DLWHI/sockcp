#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>

#include <sockcp/socket.h>
#include <sockcp/socket_observer.h>


int main(int argc, char* argv[]) {
  std::string ipaddr("127.0.0.1");
  uint16_t port = 4483;
  if (argc > 2) {
    ipaddr = argv[1];
    port = std::stoi(std::string(argv[2]));
  } else if (argc > 1) {
    ipaddr = argv[1];
  }
  sockcp::socket cl_sock(sockcp::socktype::stream);
  sockcp::ipv4 addr(ipaddr, port);
  cl_sock.connect(addr);
  bool running = true;
  std::string msg;
  for(;running;) {
    std::cin >> msg;
    cl_sock.write(msg);
    std::transform(msg.begin(), msg.end(), msg.begin(), [](char c){return std::tolower(c);});
    if (msg == "shutdown") {
      running = false;
    }
  }
  return 0;
}
