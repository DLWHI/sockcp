cmake_minimum_required(VERSION 3.2...4.2)
set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/cmake
)
include(LoadOptions)

set_project_options()
project(${PROJECT} LANGUAGES C CXX)
set_compiler_options()

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

include(EnableGoogleTest)
include(CTest)

install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/sockcp)

add_library(sockcp INTERFACE)


target_include_directories(sockcp INTERFACE include)

if(CYGWIN OR WIN32)
  target_link_libraries(sockcp INTERFACE ws2_32)
endif()

add_executable(server src/testing/server.cc)
add_executable(client src/testing/client.cc)
add_executable(server_delayed src/testing/server_delayed.cc)
add_executable(client_waiting src/testing/client_waiting.cc)

target_link_libraries(server sockcp)
target_link_libraries(client sockcp)
target_link_libraries(server_delayed sockcp)
target_link_libraries(client_waiting sockcp)

if(GTest_FOUND)
  add_executable(
    unit_tests
    tests/main.cc
    ${TEST_SOURCES}
  )
  target_include_directories(unit_tests PRIVATE include)
  target_link_libraries(
    unit_tests
    GTest::gtest_main
    sockcp
  )
  gtest_discover_tests(unit_tests)
endif()
