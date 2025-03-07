cmake_minimum_required(VERSION 3.10)
project(authmanager)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(absl REQUIRED)
find_package(gRPC CONFIG REQUIRED)

set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/generated")

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

include_directories(${SRC_DIR})
include_directories(${GENERATED_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/AuthServiceImpl.cpp
    ${SRC_DIR}/utils.cpp
    ${GENERATED_DIR}/auth.pb.cc
    ${GENERATED_DIR}/auth.grpc.pb.cc
)

add_executable(authmanager ${SOURCES})

target_link_libraries(authmanager
    gRPC::grpc++
    grpc
    gpr             
    absl::synchronization
    protobuf::libprotobuf
    pqxx
    pq
    Threads::Threads
)
