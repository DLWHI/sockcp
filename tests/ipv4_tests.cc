#include <gtest/gtest.h>
#include <iostream>

#include "sockcp/address.h"
#include "sockcp/unix_addr.h"

static const std::vector<::sockaddr_in> test_pool = {
  ::sockaddr_in{AF_INET, ::htons(4483), ::inet_addr("127.0.0.1")},
  ::sockaddr_in{AF_INET, ::htons(4483), ::inet_addr("192.168.11.253")},
  ::sockaddr_in{AF_INET, ::htons(0), ::inet_addr("127.0.0.1")},
  ::sockaddr_in{AF_INET, ::htons(1337), ::inet_addr("78.105.61.33")},
  ::sockaddr_in{AF_INET, ::htons(0), ::inet_addr("63.123.243.23")},
  ::sockaddr_in{AF_INET, ::htons(0), ::inet_addr("255.255.255.0")},
};

TEST(IPv4Test, copy_from_raw) {
  for (const auto& el : test_pool) {
    sockcp::ipv4 ip(el);
    ASSERT_EQ(ip.binary(), ::ntohl(el.sin_addr.s_addr));
    ASSERT_EQ(ip.port(), ::ntohs(el.sin_port));
  }
}
  
TEST(IPv4Test, from_array) {
  std::array<uint8_t, 4> arrays[] = {
    std::array<uint8_t, 4>{127, 0, 0, 1},
    std::array<uint8_t, 4>{192, 168, 11, 253},
    std::array<uint8_t, 4>{127, 0, 0, 1},
    std::array<uint8_t, 4>{78, 105, 61, 33},
    std::array<uint8_t, 4>{63, 123, 243, 23},
    std::array<uint8_t, 4>{255, 255, 255, 0}
  };
  uint16_t ports[] = {
    4483,
    4483,
    0,
    1337,
    0,
    0
  };
  for (std::size_t i = 0; i < test_pool.size(); ++i) {
    sockcp::ipv4 ip(arrays[i].data(), ports[i]);
    ASSERT_EQ(ip.binary(), ::ntohl(test_pool[i].sin_addr.s_addr));
    ASSERT_EQ(ip.port(), ::ntohs(test_pool[i].sin_port));
  }
}

TEST(IPv4Test, from_binary) {
  uint32_t binaries[] = {
    (127u << 24u) | 1u,
    (192u << 24u) | (168u << 16u) | (11u << 8u) | 253u,
    (127u << 24u) | 1u,
    (78u << 24u) | (105u << 16u) | (61u << 8u) | 33u,
    (63u << 24u) | (123u << 16u) | (243u << 8u) | 23u,
    0xFFFFFF00
  };
  uint16_t ports[] = {
    4483,
    4483,
    0,
    1337,
    0,
    0
  };
  for (std::size_t i = 0; i < test_pool.size(); ++i) {
    sockcp::ipv4 ip(binaries[i], ports[i]);
    ASSERT_EQ(ip.binary(), ::ntohl(test_pool[i].sin_addr.s_addr));
    ASSERT_EQ(ip.port(), ::ntohs(test_pool[i].sin_port));
  }
}

TEST(IPv4Test, from_raw_data) {
  std::string ips[] = {
    "127.0.0.1",
    "192.168.11.253",
    "127.0.0.1",
    "78.105.61.33",
    "63.123.243.23",
    "255.255.255.0"
  };
  uint16_t ports[] = {
    4483,
    4483,
    0,
    1337,
    0,
    0
  };
  for (std::size_t i = 0; i < test_pool.size(); ++i) {
    sockcp::ipv4 ip(ips[i].c_str(), ports[i]);
    ASSERT_EQ(ip.binary(), ::ntohl(test_pool[i].sin_addr.s_addr));
    ASSERT_EQ(ip.port(), ::ntohs(test_pool[i].sin_port));
  }
}

TEST(IPv4Test, str_invalid_empty) {
  ASSERT_THROW(sockcp::ipv4(""), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_newline) {
  ASSERT_THROW(sockcp::ipv4("\n\n\n"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_text) {
  ASSERT_THROW(sockcp::ipv4("Lorem ipsum kekwk"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_big_byte) {
  ASSERT_THROW(sockcp::ipv4("192.445.2.889"), sockcp::protocol_error);
  ASSERT_THROW(sockcp::ipv4("192.445.2022939239123193.255"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_big_port) {
  ASSERT_THROW(sockcp::ipv4("192.168.2.23:70000"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_missing_byte) {
  ASSERT_THROW(sockcp::ipv4("192.168..23:223"), sockcp::protocol_error);
  ASSERT_THROW(sockcp::ipv4("192.168..23"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_byte_not_enough) {
  ASSERT_THROW(sockcp::ipv4("192.168.23:22"), sockcp::protocol_error);
  ASSERT_THROW(sockcp::ipv4("192.168.23"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_with_letter) {
  ASSERT_THROW(sockcp::ipv4("192.16s.2o:22"), sockcp::protocol_error);
  ASSERT_THROW(sockcp::ipv4("192.168.23.44:94y"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_missing_port) {
  ASSERT_THROW(sockcp::ipv4("192.168.22:"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_port_only) {
  ASSERT_THROW(sockcp::ipv4(":1337"), sockcp::protocol_error);
}

TEST(IPv4Test, str_invalid_no_digits) {
  ASSERT_THROW(sockcp::ipv4("....:"), sockcp::protocol_error);
}
