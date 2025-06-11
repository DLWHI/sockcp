#include <algorithm>
#include <cctype>
#include <iostream>
#include <mutex>
#include <thread>
#include <memory>
#include <string>

#include <sockcp/socket.h>
#include <sockcp/socket_observer.h>

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
  std::cout << "Echo server running and listening on " << ipaddr << std::endl;
  std::mutex wr_lock;
  for(;;) {
    std::shared_ptr<sockcp::socket> cl_sock_ptr(new sockcp::socket(sv_sock.accept()));
    std::thread daemon([cl_sock_ptr, &wr_lock]() {
      bool is_alive = true;
      for (;is_alive;) {
        std::vector<char> data = cl_sock_ptr->read();
        std::string resp(data.begin(), data.end());
        wr_lock.lock();
        std::cout << resp << std::endl;
        wr_lock.unlock();
        std::transform(resp.begin(), resp.end(), resp.begin(), [](char c){return std::tolower(c);});
        if (resp == "shutdown") {
          is_alive = false;
        }
      }
    });
    daemon.detach();
  }
  return 0;
}
