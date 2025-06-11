cmake_minimum_required(VERSION 3.0...3.5)
project(sockcp LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HEADERS
  include/sockcp/adapter.h
  include/sockcp/address.h
  include/sockcp/errors.h
  include/sockcp/socket.h
  include/sockcp/socket_observer.h
)

set(TEST_SOURCES
  tests/ipv4_tests.cc
)

set(CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake
)

include(SetGlobalDefines)
include(SetPlatformFlags)
include(EnableGoogleTest)
include(CTest)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sockcp)

include_directories(include)

add_executable(server src/testing/server.cc)
add_executable(client src/testing/client.cc)
add_executable(server_delayed src/testing/server_delayed.cc)
add_executable(client_waiting src/testing/client_waiting.cc)


if (GTest_FOUND)
  add_executable(
      unit_tests
      tests/main.cc
      ${TEST_SOURCES}
  )
  target_include_directories(unit_tests PRIVATE include)
  target_link_libraries(
      unit_tests
      GTest::gtest_main
  )
  gtest_discover_tests(unit_tests)
endif()
