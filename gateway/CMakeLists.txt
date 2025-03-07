cmake_minimum_required(VERSION 3.10)
project(gateway)

set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)
find_package(absl REQUIRED)
find_package(gRPC CONFIG REQUIRED)
link_directories(/usr/local/lib)

set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../proto/generated")
include_directories(${GENERATED_DIR})
include_directories(${Protobuf_INCLUDE_DIRS})

set(SOURCES
${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
${CMAKE_CURRENT_SOURCE_DIR}/src/AuthServiceProxyImpl.cpp
    ${GENERATED_DIR}/auth.pb.cc
    ${GENERATED_DIR}/auth.grpc.pb.cc
)

add_executable(gateway ${SOURCES})
add_subdirectory(tests)

target_link_libraries(gateway
gRPC::grpc++
    grpc           
    gpr              
    protobuf::libprotobuf
    Threads::Threads
    absl::synchronization
)
